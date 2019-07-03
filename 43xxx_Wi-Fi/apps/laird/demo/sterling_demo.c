/***************************************************************************************************

 File:     sterling_demo.c
 Author:   Laird Technologies
 Version:  0.1

 Description:  Laird Sterling Module WiFi and BLE demo (STM32F411 Host)

The MIT License (MIT)

Copyright 2016, Laird Technologies (Laird)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sub license, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

****************************************************************************************/

/***************************************************************************************

  Sterling-LWB STM Expansion Board Wi-Fi and Bluetooth Demo

  The comments below are a brief overview of the demo code and its functionality.
  More detailed information is available on the Laird/LSR web site in a
  document titled: STERLING-LWB STM EXPANSION BOARD USER GUIDE

  Also, there is an Android  BLE Configuration and Control Application that is part of
  this demo. It is available on Google Play, named "Laird/LSR ModuleLink Sterling"

  This demo shows WICED Wi-Fi station and BLE peripheral mode operation
  using the LSR Sterling Wi-Fi/BLE module. Configuration of the Wi-Fi
  network and cloud server access parameters over BLE are also demonstrated

  Features demonstrated
   - Wi-Fi client(station) mode
   - BLE peripheral mode
   - Configuration of Wi-Fi network parameters over BLE
   - Configuration of cloud server interface parameters over BLE
   - HTTP connection to a TiWi Connect cloud server

  Application Instructions
  	1. Copy the Sterling Board Platform folder (sterling_demo\LSRSTERLING_00950) to the
  	   WICED platforms folder 43xxx_Wi-Fi\platforms\<a platform folder >

  	2. Download the Sterling Wi-Fi / BLE Configuration App and install it on a BLE capable
  	   Android device. App details are available in the STERLING-LWB STM EXPANSION BOARD USER GUIDE

    3. Build and run the application with the this build string:
       demo.sterling_demo-LSRSTERLING_00950-ThreadX-NetX-SDIO download download_apps run

    4. Open a terminal program and connect it to the WICED USB Serial Port COM Port
       The USB COM port is part of the Sterling Evaluation Board hardware.  The demo
       status/debug output is written to the COM port

    5  Use the Android App to establish a BLE connection with the Sterling Module.
       Select the WiFi network for the module to join enter the pass phrase and save it.
       The App will automatically register the device with the server.

    6. Note the serial output from the module (USB COM port) You should see the module
       connect to the Wi-Fi AP, and start sending http requests to the tiwi connect server.

    The application goes through the following steps
    	- Initialize the ThreadX RTOS and the Wi-Fi interface
    	- Initialize the board state and sensor interfaces
    	- Initialize and bring up the BLE peripheral interface
    	- Wait for Wi-Fi credentials to connect to the network (if necessary)
    	- Initialize and bring up the Wi-Fi STA interface
    	- Continuously loop, reading sensor data
    	- After each read, make an http request to the TiWi Connect server

   Notes
    - In order to connect to the TiWi Connect server, the device must be configured
      using the Android mobile app to configure the API Key
    - The Wi-Fi network parameters are also configured with the Android app
    - Data is only sent to the server on a change that exceeds the threshold

 ****************************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "wiced.h"

#include "sd_common_defs.h"
#include "ble_callbacks.h"
#include "gatt_db.h"
#include "ble_connection.h"
#include "bsec_thread.h"
#include "sd_sterling_demo_dct.h"
#include "sd_user_io.h"
#include "sd_mem_diag.h"
#include "sd_aws.h"
#include "sntp.h"
#include "wifi_config_profile.h"
#include "sterling_demo.h"

/******************************************************************************
* Constants
******************************************************************************/

/* How often to refresh time using SNTP */
#define TIME_SYNC_PERIOD        (3600 * SECONDS)

/* How long can we go without seeing sensor data before rebooting */
#define SENSOR_ERROR_TIMEOUT    (600 * SECONDS)

/* Set of defines for how much data needs to change to count as "changed" */
#define TEMP_REPORT_DELTA        0.1
#define PRES_REPORT_DELTA        10.0
#define HUM_REPORT_DELTA         0.1
#define IAQ_REPORT_DELTA         1.0

/*
 * Timestamp value (in microseconds) for Jan 1, 2018 00:00.
 * This is used to prevent reporting any sensor data before
 * we've had a chance to set the clock using SNTP.
 */
#define START_OF_2018            1514786400000

/******************************************************************************
* Global variables
******************************************************************************/

/* State of the WiFi connection */
WIFI_CONNECTION_STATE_T wifi_connection_state;

/******************************************************************************
* Local variables
******************************************************************************/

static bool bleEnabled = false;

static SD_REPORT_DATA_T last_report_data;
static SD_REPORT_DATA_T report_data;

static wiced_thread_t sensor_thread;

static wiced_bool_t wifi_requested = WICED_FALSE;
static wiced_bool_t sntp_started = WICED_FALSE;

/******************************************************************************
* Local functions
******************************************************************************/

static wiced_result_t wait_for_ble_enabled( void )
{
    int i;

    /* Wait for BLE to come up */ 
    for (i = 0; i < 50; i++ )
    {
        wiced_rtos_delay_milliseconds( 100 );
        if (bleEnabled == true)
        {
            break;
        }
    }

    /* Wait a little longer for things to finish coming up */
    wiced_rtos_delay_milliseconds (1000);

    /* We're successful if BLE is enabled */
    if (bleEnabled == true)
    {
        return WICED_SUCCESS;
    }

    return WICED_TIMEOUT;
}

static void run_wifi_state_machine (void)
{
    wiced_result_t result;

    if (wifi_requested)
    {
        switch (wifi_connection_state)
        {
            case WIFI_CONNECTION_STATE_NOT_PROVISIONED:
                /* Can't do anything if we're not provisioned */
                if (sd_network_is_configured_get() != WICED_FALSE)
                {
                    /* Indicate that we've been provisioned */
                    wifi_connection_state = WIFI_CONNECTION_STATE_DISCONNECTED;
                    wifi_config_connect_state_change();
                }
                break;

            /* WiFi is requested and currently disconnected */
            case WIFI_CONNECTION_STATE_DISCONNECTED:
                /* Indicate that we're trying to connect */
                wifi_connection_state = WIFI_CONNECTION_STATE_CONNECTING;
                wifi_config_connect_state_change();

                /* Bring up the interface */
                result = wiced_network_up(
                    WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
                if (result == WICED_SUCCESS)
                {
                    /* Turn on the status LED */
                    sd_led_state[LED_STATUS] = 1;
                    sd_update_leds();

                    /* Indicate that we're connected now */
                    wifi_connection_state = WIFI_CONNECTION_STATE_CONNECTED;
                    wifi_config_connect_state_change();
                    sd_update_ble_adv_status_flags();

                    /* If this is the first time, start SNTP */
                    if (sntp_started == WICED_FALSE)
                    {
                        sntp_start_auto_time_sync(TIME_SYNC_PERIOD);
                        sntp_started = WICED_TRUE;
                    }
                }
                else
                {
                    /* Disconnected. Try again later. */
                    wifi_connection_state = WIFI_CONNECTION_STATE_DISCONNECTED;
                    wifi_config_connect_state_change();
                }
                break;

            case WIFI_CONNECTION_STATE_CONNECTING:
            case WIFI_CONNECTION_STATE_CONNECTED:
            case WIFI_CONNECTION_STATE_DISCONNECTING:
            default:
                /* Should never reach here */
                break;
        }
    }
    else
    {
        switch (wifi_connection_state)
        {
            case WIFI_CONNECTION_STATE_NOT_PROVISIONED:
            case WIFI_CONNECTION_STATE_DISCONNECTED:
                /* No change */
                break;

            case WIFI_CONNECTION_STATE_CONNECTING:
            case WIFI_CONNECTION_STATE_CONNECTED:
            case WIFI_CONNECTION_STATE_DISCONNECTING:
            default:
                /* Take the interface down */
                result = wiced_network_down(WICED_STA_INTERFACE);
                if (result == WICED_SUCCESS)
                {
                    /* Turn off the status LED */
                    sd_led_state[LED_STATUS] = 0;
                    sd_update_leds();

                    /* Notify that the connection is down */
                    wifi_connection_state = WIFI_CONNECTION_STATE_DISCONNECTED;
                    wifi_config_connect_state_change();
                }
                break;
        }
    }
}

/******************************************************************************
* Global functions
******************************************************************************/

wiced_bool_t sd_is_wifi_up (void)
{
    if (wifi_connection_state == WIFI_CONNECTION_STATE_CONNECTED)
    {
        return WICED_TRUE;
    }
    return WICED_FALSE;
}

void sd_request_wifi (wiced_bool_t enable)
{
    /* Save the request */
    wifi_requested = enable;
}

void sd_notify_ble_enabled (wiced_bool_t enabled)
{
    bleEnabled = enabled;
}

/* @brief main function for the sterling module demo application */
void application_start (void)
{
    wiced_result_t result;
    wiced_utc_time_ms_t now;
    wiced_bool_t send_aws_report;

    /* Initialize the WICED platform */
    result = wiced_init();

    /* Initialize the LEDs */
    memset(sd_led_state, 0, sizeof(sd_led_state));
    sd_update_leds();

    /* display some info from the app DCT */
    result = print_app_dct();

    /* Create a thread to take sensor readings */
    wiced_rtos_init_mutex (&(bsec_thread_data.mutex));
    result = wiced_rtos_create_thread (
        &sensor_thread,
        WICED_APPLICATION_PRIORITY,
        "sensor_thread",
        bsec_thread,
        4096,
        0);
    if (result != WICED_SUCCESS)
    {
        printf ("failed to create sensor thread\n");
    }

    /* Set initial WiFi state */
    wifi_connection_state = WIFI_CONNECTION_STATE_DISCONNECTED;
    if (sd_network_is_configured_get() == WICED_FALSE)
    {
        wifi_connection_state = WIFI_CONNECTION_STATE_NOT_PROVISIONED;
    }

    /* Set the BLE UUID */
    ble_set_primary_service_uuid(
        gatt_db_get_primary_service_uuid(),
        gatt_db_get_primary_service_uuid_len());

    /* Register callback and configuration with the BLE stack */
    wiced_bt_stack_init (
        ble_management_callback,
        &g_wiced_bt_cfg_settings,
        wiced_bt_cfg_buf_pools);

    /* Wait for BLE to come up before doing anything else */
    wait_for_ble_enabled();

    /* Initialize the AWS connection */
    sd_aws_init();
    memset(&last_report_data, 0, sizeof(last_report_data));

    /* Forever loop */
    while(1)
    {
        /* Short delay at each iteration of the main loop */
        wiced_rtos_delay_milliseconds (100);

        /* Manage the WiFi state */
        run_wifi_state_machine();

        /* Check for new sensor data */
        wiced_rtos_lock_mutex(&(bsec_thread_data.mutex));

        /* Don't send sensor data to AWS until we get time from NTP */
        send_aws_report = WICED_TRUE;
        if (bsec_thread_data.timestamp < START_OF_2018)
        {
            send_aws_report = WICED_FALSE;
        }

        /* Check to see if a button push should force us to send a message */
        if ((bsec_thread_data.timestamp == last_report_data.timestamp) &&
            (last_report_data.button1 == sd_button_get_1()) &&
            (last_report_data.button2 == sd_button_get_2()))
        {
            /* Release the mutex */
            wiced_rtos_unlock_mutex(&(bsec_thread_data.mutex));

            /* Check to see if it's been too long since we've seen sensor data */
            if (send_aws_report == WICED_TRUE)
            {
                wiced_time_get_utc_time_ms(&now);
                if ((last_report_data.timestamp > START_OF_2018) &&
                    ((now - last_report_data.timestamp) > SENSOR_ERROR_TIMEOUT))
                {
                    printf ("no sensor data recently (%lld %lld). rebooting.\n",
                        now, last_report_data.timestamp);

                    /* Break out of the main loop to reboot */
                    break;
                }
            }
        }

        /* Else, new sensor data has been received or a button was pushed*/
        else
        {
            /* Assume nothing changed this round */
            report_data.changed_flags = 0;

            /* Get button states */
            report_data.button1 = sd_button_get_1();
            report_data.button2 = sd_button_get_2();

            /* Copy data into the current data structure */
            report_data.timestamp = bsec_thread_data.timestamp;
            report_data.temperature = bsec_thread_data.temperature;
            report_data.pressure = bsec_thread_data.pressure;
            report_data.humidity = bsec_thread_data.humidity;
            report_data.iaq = bsec_thread_data.iaq;
            report_data.iaq_accuracy = bsec_thread_data.iaq_accuracy;

            /* Release the mutex */
            wiced_rtos_unlock_mutex(&(bsec_thread_data.mutex));

            /*
             * We can get here if either the sensor task gave us some new
             * data OR if either of the buttons was pushed. If the sensor
             * data didn't change, skip over some of the code that compares
             * the sensor data.
             */
            if (bsec_thread_data.timestamp != last_report_data.timestamp)
            {
                /* Save the timestamp to check when the data changes */
                last_report_data.timestamp = report_data.timestamp;

                /* Compare temperature to see if we need to report */
                if (fabs(last_report_data.temperature -
                         report_data.temperature) > TEMP_REPORT_DELTA)
                {
                    last_report_data.temperature = report_data.temperature;
                    report_data.changed_flags |= SD_REPORT_CHANGED_TEMPERATURE;
                }

                /* Compare pressure to see if we need to report */
                if (fabs(last_report_data.pressure -
                         report_data.pressure) > PRES_REPORT_DELTA)
                {
                    last_report_data.pressure = report_data.pressure;
                    report_data.changed_flags |= SD_REPORT_CHANGED_PRESSURE;
                }

                /* Compare humidity to see if we need to report */
                if (fabs(last_report_data.humidity -
                         report_data.humidity) > HUM_REPORT_DELTA)
                {
                    last_report_data.humidity = report_data.humidity;
                    report_data.changed_flags |= SD_REPORT_CHANGED_HUMIDITY;
                }

                /* Compare IAQ to see if we need to report */
                if (fabs(last_report_data.iaq -
                         report_data.iaq) > IAQ_REPORT_DELTA)
                {
                    last_report_data.iaq = report_data.iaq;
                    last_report_data.iaq_accuracy = report_data.iaq_accuracy;
                    report_data.changed_flags |= SD_REPORT_CHANGED_IAQ;
                }
            }

            /* Compare button states to see if we need to report */
            if (last_report_data.button1 != report_data.button1)
            {
                last_report_data.button1 = report_data.button1;
                report_data.changed_flags |= SD_REPORT_CHANGED_BUTTONS;
            }
            if (last_report_data.button2 != report_data.button2)
            {
                last_report_data.button2 = report_data.button2;
                report_data.changed_flags |= SD_REPORT_CHANGED_BUTTONS;
            }

            /* Report changes to BLE and AWS routines */
            simple_sensor_environment_send_notificaton(&report_data);
            if (send_aws_report == WICED_TRUE)
            {
                sd_aws_update_data(&report_data);
            }
        }
    }

    /* If we've broken out of the loop above, just reboot */
    wiced_framework_reboot();

    return;
}

