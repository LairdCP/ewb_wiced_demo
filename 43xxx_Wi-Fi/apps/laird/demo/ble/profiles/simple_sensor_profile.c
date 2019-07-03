/***************************************************************************************************

 File:     simple_sensor_profile.c
 Author:   Laird Technologies
 Version:  0.1

 Description:  implementation of a "simple sensor" BLE profile

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

***************************************************************************************************/
#include "wiced.h"
#include "sd_common_defs.h"

#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"

#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"

#include "simple_sensor_profile.h"
#include "sd_user_io.h"
#include "gatt_db.h"

#include "sterling_demo.h"

/* Simple Sensor Profile -- environment service attributes */
uint16_t sd_env_temp_data_cfg;
uint16_t sd_env_pressure_data_cfg;
uint16_t sd_env_humidity_data_cfg;
uint16_t sd_env_iaq_data_cfg;
SS_PROFILE_DATA_T ss_data;

// Keep a reference to the connection id when the peer writes to the cfg desc
static uint16_t last_conn_id = 0;

// local function prototypes
/* none */

/*
 * Write request handler for the Simple Sensor Profile
 */
wiced_bt_gatt_status_t simple_sensor_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data )
{
    wiced_bt_gatt_status_t result    = WICED_BT_GATT_SUCCESS;
    uint8_t                *p_attr   = p_data->p_val;
    last_conn_id = conn_id;

    switch ( p_data->handle )
    {
    case HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_CFG_DESC:
        if ( p_data->val_len != sizeof(sd_env_temp_data_cfg) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        sd_env_temp_data_cfg = p_attr[0] | ( p_attr[1] << 8 );
        break;

    case HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_CFG_DESC:
        if ( p_data->val_len != sizeof(sd_env_pressure_data_cfg) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        sd_env_pressure_data_cfg = p_attr[0] | ( p_attr[1] << 8 );
        break;

    case HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_CFG_DESC:
        if ( p_data->val_len != sizeof(sd_env_humidity_data_cfg) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        sd_env_humidity_data_cfg = p_attr[0] | ( p_attr[1] << 8 );
        break;

    case HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_CFG_DESC:
        if ( p_data->val_len != sizeof(sd_env_iaq_data_cfg) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        sd_env_iaq_data_cfg = p_attr[0] | ( p_attr[1] << 8 );
        break;

	case HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_VAL:
		if (p_data->val_len != sizeof(sd_led_state[0]))
		{
			return WICED_BT_GATT_INVALID_ATTR_LEN;
		}
		memcpy(&(sd_led_state[0]), p_attr, p_data->val_len);
		sd_update_leds();
		break;

	case HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_VAL:
        if (p_data->val_len != sizeof(sd_led_state[1]))
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        memcpy(&(sd_led_state[1]), p_attr, p_data->val_len);
        sd_update_leds();
		break;

	case HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_VAL:
        if (p_data->val_len != sizeof(sd_led_state[2]))
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        memcpy(&(sd_led_state[2]), p_attr, p_data->val_len);
        sd_update_leds();
		break;

    default:
        result = WICED_BT_GATT_INVALID_HANDLE;
        break;
    }
    return result;
}

/*
 * Handle a confirmation message from a peer
 */
wiced_bt_gatt_status_t simple_sensor_confirmation_handler( ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle )
{
    if ( !ble_connection_flags->flag_indication_sent )
    {
        WPRINT_BT_APP_INFO(("Wrong Confirmation!"));
        return WICED_BT_GATT_SUCCESS;
    }
    ble_connection_flags->flag_indication_sent = 0;

    return WICED_BT_GATT_SUCCESS;
}

/*
 * Initialize the structures for the Simple Sensor profile
 */
wiced_result_t ble_simple_sensor_profile_init(void)
{
	last_conn_id = 0;
	return WICED_SUCCESS;
}

/*
 * Send a notification if the peer has enabled notification for acceleration data
 */
void simple_sensor_environment_send_notificaton (SD_REPORT_DATA_T *data)
{
    uint16_t iaq_val;

    /* Make a copy of the data */
    ss_data.temperature = data->temperature * 100;
    ss_data.pressure = data->pressure;
    ss_data.humidity = data->humidity * 100;
    iaq_val = data->iaq * 100;
    ss_data.iaq[0] = iaq_val & 0xff;
    ss_data.iaq[1] = iaq_val >> 8;
    ss_data.iaq[2] = data->iaq_accuracy;

    /* If there hasn't been a connection, do nothing */
    if (last_conn_id == 0)
    {
        return;
    }

    /* Check for temperature notification */
    if (((data->changed_flags & SD_REPORT_CHANGED_TEMPERATURE) != 0) &&
        ((sd_env_temp_data_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification(
                last_conn_id,
                HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_VAL,
                sizeof(ss_data.temperature), (uint8_t *)&(ss_data.temperature));
    }

    /* Check for pressure notification */
    if (((data->changed_flags & SD_REPORT_CHANGED_PRESSURE) != 0) &&
        ((sd_env_pressure_data_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification(
                last_conn_id,
                HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_VAL,
                sizeof(ss_data.pressure), (uint8_t *)&(ss_data.pressure));
    }

    /* Check for humidity notification */
    if (((data->changed_flags & SD_REPORT_CHANGED_HUMIDITY) != 0) &&
        ((sd_env_humidity_data_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification(
                last_conn_id,
                HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_VAL,
                sizeof(ss_data.humidity), (uint8_t *)&(ss_data.humidity));
    }

    /* Check for gas resistance notification */
    if (((data->changed_flags & SD_REPORT_CHANGED_IAQ) != 0) &&
        ((sd_env_iaq_data_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification(
                last_conn_id,
                HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_VAL,
                sizeof(ss_data.iaq), (uint8_t *)&(ss_data.iaq));
    }
}
