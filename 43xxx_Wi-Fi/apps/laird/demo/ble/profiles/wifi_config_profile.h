/***************************************************************************************************

 File:     wifi_config_profile.h
 Author:   Laird Technologies
 Version:  0.1

 Description:  implementation of the "WiFi Config" BLE profile

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

#include "sterling_demo.h"

#include "ble_connection.h"

wiced_bt_gatt_status_t wifi_config_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data );
void wifi_config_scan_mode_write_handler(void);
wiced_bt_gatt_status_t wifi_config_confirmation_handler( ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle );
void wifi_config_ap_parameters_write_handler(void);
void wifi_config_connect_state_change (void);

#define SSID_DETAILS_STATE_NO_DETAILS (0)
#define SSID_DETAILS_STATE_STALE      (1)
#define SSID_DETAILS_STATE_CURRENT    (2)

#define WIFI_CONFIG_SCAN_MODE_START_SCAN (1)
#define WIFI_CONFIG_SCAN_MODE_DEFAULT       (0)
#define WIFI_CONFIG_SCAN_MODE_SCAN_RUNNING  (1)
#define WIFI_CONFIG_SCAN_MODE_SCAN_DONE     (2)

// ********************************* Wi-Fi Config Profile - Scan Service *********************************************
/* UUID value of the Wi-Fi Scan Service */
#define UUID_SCAN_SERVICE                  0xa0, 0x97, 0xfa, 0xee, 0x77, 0x74, 0x34, 0xbd, 0x97, 0x42, 0x1c, 0xa2, 0xfd, 0xad, 0xc8, 0x78
/* UUID value of the Scanning Mode Characteristic, Value Desc Config */
#define UUID_SCANNING_MODE_CHARACTERISTIC  0x27, 0xf6, 0x69, 0x91, 0x68, 0xee, 0xc2, 0xbe, 0x44, 0x4d, 0xb9, 0x5c, 0x3f, 0x2d, 0xc3, 0x8a
/* UUID value of the AP Count Characteristic, Value Desc */
#define UUID_AP_COUNT_CHARACTERISTIC       0x28, 0xf6, 0x69, 0x91, 0x68, 0xee, 0xc2, 0xbe, 0x44, 0x4d, 0xb9, 0x5c, 0x3f, 0x2d, 0xc3, 0x8a
/* UUID value of the Scanning Mode Characteristic, Value Desc */
#define UUID_AP_DETAILS_CHARACTERISTIC     0x29, 0xf6, 0x69, 0x91, 0x68, 0xee, 0xc2, 0xbe, 0x44, 0x4d, 0xb9, 0x5c, 0x3f, 0x2d, 0xc3, 0x8a

/* Declare the Characteristic Descriptors for the Wi-Fi Config Scan Service */
#define WIFI_CONFIG_SCAN_SERVICE_ATTRIBUTE_DECLARATIONS \
	char wifi_config_char_scan_mode_desc[]         = { 'S', 'c', 'a', 'n', 'n', 'i', 'n', 'g', ' ', 'M', 'o', 'd', 'e', 0, }; \
	char wifi_config_char_ap_count_desc[]          = { 'A', 'P', ' ', 'C', 'o', 'u', 'n', 't', 0, }; \
	char wifi_config_char_ap_details_desc[]        = { 'A', 'P', ' ', 'D', 'e', 't', 'a', 'i', 'l', 's', 0, };

/* Declare a structure to store the AP details for the "AP Details" characteristic */
#pragma pack(push,1)
typedef PACKED struct
{
	uint8_t state;
	uint8_t channel;
	uint8_t freqBand;
	int8_t rssi;
	uint32_t securityMode;
	uint8_t ssidLength;
	char ssid[32];
} wifi_config_ap_details_t;
#pragma pack(pop)

/* Declare externs for the Attribute variables declared by the Scan Service (see the wifi_config_profile.c file for definitions) */
extern uint8_t wifi_config_char_scan_mode;
extern uint16_t wifi_config_char_scan_mode_cfg;
extern uint8_t wifi_config_char_ap_count;
extern wifi_config_ap_details_t wifi_config_char_ap_details;
extern uint16_t wifi_config_char_ap_details_cfg;

/* Declare the Attribute Table entries for the Scan Service */
#define WIFI_CONFIG_SCAN_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
	    { HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL,          sizeof(wifi_config_char_scan_mode),        &wifi_config_char_scan_mode }, \
		{ HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_DESCRIPTION,  sizeof(wifi_config_char_scan_mode_desc),   (void*)wifi_config_char_scan_mode_desc }, \
	    { HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_CFG_DESC,     sizeof(wifi_config_char_scan_mode_cfg),    &wifi_config_char_scan_mode_cfg }, \
	    { HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_VAL,           sizeof(wifi_config_char_ap_count),         &wifi_config_char_ap_count }, \
		{ HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_DESCRIPTION,   sizeof(wifi_config_char_ap_count_desc),    wifi_config_char_ap_count_desc }, \
	    { HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL,         sizeof(wifi_config_char_ap_details),       &wifi_config_char_ap_details }, \
		{ HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_DESCRIPTION, sizeof(wifi_config_char_ap_details_desc),  wifi_config_char_ap_details_desc }, \
		{ HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_CFG_DESC,    sizeof(wifi_config_char_ap_details_cfg),   &wifi_config_char_ap_details_cfg }

/* Declare the handles for the Wi-Fi Config Scan Service */
#define HANDLE_WIFI_CONFIG_PROFILE_SCAN_SERVICE(baseHandle) \
		HANDLE_WCONF_SCAN_SERVICE = baseHandle, \
    	\
		HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_DESCRIPTION, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_CFG_DESC, \
		\
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_VAL, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_DESCRIPTION, \
		\
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_DESCRIPTION, \
		HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_CFG_DESC,

/* Declare the Wi-Fi Config Scan Service and its Characteristics for entry into the GATT DB */
#define WIFI_CONFIG_SCAN_SERVICE_DECLARATION \
	    /* Declare proprietary Wi-Fi Scan Service with 128 byte UUID */ \
	    PRIMARY_SERVICE_UUID128( HANDLE_WCONF_SCAN_SERVICE, UUID_SCAN_SERVICE ), \
\
				/* Declare "Scanning Mode" characteristic to start a Wi-Fi scan */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE, HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL, \
						UUID_SCANNING_MODE_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY, \
						LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(wifi_config_char_scan_mode_desc) ), \
\
					/* Declare client characteristic configuration descriptor \
					 * Value of the descriptor can be modified by the client  \
					 * Value modified shall be retained during connection and across connection \
					 * for bonded devices.  Setting value to 1 tells this application to send notification \
					 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
					CHAR_DESCRIPTOR_UUID16_WRITABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_CFG_DESC, GATT_UUID_CHAR_CLIENT_CONFIG, \
						LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ), \
\
				/* Declare "AP Count" characteristic */ \
				CHARACTERISTIC_UUID128( HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT, HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_VAL, \
						UUID_AP_COUNT_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ), \
\
						/* User description for this characteristic */ \
						CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_COUNT_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
							LEGATTDB_PERM_READABLE, sizeof(wifi_config_char_ap_count_desc) ), \
\
				/* Declare "AP Details" characteristic */ \
				CHARACTERISTIC_UUID128( HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS, HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL, \
						UUID_AP_DETAILS_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, \
						LEGATTDB_PERM_READABLE ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(wifi_config_char_ap_details_desc) ), \
\
					/* Declare client characteristic configuration descriptor \
					 * Value of the descriptor can be modified by the client  \
					 * Value modified shall be retained during connection and across connection \
					 * for bonded devices.  Setting value to 1 tells this application to send notification \
					 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
					CHAR_DESCRIPTOR_UUID16_WRITABLE( HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_CFG_DESC, GATT_UUID_CHAR_CLIENT_CONFIG, \
						LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ)



// ********************************* Wi-Fi Config Profile - Connect Service ******************************************

/* UUID value of the Wi-Fi Connect Service */
#define UUID_CONNECT_SERVICE                 0x30, 0x26, 0x34, 0xa6, 0x2a, 0x1d, 0xca, 0xa0, 0x60, 0x48, 0x2c, 0x0d, 0x56, 0x3c, 0x9d, 0xc5
/* UUID of the Connection State Characteristic, Value Desc Config */
#define UUID_CONNECTION_STATE_CHARACTERISTIC 0x56, 0x43, 0xba, 0xe9, 0x6c, 0x43, 0x91, 0x90, 0x2f, 0x47, 0x51, 0xca, 0xdd, 0x6d, 0xe2, 0x3e
/* UUID of the AP Parameters Characteristic, Value Desc */
#define UUID_AP_PARAMETERS_CHARACTERISTIC    0x93, 0x35, 0x03, 0xb2, 0x0c, 0x60, 0x21, 0xab, 0x34, 0x49, 0xb9, 0x23, 0x80, 0x02, 0xed, 0x98

/* Declare the Characteristic Descriptors for the Wi-Fi Config Connect Service */
#define WIFI_CONFIG_CONNECT_SERVICE_ATTRIBUTE_DECLARATIONS \
	char wifi_config_char_connection_state_desc[]  = { 'C', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', ' ', 'S', 't', 'a', 't', 'e', 0, }; \
	char wifi_config_char_ap_parameters_desc[]     = { 'A', 'P', ' ', 'P', 'a', 'r', 'a', 'm', 'e', 't', 'e', 'r', 's', 0, };

/* Declare a structure to store the AP parameters for the "AP Parameters" characteristic */
#pragma pack(push,1)
typedef PACKED struct
{
	uint32_t securityMode;
	uint8_t ssidLength;
	char ssid[32];
	uint8_t passphraseLength;
	char passphrase[63];
} wifi_config_ap_params_t;
#pragma pack(pop)

/* Declare externs for the Attribute variables declared by the Connect Service (see the wifi_config_profile.c file for definitions) */
extern uint8_t wifi_config_char_connection_state;
extern uint16_t wifi_config_char_connection_state_cfg;
extern wifi_config_ap_params_t wifi_config_char_ap_parameters;

/* Declare the Attribute Table entries for the Scan Service */
#define WIFI_CONFIG_CONNECT_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
		{ HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL,         sizeof(wifi_connection_state), &wifi_connection_state }, \
		{ HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_DESCRIPTION, sizeof(wifi_config_char_connection_state_desc), wifi_config_char_connection_state_desc }, \
		{ HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_CFG_DESC,    sizeof(wifi_config_char_connection_state_cfg), &wifi_config_char_connection_state_cfg }, \
		{ HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_VAL,            sizeof(wifi_config_char_ap_parameters), &wifi_config_char_ap_parameters }, \
		{ HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_DESCRIPTION,    sizeof(wifi_config_char_ap_parameters_desc), wifi_config_char_ap_parameters_desc }

/* Declare the handles for the Wi-Fi Config Connect Service */
#define HANDLE_WIFI_CONFIG_PROFILE_CONNECT_SERVICE(baseHandle) \
		HANDLE_WCONF_CONNECT_SERVICE = baseHandle, \
		\
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE, \
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL, \
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_DESCRIPTION, \
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_CFG_DESC, \
		\
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS, \
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_VAL, \
		HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_DESCRIPTION,

/* Declare the Wi-Fi Config Connect Service and its Characteristics for entry into the GATT DB */
#define WIFI_CONFIG_CONNECT_SERVICE_DECLARATION \
PRIMARY_SERVICE_UUID128( HANDLE_WCONF_CONNECT_SERVICE, UUID_CONNECT_SERVICE ), \
\
		/* Declare "Connection State" characteristic to start a Wi-Fi connection */ \
		CHARACTERISTIC_UUID128( HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE, HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL, \
				UUID_CONNECTION_STATE_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, \
				LEGATTDB_PERM_READABLE ), \
\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(wifi_config_char_connection_state_desc) ), \
\
			/* Declare client characteristic configuration descriptor \
			 * Value of the descriptor can be modified by the client \
			 * Value modified shall be retained during connection and across connection \
			 * for bonded devices.  Setting value to 1 tells this application to send notification \
			 * when value of the characteristic changes.  Value 2 is to allow indications. */ \
			CHAR_DESCRIPTOR_UUID16_WRITABLE( HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_CFG_DESC, GATT_UUID_CHAR_CLIENT_CONFIG, \
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ), \
\
		/* Declare "AP Parameters" characteristic */ \
		CHARACTERISTIC_UUID128_WRITABLE( HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS, HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_VAL, \
				UUID_AP_PARAMETERS_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, \
				LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_CMD | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_VARIABLE_LENGTH \
				/* | LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_AUTH_WRITABLE */), \
\
				/* User description for this characteristic */ \
				CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
					LEGATTDB_PERM_READABLE, sizeof(wifi_config_char_ap_parameters_desc) )


