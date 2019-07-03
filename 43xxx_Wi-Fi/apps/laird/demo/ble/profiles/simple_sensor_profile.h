/***************************************************************************************************

 File:     simple_sensor_profile.h
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
#pragma once

#include "ble_connection.h"
//#include "sd_sensors.h"
#include "sd_user_io.h"

#include "sterling_demo.h"

wiced_bt_gatt_status_t simple_sensor_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data );
wiced_bt_gatt_status_t simple_sensor_confirmation_handler( ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle );

void simple_sensor_environment_send_notificaton (SD_REPORT_DATA_T *data);

wiced_result_t ble_simple_sensor_profile_init(void);

typedef struct
{
    uint16_t temperature;
    uint32_t pressure;
    uint16_t humidity;
    uint8_t iaq[3];
} SS_PROFILE_DATA_T;

/* UUID for the environmental data service 1ac59fa2-c727-11e8-8b43-13f22a638e55 */
#define UUID_SIMPLE_SENSOR_ENVIRONMENT_SERVICE  0x55, 0x8e, 0x63, 0x2a, 0xf2, 0x13, 0x43, 0x8b, 0xe8, 0x11, 0x27, 0xc7, 0xa2, 0x9f, 0xc5, 0x1a
/* UUID for the temperature characteristic a60dd9cc-c730-11e8-9326-37efdb4415c6 */
#define UUID_SIMPLE_SENSOR_ENVIRONMENT_TEMP     0xc6, 0x15, 0x44, 0xdb, 0xef, 0x37, 0x26, 0x93, 0xe8, 0x11, 0x30, 0xc7, 0xcc, 0xd9, 0x0d, 0xa6
/* UUID for the pressure characteristic be8c7d5a-c730-11e8-a328-8bb557a3f2b8 */
#define UUID_SIMPLE_SENSOR_ENVIRONMENT_PRESSURE 0xb8, 0xf2, 0xa3, 0x57, 0xb5, 0x8b, 0x28, 0xa3, 0xe8, 0x11, 0x30, 0xc7, 0x5a, 0x7d, 0x8c, 0xbe
/* UUID for the humidity characteristic bed49edc-c730-11e8-b1e1-278d4cc2e61a */
#define UUID_SIMPLE_SENSOR_ENVIRONMENT_HUMIDITY 0x1a, 0xe6, 0xc2, 0x4c, 0x8d, 0x27, 0xe1, 0xb1, 0xe8, 0x11, 0x30, 0xc7, 0xdc, 0x93, 0xd4, 0xbe
/* UUID for the air quality characteristic bf1b508e-c730-11e8-a16f-43dd4ade3623 */
#define UUID_SIMPLE_SENSOR_ENVIRONMENT_IAQ      0x23, 0x36, 0xde, 0x4a, 0xdd, 0x43, 0x6f, 0xa1, 0xe8, 0x11, 0x30, 0xc7, 0x8e, 0x50, 0x1b, 0xbf

extern uint16_t sd_env_temp_data_cfg;
extern uint16_t sd_env_pressure_data_cfg;
extern uint16_t sd_env_humidity_data_cfg;
extern uint16_t sd_env_iaq_data_cfg;
extern SS_PROFILE_DATA_T ss_data;

/* Declare the Characteristic Descriptors for the environment Service */
#define SIMPLE_SENSOR_ENVIRONMENT_SERVICE_ATTRIBUTE_DECLARATIONS \
    char simple_sensor_env_temp_desc[] = "Temperature"; \
    char simple_sensor_env_pressure_desc[] = "Pressure"; \
    char simple_sensor_env_humidity_desc[] = "Humidity"; \
    char simple_sensor_env_iaq_desc[] = "Air Quality";

/* Declare the Attribute Table entries for the environment Service */
#define SIMPLE_SENSOR_ENVIRONMENT_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_VAL,         sizeof(ss_data.temperature), &(ss_data.temperature) }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_DESCRIPTION, sizeof(simple_sensor_env_temp_desc), simple_sensor_env_temp_desc }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_CFG_DESC,    sizeof(sd_env_temp_data_cfg), &sd_env_temp_data_cfg }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_VAL,         sizeof(ss_data.pressure), &(ss_data.pressure) }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_DESCRIPTION, sizeof(simple_sensor_env_pressure_desc), simple_sensor_env_pressure_desc }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_CFG_DESC,    sizeof(sd_env_pressure_data_cfg), &sd_env_pressure_data_cfg }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_VAL,          sizeof(ss_data.humidity), &(ss_data.humidity) }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_DESCRIPTION,  sizeof(simple_sensor_env_humidity_desc), simple_sensor_env_humidity_desc }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_CFG_DESC,     sizeof(sd_env_humidity_data_cfg), &sd_env_humidity_data_cfg }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_VAL,          sizeof(ss_data.iaq), &(ss_data.iaq) }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_DESCRIPTION,  sizeof(simple_sensor_env_iaq_desc), simple_sensor_env_iaq_desc }, \
        { HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_CFG_DESC,     sizeof(sd_env_iaq_data_cfg), &sd_env_iaq_data_cfg }

/* Declare the handles for the eCompass Service */
#define HANDLE_SIMPLE_SENSOR_PROFILE_ENVIRONMENT_SERVICE(baseHandle) \
		HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVICE = baseHandle, \
\
    	HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_VAL, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_DESCRIPTION, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_CFG_DESC, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_VAL, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_DESCRIPTION, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_CFG_DESC, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_VAL, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_DESCRIPTION, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_CFG_DESC, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_VAL, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_DESCRIPTION, \
        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_CFG_DESC,

/* Declare the Simple Sensor eCompass Service and its Characteristics for entry into the GATT DB */
#define SIMPLE_SENSOR_ENVIRONMENT_SERVICE_DECLARATION \
	    /* Declare proprietary environment service with 128 byte UUID */ \
	    PRIMARY_SERVICE_UUID128( \
	            HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVICE,\
	            UUID_SIMPLE_SENSOR_ENVIRONMENT_SERVICE ), \
\
			/* Declare "Temperature" characteristic */ \
			CHARACTERISTIC_UUID128(\
			        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP,\
			        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_VAL,\
			        UUID_SIMPLE_SENSOR_ENVIRONMENT_TEMP,\
					LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE ), \
\
				/* User description for this characteristic */ \
				CHAR_DESCRIPTOR_UUID16_VARIABLE(\
				        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_DESCRIPTION,\
				        GATT_UUID_CHAR_DESCRIPTION, \
				        LEGATTDB_PERM_READABLE, sizeof(simple_sensor_env_temp_desc) ), \
\
				/* Declare client characteristic configuration descriptor \
				 * Value of the descriptor can be modified by the client  \
				 * Value modified shall be retained during connection and across connection \
				 * for bonded devices.  Setting value to 1 tells this application to send notification \
				 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
				CHAR_DESCRIPTOR_UUID16_WRITABLE(\
				        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_TEMP_CFG_DESC,\
				        GATT_UUID_CHAR_CLIENT_CONFIG, \
				        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),\
\
            /* Declare "Pressure" characteristic */ \
            CHARACTERISTIC_UUID128(\
                    HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES,\
                    HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_VAL,\
                    UUID_SIMPLE_SENSOR_ENVIRONMENT_PRESSURE,\
                    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE ), \
\
                /* User description for this characteristic */ \
                CHAR_DESCRIPTOR_UUID16_VARIABLE(\
                        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_DESCRIPTION,\
                        GATT_UUID_CHAR_DESCRIPTION, \
                        LEGATTDB_PERM_READABLE, sizeof(simple_sensor_env_pressure_desc) ), \
\
                /* Declare client characteristic configuration descriptor \
                 * Value of the descriptor can be modified by the client  \
                 * Value modified shall be retained during connection and across connection \
                 * for bonded devices.  Setting value to 1 tells this application to send notification \
                 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
                CHAR_DESCRIPTOR_UUID16_WRITABLE(\
                        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_PRES_CFG_DESC,\
                        GATT_UUID_CHAR_CLIENT_CONFIG, \
                        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),\
 \
            /* Declare "Humidity" characteristic */ \
            CHARACTERISTIC_UUID128(\
                  HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM,\
                  HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_VAL,\
                  UUID_SIMPLE_SENSOR_ENVIRONMENT_HUMIDITY,\
                  LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE ), \
\
              /* User description for this characteristic */ \
              CHAR_DESCRIPTOR_UUID16_VARIABLE(\
                      HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_DESCRIPTION,\
                      GATT_UUID_CHAR_DESCRIPTION, \
                      LEGATTDB_PERM_READABLE, sizeof(simple_sensor_env_humidity_desc) ), \
\
              /* Declare client characteristic configuration descriptor \
               * Value of the descriptor can be modified by the client  \
               * Value modified shall be retained during connection and across connection \
               * for bonded devices.  Setting value to 1 tells this application to send notification \
               * when value of the characteristic changes.  Value 2 is to allow indications. */ \
              CHAR_DESCRIPTOR_UUID16_WRITABLE(\
                      HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_HUM_CFG_DESC,\
                      GATT_UUID_CHAR_CLIENT_CONFIG, \
                      LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),\
\
            /* Declare "IAQ" characteristic */ \
            CHARACTERISTIC_UUID128(\
                    HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ,\
                    HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_VAL,\
                    UUID_SIMPLE_SENSOR_ENVIRONMENT_IAQ,\
                    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE ), \
\
                /* User description for this characteristic */ \
                CHAR_DESCRIPTOR_UUID16_VARIABLE(\
                        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_DESCRIPTION,\
                        GATT_UUID_CHAR_DESCRIPTION, \
                        LEGATTDB_PERM_READABLE, sizeof(simple_sensor_env_iaq_desc) ), \
\
                /* Declare client characteristic configuration descriptor \
                 * Value of the descriptor can be modified by the client  \
                 * Value modified shall be retained during connection and across connection \
                 * for bonded devices.  Setting value to 1 tells this application to send notification \
                 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
                CHAR_DESCRIPTOR_UUID16_WRITABLE(\
                        HANDLE_SIMPLE_SENSOR_ENVIRONMENT_SERVER_CHAR_IAQ_CFG_DESC,\
                        GATT_UUID_CHAR_CLIENT_CONFIG, \
                        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ)



// ********************************* Simple Sensor Profile - Status LED Service ******************************************

/* UUID value of the Simple Sensor Status LED Service */ // 98038100-3b0f-4043-a704-6ffe2130c1f0
#define UUID_SIMPLE_SENSOR_STATUS_LED_SERVICE              0xf0, 0xc1, 0x30, 0x21, 0xfe, 0x6f, 0x04, 0xa7, 0x43, 0x40, 0x0f, 0x3b, 0x00, 0x81, 0x03, 0x98
/* UUID value of the LED1 Characteristic, Value Desc Cfg */ // 9e02d932-1359-4341-becf-4c923aad4e08
#define UUID_SIMPLE_SENSOR_STATUS_LED_LED1_CHARACTERISTIC 0x08, 0x4e, 0xad, 0x3a, 0x92, 0x4c, 0xcf, 0xbe, 0x41, 0x43, 0x59, 0x13, 0x32, 0xd9, 0x02, 0x9e
/* UUID value of the LED2 Characteristic, Value Desc Cfg */ // 127b3ca6-d4d7-4178-967e-bf094bd36633
#define UUID_SIMPLE_SENSOR_STATUS_LED_LED2_CHARACTERISTIC 0x33, 0x66, 0xd3, 0x4b, 0x09, 0xbf, 0x7e, 0x96, 0x78, 0x41, 0xd7, 0xd4, 0xa6, 0x3c, 0x7b, 0x12
/* UUID value of the LED3 Characteristic, Value Desc Cfg */ // ae554c11-4fc2-4434-aa24-2842ac2f9ffd
#define UUID_SIMPLE_SENSOR_STATUS_LED_LED3_CHARACTERISTIC 0xfd, 0x9f, 0x2f, 0xac, 0x42, 0x28, 0x24, 0xaa, 0x34, 0x44, 0xc2, 0x4f, 0x11, 0x4c, 0x55, 0xae

/* Declare the Characteristic Descriptors for the Simple Sensor Status LED Service */
#define SIMPLE_SENSOR_STATUS_LED_SERVICE_ATTRIBUTE_DECLARATIONS \
		char simple_sensor_status_led_led1_desc[]   = { 'L', 'E', 'D', '1', 0, }; \
		char simple_sensor_status_led_led2_desc[]   = { 'L', 'E', 'D', '2', 0, }; \
		char simple_sensor_status_led_led3_desc[]   = { 'L', 'E', 'D', '3', 0, };

/* Declare the Attribute Table entries for the Status LED Service */
#define SIMPLE_SENSOR_STATUS_LED_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_VAL,            sizeof(sd_led_state[0]), (void*)&(sd_led_state[0]) }, \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_DESCRIPTION,    sizeof(simple_sensor_status_led_led1_desc), (void*)simple_sensor_status_led_led1_desc }, \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_VAL,            sizeof(sd_led_state[1]), (void*)&(sd_led_state[1]) }, \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_DESCRIPTION,    sizeof(simple_sensor_status_led_led2_desc), (void*)simple_sensor_status_led_led2_desc }, \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_VAL,            sizeof(sd_led_state[2]), (void*)&(sd_led_state[2]) }, \
		{ HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_DESCRIPTION,    sizeof(simple_sensor_status_led_led3_desc), (void*)simple_sensor_status_led_led3_desc }

/* Declare the handles for the Simple Sensor Status LED Service */
#define HANDLE_SIMPLE_SENSOR_PROFILE_STATUS_LED_SERVICE(baseHandle) \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE = baseHandle, \
		\
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_VAL, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_DESCRIPTION, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_VAL, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_DESCRIPTION, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_VAL, \
		HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_DESCRIPTION,



/* Declare the Simple Sensor Status LED Service and its Characteristics for entry into the GATT DB */
#define SIMPLE_SENSOR_STATUS_LED_SERVICE_DECLARATION \
PRIMARY_SERVICE_UUID128( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE, UUID_SIMPLE_SENSOR_STATUS_LED_SERVICE ), \
\
		/* Declare "LED1" characteristic */ \
		CHARACTERISTIC_UUID128_WRITABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1, HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_VAL, \
				UUID_SIMPLE_SENSOR_STATUS_LED_LED1_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ ), \
\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED1_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(simple_sensor_status_led_led1_desc) ), \
\
		/* Declare "LED2" characteristic */ \
		CHARACTERISTIC_UUID128_WRITABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2, HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_VAL, \
				UUID_SIMPLE_SENSOR_STATUS_LED_LED2_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ ), \
\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED2_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(simple_sensor_status_led_led2_desc) ), \
\
		/* Declare "LED3" characteristic */ \
		CHARACTERISTIC_UUID128_WRITABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3, HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_VAL, \
				UUID_SIMPLE_SENSOR_STATUS_LED_LED3_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ ), \
\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_SIMPLE_SENSOR_STATUS_LED_SERVICE_CHAR_LED3_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(simple_sensor_status_led_led3_desc) )
