#include "wiced.h"
#include "wiced_aws.h"
#include "aws_common.h"
#include "JSON.h"

#include "sd_mem_diag.h"
#include "sd_user_io.h"
#include "sd_aws.h"

/******************************************************************************
* Constants
*******************************************************************************/

#define SHADOW_UPDATE_DCT  0x01
#define SHADOW_UPDATE_LEDS 0x02

#define MAX_QUEUE_SIZE     64

/******************************************************************************
* Type definitions
*******************************************************************************/

typedef struct aws_queue_t
{
    struct aws_queue_t *next;

    wiced_utc_time_ms_t timestamp;
    SD_REPORT_DATA_T data;
} AWS_QUEUE_T;

/******************************************************************************
* Local variables
******************************************************************************/

static wiced_aws_thing_security_info_t my_shadow_security_creds =
{
    .private_key        = NULL,
    .key_length         = 0,
    .certificate        = NULL,
    .certificate_length = 0,
};

static wiced_aws_endpoint_info_t my_shadow_aws_iot_endpoint =
{
    .transport           = WICED_AWS_TRANSPORT_MQTT_NATIVE,
    .uri                 = NULL,
    .ip_addr             = {0},
    .port                = WICED_AWS_IOT_DEFAULT_MQTT_PORT,
    .root_ca_certificate = NULL,
    .root_ca_length      = 0,
    .peer_common_name    = NULL,
};

static wiced_aws_thing_info_t my_shadow_aws_config =
{
    .name            = NULL,
    .credentials     = &my_shadow_security_creds,
};

/* Mutex to protect provisioning structures from multiple access */
static wiced_mutex_t aws_provis_mutex;

/* Handle for open connection to AWS IoT */
static wiced_aws_handle_t aws_shadow_handle;

/* Topic strings */
static char data_topic[64];
static char shadow_topic[64];
static char shadow_delta_topic[64];
static char wifi_version[128];

/* Timestamps for next reports to AWS */
static wiced_utc_time_ms_t next_shadow_update_time = 0;
static wiced_utc_time_ms_t next_report_time = 0;

/* Booleans to describe the current state */
static wiced_bool_t is_connected = WICED_FALSE;
static wiced_bool_t is_subscribed = WICED_FALSE;

/* Our thread handle */
static wiced_thread_t aws_thread;

/* Variables to hold queue of messages to be sent */
static wiced_mutex_t queue_mutex;
static AWS_QUEUE_T *aws_queue = NULL;
static int aws_queue_size = 0;
static int aws_queue_high_prio = 0;

/* String buffer to be used for building JSON strings */
static char json_string[2048];

/* Array of keys reported in the device shadow */
static struct
{
    char *key;
    uint8_t update_flag;
    int changed;
    uint32_t *value;
} SHADOW_KEYS[] = {
        { "report_interval", SHADOW_UPDATE_DCT, 0, &(sd_aws_config.aws_report_interval) },
        { "report_queue", SHADOW_UPDATE_DCT, 0, &(sd_aws_config.aws_report_queue) },
        { "shadow_update_interval", SHADOW_UPDATE_DCT, 0, &(sd_aws_config.aws_shadow_update_interval) },
        { "led_red", SHADOW_UPDATE_LEDS, 0, &(sd_led_state[0]) },
        { "led_blue", SHADOW_UPDATE_LEDS, 0, &(sd_led_state[1]) },
        { "led_green", SHADOW_UPDATE_LEDS, 0, &(sd_led_state[2]) },
};

/******************************************************************************
* Global variables
******************************************************************************/

AWS_CONFIG_DATA_T sd_aws_config;
uint8_t aws_config_prov_status = AWS_PROV_STATUS_UNPROVISIONED;

extern sd_device_data_t sd_devinfo;

/******************************************************************************
* Local functions
******************************************************************************/

/* Function prototype for updating AWS status in BLE */
void aws_config_status_notification(void);

static wiced_bool_t is_provisioned (void)
{
    wiced_bool_t result = WICED_TRUE;

    /* Lock the mutex for the provisioning data */
    wiced_rtos_lock_mutex(&aws_provis_mutex);

    if ((my_shadow_security_creds.certificate_length == 0) ||
        (my_shadow_security_creds.key_length == 0) ||
        (my_shadow_aws_iot_endpoint.root_ca_length == 0) ||
        (strlen(my_shadow_aws_config.name) == 0))
    {
        result = WICED_FALSE;
    }

    /* Unlock the mutex for the provisioning data */
    wiced_rtos_unlock_mutex(&aws_provis_mutex);

    return result;
}

static wiced_result_t save_aws_params (void)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("save_aws_params: failed to read user DCT\n"));
        return WICED_ERROR;
    }

    /* Copy things into the DCT */
    dct_app->aws_report_interval = sd_aws_config.aws_report_interval;
    dct_app->aws_report_queue = sd_aws_config.aws_report_queue;
    dct_app->aws_shadow_update_interval =
        sd_aws_config.aws_shadow_update_interval;

    /* Write the user DCT */
    wiced_dct_write((const void*) dct_app, DCT_APP_SECTION,
            0, sizeof(*dct_app));
    wiced_dct_read_unlock(dct_app, WICED_TRUE);

    return WICED_SUCCESS;
}

static wiced_result_t parse_json_shadow_status(
    wiced_json_object_t * json_object)
{
    int i;
    char value_string[32];

    /* Search our list of keys for a match */
    for (i = 0; i < sizeof(SHADOW_KEYS)/sizeof(SHADOW_KEYS[0]); i++)
    {
        /* If we find a match, save the data */
        if (strncmp(json_object->object_string,
                SHADOW_KEYS[i].key, strlen(SHADOW_KEYS[i].key)) == 0)
        {
            printf ("parse_json_shadow_status: object '%.*s' = '%.*s'\n",
                json_object->object_string_length,
                json_object->object_string,
                json_object->value_length,
                json_object->value);

            /* Copy the value and nul-terminate the string */
            memcpy(value_string, json_object->value, json_object->value_length);
            value_string[json_object->value_length] = '\0';

            /* Indicate that the value has changed in the shadow */
            SHADOW_KEYS[i].changed = 1;

            /* Convert integer or boolean into the uint32_t value */
            if (SHADOW_KEYS[i].value != NULL)
            {
                /* If it starts with a digit, use atoi() */
                if (value_string[0] >= '0' && value_string[0] <= '9')
                {
                    *(SHADOW_KEYS[i].value) = atoi(value_string);
                }
                /* If it starts with a 't' or 'T' assume that it's "true" (1) */
                else if (value_string[0] == 't' || value_string[0] == 'T')
                {
                    *(SHADOW_KEYS[i].value) = 1;
                }
                /* Else, assume "false" (0) */
                else
                {
                    *(SHADOW_KEYS[i].value) = 0;
                }
            }
        }
    }

    return WICED_SUCCESS;
}

static void my_shadow_aws_callback( wiced_aws_handle_t aws, wiced_aws_event_type_t event, wiced_aws_callback_data_t* data )
{
    wiced_result_t result;
    wiced_aws_callback_message_t* message = NULL;
    int i;
    uint8_t change_flags;

    switch ( event )
    {
        case WICED_AWS_EVENT_CONNECTED:
            is_connected = WICED_TRUE;
            printf ("WICED_AWS_EVENT_CONNECTED\n");
            break;

        case WICED_AWS_EVENT_DISCONNECTED:
            is_connected = WICED_FALSE;
            is_subscribed = WICED_FALSE;
            printf ("WICED_AWS_EVENT_DISCONNECTED\n");
            break;

        case WICED_AWS_EVENT_PUBLISHED:
            printf ("WICED_AWS_EVENT_PUBLISHED\n");
            break;

        case WICED_AWS_EVENT_SUBSCRIBED:
            is_subscribed = WICED_TRUE;
            printf ("WICED_AWS_EVENT_SUBSCRIBED\n");
            break;

        case WICED_AWS_EVENT_UNSUBSCRIBED:
            is_subscribed = WICED_FALSE;
            printf ("WICED_AWS_EVENT_UNSUBSCRIBED\n");
            break;

        case WICED_AWS_EVENT_PAYLOAD_RECEIVED:
        {
            /* Make sure that we have a data to parse */
            if (data == NULL)
            {
                break;
            }
            message = &data->message;

            /* Clear all the flags so we know what changed */
            for (i = 0; i < sizeof(SHADOW_KEYS)/sizeof(SHADOW_KEYS[0]); i++)
            {
                SHADOW_KEYS[i].changed = 0;
            }

            /* Parse the JSON delta message */
            result = wiced_JSON_parser( (const char*)message->data , message->data_length );
            if (result != WICED_SUCCESS)
            {
                break;
            }

            /* Perform any actions as a result of a change */
            change_flags = 0;
            for (i = 0; i < sizeof(SHADOW_KEYS)/sizeof(SHADOW_KEYS[0]); i++)
            {
                if (SHADOW_KEYS[i].changed)
                {
                    change_flags |= SHADOW_KEYS[i].update_flag;
                }
            }
            if (change_flags & SHADOW_UPDATE_DCT)
            {
                save_aws_params();
            }
            if (change_flags & SHADOW_UPDATE_LEDS)
            {
                sd_update_leds();
            }

            /* Force a shadow update next time */
            if (change_flags != 0)
            {
                printf ("force shadow update next time\n");
                next_shadow_update_time = 0;
            }
            break;
        }
        default:
            break;
    }
}

static void send_queued_data (void)
{
    AWS_QUEUE_T *t = aws_queue;
    AWS_QUEUE_T *n;
    wiced_result_t result;
    wiced_iso8601_time_t ts;

    /* Acquire the mutex for the queue */
    wiced_rtos_lock_mutex(&(queue_mutex));

    while (t != NULL)
    {
        n = t->next;

        /* Convert time to string */
        wiced_time_convert_utc_ms_to_iso8601(t->timestamp, &ts);

        snprintf (json_string, sizeof(json_string),
            "{ \"time\": \"%.27s\", \"temp\": %.2f, \"hum\": %.2f, "
                 "\"pres\": %.2f, \"iaq\": %.2f, \"iaq_accuracy\": %d, "
                 "\"button_1\": %s, \"button_2\": %s }",
            (char *)&ts,
            t->data.temperature,
            t->data.humidity,
            t->data.pressure,
            t->data.iaq,
            t->data.iaq_accuracy,
            (t->data.button1 == 0) ? "false": "true",
            (t->data.button2 == 0) ? "false": "true");

        printf ("publishing '%s'\n", json_string);

        result = wiced_aws_publish (aws_shadow_handle,
            data_topic, (uint8_t *)json_string, strlen(json_string),
            WICED_AWS_QOS_ATMOST_ONCE);
        if (result != WICED_SUCCESS)
        {
            printf ("wiced_aws_publish (data): %d\n", result);
        }

        /*
         * If this is the last item (most recent) item in the queue,
         * update the shadow if it's time
         */
        if ((n == NULL) &&
            (t->timestamp >= next_shadow_update_time))
        {
            int32_t rssi;
            wwd_wifi_get_rssi(&rssi);

            snprintf (json_string, sizeof(json_string),
                "{ \"state\": { \"reported\": { "
                    "\"firmware_version\": \"%s-%s\", "
                    "\"radio_version\": \"%s\", "
                    "\"radio_rssi\": %ld, "
                    "\"temperature\": %.2f, "
                    "\"humidity\": %.2f, "
                    "\"pressure\": %.2f, "
                    "\"iaq\": %.2f, "
                    "\"iaq_accuracy\": %d, "
                    "\"led_red\": %s, \"led_blue\": %s, \"led_green\": %s, "
                    "\"button_1\": %s, \"button_2\": %s, "
                    "\"report_interval\": %ld, "
                    "\"report_queue\": %ld, "
                    "\"shadow_update_interval\": %ld } } }",
                sd_devinfo.firmwareName, sd_devinfo.firmwareVer,
                wifi_version, rssi,
                t->data.temperature,
                t->data.humidity,
                t->data.pressure,
                t->data.iaq,
                t->data.iaq_accuracy,
                (sd_led_state[0] == 0) ? "false": "true",
                (sd_led_state[1] == 0) ? "false": "true",
                (sd_led_state[2] == 0) ? "false": "true",
                (t->data.button1 == 0) ? "false": "true",
                (t->data.button2 == 0) ? "false": "true",
                sd_aws_config.aws_report_interval,
                sd_aws_config.aws_report_queue,
                sd_aws_config.aws_shadow_update_interval);

            printf ("shadow update '%s'\n", json_string);

            result = wiced_aws_publish (aws_shadow_handle,
                shadow_topic, (uint8_t *)json_string, strlen(json_string),
                WICED_AWS_QOS_ATMOST_ONCE);
            if (result != WICED_SUCCESS)
            {
                printf ("wiced_aws_publish (shadow): %d\n", result);
            }

            /* Update the timer */
            wiced_time_get_utc_time_ms(&next_shadow_update_time);
            next_shadow_update_time +=
                (sd_aws_config.aws_shadow_update_interval * 1000);
        }

        free(t);
        t = n;
    }

    aws_queue = NULL;
    aws_queue_size = 0;
    aws_queue_high_prio = 0;

    /* Release the mutex for the queue */
    wiced_rtos_unlock_mutex(&(queue_mutex));
}

static void set_status (AWS_PROV_STATUS_T new)
{
    /* Do nothing if the status didn't change */
    if (aws_config_prov_status == new)
    {
        return;
    }

    /* Make the connect error state "sticky" */
    if ((new == AWS_PROV_STATUS_UNPROVISIONED) ||
        (new == AWS_PROV_STATUS_CONNECTED) ||
        (aws_config_prov_status != AWS_PROV_STATUS_CONNECT_ERROR))
    {
        aws_config_prov_status = new;
        aws_config_status_notification();
    }
}

static void aws_thread_main (wiced_thread_arg_t arg)
{
    wiced_result_t result;
    int counter;

    /* Loop to handle connect/reconnect */
    while (1)
    {
        /* Wait for provisioning */
        while (is_provisioned() == WICED_FALSE)
        {
            printf ("aws_main_thread: waiting for provisioning\n");
            set_status (AWS_PROV_STATUS_UNPROVISIONED);
            wiced_rtos_delay_milliseconds(3000);
        }

        /* Indicate that we're disconnected */
        set_status (AWS_PROV_STATUS_DISCONNECTED);

        /* If we're queueing messages, wait until we have enough to connect */
        if (sd_aws_config.aws_report_queue >= 2)
        {
            printf ("aws_main_thread: waiting for queue to fill to %lu\n",
                sd_aws_config.aws_report_queue);
            while ((aws_queue_size < sd_aws_config.aws_report_queue) &&
                   (aws_queue_high_prio == 0) &&
                   ((next_shadow_update_time != 0) || (aws_queue_size >= 1)))
            {
                wiced_rtos_delay_milliseconds(100);
            }
        }

        /* Enable WiFi */
        sd_request_wifi (WICED_TRUE);

        /* Wait for WiFi connection to be established before connecting */
        while (sd_is_wifi_up() == WICED_FALSE)
        {
            printf ("aws_main_thread: waiting for WiFi\n");
            wiced_rtos_delay_milliseconds(1000);

            /*
             * No point in breaking out of this loop if WiFi never comes up. If
             * we don't get a network connection, there's nothing to do.
             */
        }

        /* Initialize the library */
        result = wiced_aws_init(&my_shadow_aws_config, my_shadow_aws_callback);
        if( result != WICED_SUCCESS )
        {
            printf ("aws_main_thread: wiced_aws_init failed %d\n", result);

            /* Delay for a bit here. Otherwise could get into a fast loop */
            wiced_rtos_delay_milliseconds(1000);

            set_status(AWS_PROV_STATUS_CONNECT_ERROR);
            if (result == WICED_ALREADY_INITIALIZED)
            {
                goto fail_deinit;
            }
            else
            {
                goto fail_disable_wifi;
            }
        }

        /* Create an endpoint */
        aws_shadow_handle = (wiced_aws_handle_t)
            wiced_aws_create_endpoint(&my_shadow_aws_iot_endpoint);
        if( !aws_shadow_handle )
        {
            printf ("aws_main_thread: wiced_aws_create_endpoint failed %d\n",
                result);

            /* Delay for a bit here. Otherwise could get into a fast loop */
            wiced_rtos_delay_milliseconds(1000);

            set_status(AWS_PROV_STATUS_CONNECT_ERROR);
            goto fail_deinit;
        }

        /* Connect to the server */
        is_connected = WICED_FALSE;
        result = wiced_aws_connect( aws_shadow_handle );
        if ( result != WICED_SUCCESS )
        {
            printf ("aws_main_thread: wiced_aws_connect failed %d\n", result);

            /* Delay for a bit here. Otherwise could get into a fast loop */
            wiced_rtos_delay_milliseconds(1000);

            set_status(AWS_PROV_STATUS_CONNECT_ERROR);
            goto fail_disconnect;
        }

        /* Wait for the connection to complete */
        counter = 0;
        while ((is_connected == WICED_FALSE) &&
               (counter < 30))
        {
            printf ("aws_main_thread: wait for MQTT connection\n");
            wiced_rtos_delay_milliseconds(1000);
            counter++;
        }
        if (is_connected == WICED_FALSE)
        {
            printf ("aws_main_thread: connect never happened\n");
            set_status(AWS_PROV_STATUS_CONNECT_ERROR);
            goto fail_disconnect;
        }

        /* Subscribe to the shadow update topic */
        is_subscribed = WICED_FALSE;
        result = wiced_aws_subscribe (aws_shadow_handle,
            shadow_delta_topic, WICED_AWS_QOS_ATMOST_ONCE );
        if (result != WICED_SUCCESS)
        {
            printf ("aws_main_thread: wiced_aws_subscribe failed %d\n", result);

            /* Delay for a bit here. Otherwise could get into a fast loop */
            wiced_rtos_delay_milliseconds(1000);

            goto fail_disconnect;
        }

        /* Wait for subscribe confirmation */
        counter = 0;
        while ((is_subscribed == WICED_FALSE) &&
               (counter < 30))
        {
            wiced_rtos_delay_milliseconds(1000);
            counter++;
        }
        if (is_subscribed == WICED_FALSE)
        {
            printf ("aws_main_thread: subscribe never happened\n");
            goto fail_unsubscribe;
        }

        /* Indicate that we're connected */
        set_status(AWS_PROV_STATUS_CONNECTED);

        /* Send events as long as we're connected */
        while (is_connected == WICED_TRUE)
        {
            /* If there's something to send, send it */
            if (aws_queue_size > 0)
            {
                send_queued_data();
            }

            /*
             * If we're queueing data messages and we don't have enough,
             * disconnect unless we're supposed to be updating the shadow.
             */
            if ((sd_aws_config.aws_report_queue >= 2) &&
                (aws_queue_size < sd_aws_config.aws_report_queue) &&
                (aws_queue_high_prio == 0) &&
                (next_shadow_update_time != 0))
            {
                printf ("aws_main_thread: closing connection to wait for data\n");
                break;
            }

            /* Wait for a bit */
            wiced_rtos_delay_milliseconds(100);
        }

fail_unsubscribe:
        /* Unsubscribe from the shadow update topic */
        result = wiced_aws_unsubscribe (aws_shadow_handle, shadow_delta_topic);
        if (result != WICED_SUCCESS)
        {
            printf ("aws_main_thread: wiced_aws_unsubscribe failed %d\n",
                result);
        }

        /* Wait for unsubscribe confirmation */
        counter = 0;
        while ((is_subscribed == WICED_TRUE) &&
               (counter < 30))
        {
            wiced_rtos_delay_milliseconds(1000);
            counter++;
        }
        if (is_subscribed == WICED_TRUE)
        {
            printf ("aws_main_thread: unsubscribe never happened\n");
            is_subscribed = WICED_FALSE;
        }

fail_disconnect:
        /* Attempt to disconnect */
        result = wiced_aws_disconnect (aws_shadow_handle);
        if (result != WICED_SUCCESS)
        {
            printf ("aws_main_thread: wiced_aws_disconnect failed %d\n",
                result);
        }

        /* Wait for disconnect */
        counter = 0;
        while ((is_connected == WICED_TRUE) &&
               (counter < 30))
        {
            wiced_rtos_delay_milliseconds(1000);
            counter++;
        }
        if (is_connected == WICED_TRUE)
        {
            printf ("aws_main_thread: disconnect never happened\n");
            is_connected = WICED_FALSE;
        }

fail_deinit:
        /* De-init the library */
        result = wiced_aws_deinit();
        if (result != WICED_SUCCESS)
        {
            printf ("aws_main_thread: wiced_aws_deinit failed %d\n", result);
        }

fail_disable_wifi:
        /* Disable WiFi */
        sd_request_wifi (WICED_FALSE);

        /* Indicate that we're disconnected */
        set_status(AWS_PROV_STATUS_DISCONNECTED);
    }
}

/******************************************************************************
* Global functions
******************************************************************************/

void sd_aws_init (void)
{
    wiced_result_t result;

    /* Initialize a mutex for protecting provisioning data */
    wiced_rtos_init_mutex (&aws_provis_mutex);

    /* Get the WiFi firmware version */
    wwd_wifi_get_wifi_version(wifi_version, sizeof(wifi_version));

    /* Try to parse the configuration data */
    sd_aws_config_updated();

    /* Init JSON parser */
    wiced_JSON_parser_register_callback(parse_json_shadow_status);

    /* Create a thread to take sensor readings */
    wiced_rtos_init_mutex (&queue_mutex);
    result = wiced_rtos_create_thread (
        &aws_thread,
        WICED_APPLICATION_PRIORITY,
        "AWS_thread",
        aws_thread_main,
        16384,
        0);
    if (result != WICED_SUCCESS)
    {
        printf ("failed to create AWS thread\n");
    }
}

void sd_aws_config_updated (void)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;
    platform_dct_security_t* dct_security = NULL;

    /* Mark us as unprovisioned while we're doing this */
    set_status(AWS_PROV_STATUS_UNPROVISIONED);

    /* Lock the mutex for the provisioning data */
    wiced_rtos_lock_mutex(&aws_provis_mutex);

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_FALSE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("[Shadow] Unable to lock DCT to read config\n"));

        /* Unlock the mutex for the provisioning data */
        wiced_rtos_unlock_mutex(&aws_provis_mutex);

        return;
    }

    /* Copy out of the DCT into our local settings structure */
    memcpy(sd_aws_config.ca_cert, dct_app->ca_cert,
        sizeof(sd_aws_config.ca_cert));
    memcpy(sd_aws_config.aws_endpoint, dct_app->aws_endpoint,
        sizeof(sd_aws_config.aws_endpoint));
    memcpy(sd_aws_config.client_id, dct_app->client_id,
        sizeof(sd_aws_config.client_id));
    sd_aws_config.aws_report_interval = dct_app->aws_report_interval;
    sd_aws_config.aws_report_queue = dct_app->aws_report_queue;
    sd_aws_config.aws_shadow_update_interval =
        dct_app->aws_shadow_update_interval;

    wiced_dct_read_unlock (dct_app, WICED_FALSE);

    /* Lock the DCT to allow us to access the certificate and key */
    if (wiced_dct_read_lock (
            (void**)&dct_security, WICED_FALSE, DCT_SECURITY_SECTION,
            0, sizeof(*dct_security)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("[Shadow] Unable to lock DCT to read certificate\n"));

        /* Unlock the mutex for the provisioning data */
        wiced_rtos_unlock_mutex(&aws_provis_mutex);

        return;
    }

    /* Copy out of the DCT into our local settings structure */
    memcpy (sd_aws_config.client_cert, dct_security->certificate,
        sizeof(sd_aws_config.client_cert));
    memcpy (sd_aws_config.client_key, dct_security->private_key,
        sizeof(sd_aws_config.client_key));

    wiced_dct_read_unlock(dct_security, WICED_FALSE);

    /* Fill in the other local structures with the data we have */
    my_shadow_security_creds.certificate = sd_aws_config.client_cert;
    my_shadow_security_creds.certificate_length =
        strlen((char *)sd_aws_config.client_cert);
    my_shadow_security_creds.private_key = sd_aws_config.client_key;
    my_shadow_security_creds.key_length =
        strlen((char *)sd_aws_config.client_key);
    my_shadow_aws_iot_endpoint.root_ca_certificate = sd_aws_config.ca_cert;
    my_shadow_aws_iot_endpoint.root_ca_length =
        strlen((char *)sd_aws_config.ca_cert);
    my_shadow_aws_iot_endpoint.uri = (char *)sd_aws_config.aws_endpoint;
    my_shadow_aws_config.name = (char *)sd_aws_config.client_id;

    /* Build topic strings */
    snprintf (data_topic, sizeof(data_topic),
        "ewb-demo/ewb-devkit/%s/notif", my_shadow_aws_config.name);
    snprintf (shadow_topic, sizeof(shadow_topic),
        "$aws/things/deviceId-%s/shadow/update", my_shadow_aws_config.name);
    snprintf (shadow_delta_topic, sizeof(shadow_delta_topic),
        "$aws/things/deviceId-%s/shadow/update/delta", my_shadow_aws_config.name);

    /*
     * Force disconnect of any active connection so that we can use
     * the (possibly) new settings
     */
    is_connected = WICED_FALSE;
    is_subscribed = WICED_FALSE;

    /* Unlock the mutex for the provisioning data */
    wiced_rtos_unlock_mutex(&aws_provis_mutex);

    /* Fix the state if we have a good configuration */
    if (is_provisioned() == WICED_TRUE)
    {
        set_status(AWS_PROV_STATUS_DISCONNECTED);
    }
}

void sd_aws_update_data(SD_REPORT_DATA_T *data)
{
    AWS_QUEUE_T *q;
    AWS_QUEUE_T *t;

    /* Ignore data messages until we're provisioned */
    if (is_provisioned() == WICED_FALSE)
    {
        return;
    }

    /*
     * Keep the event only if it's time for a report or if it is
     * for a high-priority button event.
     */
    if ((data->timestamp < next_report_time) &&
        ((data->changed_flags & SD_REPORT_CHANGED_BUTTONS)) == 0)
    {
        return;
    }

    /* Allocate memory for a new queue entry */
    q = (AWS_QUEUE_T *)malloc(sizeof(AWS_QUEUE_T));
    if (q == NULL)
    {
        return;
    }

    /* Attach the timestamp to the entry */
    q->timestamp = data->timestamp;

    /* Copy in the new data */
    memcpy(&(q->data), data, sizeof(q->data));

    /* Acquire the mutex for the queue */
    wiced_rtos_lock_mutex(&(queue_mutex));

    /* If the queue is full, discard the oldest item first */
    if (aws_queue_size >= MAX_QUEUE_SIZE)
    {
        if (aws_queue != NULL)
        {
            t = aws_queue;
            aws_queue = t->next;
            free(t);
            aws_queue_size--;
        }
        /* Else, queue size is positive, but pointer is NULL should never happen */
    }

    /* Add the entry to the queue */
    q->next = NULL;
    if (aws_queue == NULL)
    {
        aws_queue = q;
    }
    else
    {
        t = aws_queue;
        while (t->next != NULL)
        {
            t = t->next;
        }

        if (t != NULL)
        {
            t->next = q;
        }
    }

    /* Increment the number of elements in the queue */
    aws_queue_size++;

    /*
     * If the event we just added was for a high-priority button event,
     * increment the count of high-priority events.
     */
    if ((data->changed_flags & SD_REPORT_CHANGED_BUTTONS) != 0)
    {
        aws_queue_high_prio++;
    }

    /* Release the mutex for the queue */
    wiced_rtos_unlock_mutex(&(queue_mutex));

    /* Update the timer */
    wiced_time_get_utc_time_ms(&next_report_time);
    next_report_time += (sd_aws_config.aws_report_interval * 1000);
}
