/***************************************************************************************************

 File:     lift_config_profile.h
 Author:   Laird Technologies
 Version:  0.1

 Description:  implementation of the "LIFT Config" BLE profile

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

#include "sd_aws.h"
#include "ble_connection.h"

#define MAX_LARGE_CHAR_SIZE   512

wiced_bt_gatt_status_t aws_config_write_request_handler (
    uint16_t conn_id, wiced_bt_gatt_write_t * p_data);
wiced_bt_gatt_status_t aws_config_read_request_handler (
    uint16_t conn_id, wiced_bt_gatt_read_t * p_read_data);
wiced_bt_gatt_status_t aws_config_confirmation_handler (
    ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle);
wiced_result_t ble_aws_config_profile_init(void);

void aws_config_status_notification (void);

extern sd_device_data_t sd_devinfo;
extern char sd_bdaddr[18];
extern char sd_macaddr[18];
extern uint8_t aws_config_prov_save_clear;
extern uint16_t aws_config_prov_status_cfg;

// ********************************* AWS Config Profile - Provisioning Service *********************************************
/* UUID value of the AWS Provisioning Service */
#define UUID_AWS_PROVISIONING_SERVICE           0xc6, 0x48, 0xeb, 0x9b, 0x82, 0xce, 0xe2, 0xbf, 0x22, 0x4d, 0xe4, 0x58, 0xe1, 0xb7, 0x80, 0x5c
/* UUID value of the Client Id Characteristic, Value Desc */ // beed1fd7-fb93-470d-88cb-810eae3a2848
#define UUID_AWS_PROV_CLIENT_ID_CHARACTERISTIC  0x48, 0x28, 0x3a, 0xae, 0x0e, 0x81, 0xcb, 0x88, 0x0d, 0x47, 0xfb, 0x93, 0xd7, 0x1f, 0xed, 0xbe
/* UUID value of the API Key Characteristic, Value Desc */ // ae7203f6-55a9-4a14-bcd7-7c59f234a9b5
#define UUID_AWS_PROV_ENDPOINT_CHARACTERISTIC    0xb5, 0xa9, 0x34, 0xf2, 0x59, 0x7c, 0xd7, 0xbc, 0x14, 0x4a, 0xa9, 0x55, 0xf6, 0x03, 0x72, 0xae
/* UUID value of the root CA certificate Characteristic, Value Desc *//* 207ae364-cb35-11e8-852e-8f64e1c526e3 */
#define UUID_AWS_PROV_CA_CERT_CHARACTERISTIC 0xe3, 0x26, 0xc5, 0xe1, 0x64, 0x8f, 0x2e, 0x85, 0xe8, 0x11, 0x35, 0xcb, 0x64, 0xe3, 0x7a, 0x20
/* UUID value of the User Key Characteristic, Value Desc */ // c1ae3a84-3093-4c0e-a66e-865abf7b084d
#define UUID_AWS_PROV_CLIENT_CERT_CHARACTERISTIC   0x4d, 0x08, 0x7b, 0xbf, 0x5a, 0x86, 0x6e, 0xa6, 0x0e, 0x4c, 0x93, 0x30, 0x84, 0x3a, 0xae, 0xc1
/* UUID value of the Secret Key Characteristic, Value Desc */ // 7d5b3df4-9287-4434-a6ed-9b96aba33b90
#define UUID_AWS_PROV_CLIENT_KEY_CHARACTERISTIC 0x90, 0x3b, 0xa3, 0xab, 0x96, 0x9b, 0xed, 0xa6, 0x34, 0x44, 0x87, 0x92, 0xf4, 0x3d, 0x5b, 0x7d
/* UUID value of the Save/Clear Characteristic, Value Desc */ // 22641e93-1f4a-4de3-ae0c-b1765e274be9
#define UUID_AWS_PROV_SAVE_CLEAR_CHARACTERISTIC      0xe9, 0x4b, 0x27, 0x5e, 0x76, 0xb1, 0x0c, 0xae, 0xe3, 0x4d, 0x4a, 0x1f, 0x93, 0x1e, 0x64, 0x22
/* UUID valud of the Status Characteristic, Value Desc */ // c8dcec28-d89b-11e8-800b-5fe460d8e08d
#define UUID_AWS_PROV_STATUS_CHARACTERISTIC 0x8d, 0xe0, 0xd8, 0x60, 0xe4, 0x5f, 0x0b, 0x80, 0xe8, 0x11, 0x9b, 0xd8, 0x28, 0xec, 0xdc, 0xc8


/* Declare the Characteristic Descriptors for the AWS Config Provisioning Service */
#define AWS_CONFIG_PROV_SERVICE_ATTRIBUTE_DECLARATIONS \
	char aws_config_prov_client_id_desc[]   = "Client ID"; \
	char aws_config_prov_endpoint_desc[]    = "AWS Endpoint"; \
	char aws_config_prov_ca_cert_desc[]     = "CA Cert"; \
	char aws_config_prov_client_cert_desc[] = "Client Cert"; \
	char aws_config_prov_client_key_desc[]  = "Client Key";\
	char aws_config_prov_save_clear_desc[]  = "Save/Clear"; \
	char aws_config_prov_status_desc[]      = "Status";


/* Declare the Attribute Table entries for the Provisioning Service */
#define AWS_CONFIG_PROV_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
	    { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_VAL,           sizeof(sd_aws_config.client_id), sd_aws_config.client_id }, \
		{ HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_DESCRIPTION,   sizeof(aws_config_prov_client_id_desc), (void *)aws_config_prov_client_id_desc }, \
	    { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_VAL,            sizeof(sd_aws_config.aws_endpoint), (void*)sd_aws_config.aws_endpoint }, \
		{ HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_DESCRIPTION,    sizeof(aws_config_prov_endpoint_desc), (void*)aws_config_prov_endpoint_desc }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_VAL,             MAX_LARGE_CHAR_SIZE, NULL }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_DESCRIPTION,     sizeof(aws_config_prov_ca_cert_desc), (void*)aws_config_prov_ca_cert_desc }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_VAL,         MAX_LARGE_CHAR_SIZE, NULL }, \
		{ HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_DESCRIPTION, sizeof(aws_config_prov_client_cert_desc), (void*)aws_config_prov_client_cert_desc }, \
	    { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_VAL,          MAX_LARGE_CHAR_SIZE, NULL }, \
		{ HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_DESCRIPTION,  sizeof(aws_config_prov_client_key_desc), (void*)aws_config_prov_client_key_desc }, \
	    { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_VAL,          sizeof(aws_config_prov_save_clear), &aws_config_prov_save_clear }, \
		{ HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_DESCRIPTION,  sizeof(aws_config_prov_save_clear_desc), (void*)aws_config_prov_save_clear_desc }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_VAL,              sizeof(aws_config_prov_status), &aws_config_prov_status }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_DESCRIPTION,      sizeof(aws_config_prov_status_desc), (void*)aws_config_prov_status_desc }, \
        { HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_CFG_DESC,         sizeof(aws_config_prov_status_cfg), &aws_config_prov_status_cfg }


/* Declare the handles for the AWS Config Provisioning Service */
#define HANDLE_AWS_CONFIG_PROFILE_PROVISIONING_SERVICE(baseHandle) \
		HANDLE_AWS_CONFIG_PROV_SERVICE = baseHandle, \
    	\
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_VAL, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_DESCRIPTION, \
		\
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_VAL, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_DESCRIPTION, \
		\
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT, \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_VAL, \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_DESCRIPTION, \
        \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_VAL, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_DESCRIPTION, \
		\
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_VAL, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_DESCRIPTION, \
		\
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_VAL, \
		HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_DESCRIPTION, \
        \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS, \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_VAL, \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_DESCRIPTION, \
        HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_CFG_DESC,


/* Declare the AWS Config Provisioning Service and its Characteristics for entry into the GATT DB */
#define AWS_CONFIG_PROV_SERVICE_DECLARATION \
	    /* Declare proprietary Provisioning Service with 128 byte UUID */ \
	    PRIMARY_SERVICE_UUID128( HANDLE_AWS_CONFIG_PROV_SERVICE, UUID_AWS_PROVISIONING_SERVICE ), \
\
				/* Declare "Client Id" characteristic */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_VAL, \
						UUID_AWS_PROV_CLIENT_ID_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, \
						LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_client_id_desc) ), \
\
				/* Declare "Endpoint" characteristic */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_VAL, \
				        UUID_AWS_PROV_ENDPOINT_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, \
				        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_endpoint_desc) ), \
\
                /* Declare "CA Cert" characteristic */ \
                CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_VAL, \
                        UUID_AWS_PROV_CA_CERT_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, \
                        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ ), \
\
                    /* User description for this characteristic */ \
                    CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
                        LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_ca_cert_desc) ), \
\
				/* Declare "Client Cert" characteristic */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_VAL, \
				        UUID_AWS_PROV_CLIENT_CERT_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_WRITE, \
				        LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_client_cert_desc) ), \
\
				/* Declare "Client Key" characteristic */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_VAL, \
				        UUID_AWS_PROV_CLIENT_KEY_CHARACTERISTIC, LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_client_key_desc) ), \
\
				/* Declare "Save/Clear" characteristic */ \
				CHARACTERISTIC_UUID128_WRITABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_VAL, \
						UUID_AWS_PROV_SAVE_CLEAR_CHARACTERISTIC, LEGATTDB_CHAR_PROP_WRITE, LEGATTDB_PERM_WRITE_REQ ), \
\
					/* User description for this characteristic */ \
					CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
						LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_save_clear_desc) ), \
\
                /* Declare "Status" characteristic */ \
                CHARACTERISTIC_UUID128( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS, HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_VAL, \
                        UUID_AWS_PROV_STATUS_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_READABLE ), \
\
                    /* User description for this characteristic */ \
                    CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
                        LEGATTDB_PERM_READABLE, sizeof(aws_config_prov_status_desc) ), \
\
                    /* Declare client characteristic configuration descriptor \
                     * Value of the descriptor can be modified by the client  \
                     * Value modified shall be retained during connection and across connection \
                     * for bonded devices.  Setting value to 1 tells this application to send notification \
                     * when value of the characteristic changes.  Value 2 is to allow indications. */ \
                    CHAR_DESCRIPTOR_UUID16_WRITABLE(\
                            HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_CFG_DESC,\
                            GATT_UUID_CHAR_CLIENT_CONFIG, \
                            LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ)


// ********************************* AWS Config Profile - Device Info Service ******************************************

/* UUID value of the AWS Device Info Service */ // ffa7be0d-b590-4629-a81f-b87e13b250ba
#define UUID_AWS_DEVINFO_SERVICE                             0xba, 0x50, 0xb2, 0x13, 0x7e, 0xb8, 0x1f, 0xa8, 0x29, 0x46, 0x90, 0xb5, 0x0d, 0xbe, 0xa7, 0xff
/* UUID value of the MAC Address Characteristic, Value Desc */ // d2f9dc18-cf45-40ed-98e2-687de46808f7
#define UUID_AWS_DEVINFO_MAC_ADDRESS_CHARACTERISTIC          0xf7, 0x08, 0x68, 0xe4, 0x7d, 0x68, 0xe2, 0x98, 0xed, 0x40, 0x45, 0xcf, 0x18, 0xdc, 0xf9, 0xd2
/* UUID value of the BD Address Characteristic, Value Desc */ // 03ed248c-f28b-4d6b-be8d-51d128d22f55
#define UUID_AWS_DEVINFO_BD_ADDRESS_CHARACTERISTIC           0x55, 0x2f, 0xd2, 0x28, 0xd1, 0x51, 0x8d, 0xbe, 0x6b, 0x4d, 0x8b, 0xf2, 0x8c, 0x24, 0xed, 0x03
/* UUID value of the Firmware Name Characteristic, Value Desc */ // 98f83c8a-d692-47ed-98ad-c9a070867e84
#define UUID_AWS_DEVINFO_FIRMWARE_NAME_CHARACTERISTIC        0x84, 0x7e, 0x86, 0x70, 0xa0, 0xc9, 0xad, 0x98, 0xed, 0x47, 0x92, 0xd6, 0x8a, 0x3c, 0xf8, 0x98
/* UUID value of the Firmware Version Characteristic, Value Desc */ // 690f526d-12e7-402e-b2fd-a9fb911b22e4
#define UUID_AWS_DEVINFO_FIRMWARE_VERSION_CHARACTERISTIC     0xe4, 0x22, 0x1b, 0x91, 0xfb, 0xa9, 0xfd, 0xb2, 0x2e, 0x40, 0xe7, 0x12, 0x6d, 0x52, 0x0f, 0x69

/* Declare the Characteristic Descriptors for the AWS Config Device Info Service */
#define AWS_CONFIG_DEVINFO_SERVICE_ATTRIBUTE_DECLARATIONS \
		char aws_config_devinfo_mac_address_desc[]          = "MAC Address"; \
		char aws_config_devinfo_bd_address_desc[]           = "BT Address"; \
		char aws_config_devinfo_firmware_name_desc[]        = "Firmware Name"; \
		char aws_config_devinfo_firmware_version_desc[]     = "Firmware Version";


/* Declare the Attribute Table entries for the Device Info Service */
#define AWS_CONFIG_DEVINFO_SERVICE_ATTRIBUTE_TABLE_ENTRIES \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_VAL,              sizeof(sd_macaddr), (void*)sd_macaddr }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_DESCRIPTION,      sizeof(aws_config_devinfo_mac_address_desc), aws_config_devinfo_mac_address_desc }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_VAL,               sizeof(sd_bdaddr), (void*)sd_bdaddr }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_DESCRIPTION,       sizeof(aws_config_devinfo_bd_address_desc), aws_config_devinfo_bd_address_desc }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_VAL,            sizeof(sd_devinfo.firmwareName), (void*)sd_devinfo.firmwareName }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_DESCRIPTION,    sizeof(aws_config_devinfo_firmware_name_desc), aws_config_devinfo_firmware_name_desc }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_VAL,         sizeof(sd_devinfo.firmwareVer), (void*)sd_devinfo.firmwareVer }, \
		{ HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_DESCRIPTION, sizeof(aws_config_devinfo_firmware_version_desc), aws_config_devinfo_firmware_version_desc }


/* Declare the handles for the Lift Config Device Info Service */
#define HANDLE_AWS_CONFIG_PROFILE_DEVINFO_SERVICE(baseHandle) \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE = baseHandle, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_VAL, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_DESCRIPTION, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_VAL, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_DESCRIPTION, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_VAL, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_DESCRIPTION, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_VAL, \
		HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_DESCRIPTION,


/* Declare the AWS Config Device Info Service and its Characteristics for entry into the GATT DB */
#define AWS_CONFIG_DEVINFO_SERVICE_DECLARATION \
PRIMARY_SERVICE_UUID128( HANDLE_AWS_CONFIG_DEVINFO_SERVICE, UUID_AWS_DEVINFO_SERVICE ), \
\
		/* Declare "MAC Address" characteristic */ \
		CHARACTERISTIC_UUID128( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS, HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_VAL, \
				UUID_AWS_DEVINFO_MAC_ADDRESS_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ , LEGATTDB_PERM_READABLE ), \
		\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_MAC_ADDRESS_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(aws_config_devinfo_mac_address_desc) ), \
\
		/* Declare "BD Address" characteristic */ \
		CHARACTERISTIC_UUID128( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS, HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_VAL, \
				UUID_AWS_DEVINFO_BD_ADDRESS_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ , LEGATTDB_PERM_READABLE ), \
		\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_BD_ADDRESS_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(aws_config_devinfo_bd_address_desc) ), \
\
		/* Declare "Firmware Name" characteristic */ \
		CHARACTERISTIC_UUID128( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME, HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_VAL, \
				UUID_AWS_DEVINFO_FIRMWARE_NAME_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ , LEGATTDB_PERM_READABLE ), \
		\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_NAME_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(aws_config_devinfo_firmware_name_desc) ), \
\
		/* Declare "Firmware Version" characteristic */ \
		CHARACTERISTIC_UUID128( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION, HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_VAL, \
				UUID_AWS_DEVINFO_FIRMWARE_VERSION_CHARACTERISTIC, LEGATTDB_CHAR_PROP_READ , LEGATTDB_PERM_READABLE ), \
		\
			/* User description for this characteristic */ \
			CHAR_DESCRIPTOR_UUID16_VARIABLE( HANDLE_AWS_CONFIG_DEVINFO_SERVICE_CHAR_FIRMWARE_VERSION_DESCRIPTION, GATT_UUID_CHAR_DESCRIPTION, \
				LEGATTDB_PERM_READABLE, sizeof(aws_config_devinfo_firmware_version_desc) )
