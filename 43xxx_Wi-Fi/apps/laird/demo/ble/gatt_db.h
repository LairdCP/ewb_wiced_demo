/***************************************************************************************************

 File:     gatt_db.c
 Author:   Laird Technologies
 Version:  0.1

 Description:  application-specific gatt database

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

//
// Include the header files for the BLE profiles you want to include in this application here
//
#include "wifi_config_profile.h" // Wi-Fi config profile
#include "aws_config_profile.h" // AWS config profile
#include "simple_sensor_profile.h" // Simple sensor profile

#pragma pack(push,1)

typedef struct
{
    uint16_t        handle;
    uint16_t        attr_len;
    void*           p_attr;
} attribute_t;

#pragma pack(pop)

uint8_t * gatt_db_get_primary_service_uuid(void);
uint8_t gatt_db_get_primary_service_uuid_len(void);
uint32_t gatt_db_size(void);
uint8_t *gatt_db_get(void);
attribute_t *gatt_db_get_user_attributes(void);
attribute_t *gatt_db_get_attribute(uint16_t handle);

/******************************************************************************
 *                          GATT Attribute Handles
 ******************************************************************************/
typedef enum
{
	/* BLE Profile - GATT Service */
    HANDLE_GATT_SERVICE = 0x1,

	/* BLE Profile - GAP Service */
    HANDLE_GAP_SERVICE = 0x14,
        HANDLE_GAP_SERVICE_CHAR_DEV_NAME,
        HANDLE_GAP_SERVICE_CHAR_DEV_NAME_VAL,

        HANDLE_GAP_SERVICE_CHAR_DEV_APPEARANCE,
        HANDLE_GAP_SERVICE_CHAR_DEV_APPEARANCE_VAL,

	/* BLE Profile - Device Info Service */
    HANDLE_DEV_INFO_SERVICE = 0x28,
        HANDLE_DEV_INFO_SERVICE_CHAR_MFR_NAME,
        HANDLE_DEV_INFO_SERVICE_CHAR_MFR_NAME_VAL,

        HANDLE_DEV_INFO_SERVICE_CHAR_MODEL_NUM,
        HANDLE_DEV_INFO_SERVICE_CHAR_MODEL_NUM_VAL,

        HANDLE_DEV_INFO_SERVICE_CHAR_SYSTEM_ID,
        HANDLE_DEV_INFO_SERVICE_CHAR_SYSTEM_ID_VAL,

	/* Wi-Fi Config Profile - Scan Service */
	HANDLE_WIFI_CONFIG_PROFILE_SCAN_SERVICE(0x30)

	/* Wi-Fi Config Profile - Connect Service */
	HANDLE_WIFI_CONFIG_PROFILE_CONNECT_SERVICE(0x40)

	/* AWS Config Profile - Provisioning Service */
	HANDLE_AWS_CONFIG_PROFILE_PROVISIONING_SERVICE(0x50)

	/* AWS Config Profile - Device Info Service */
	HANDLE_AWS_CONFIG_PROFILE_DEVINFO_SERVICE(0x70)

	/* Simple Sensor Profile - environment Service */
	HANDLE_SIMPLE_SENSOR_PROFILE_ENVIRONMENT_SERVICE(0xA0)

	/* Simple Sensor Profile - Status LED Service */
	HANDLE_SIMPLE_SENSOR_PROFILE_STATUS_LED_SERVICE(0xC0)

} gatt_db_handles;
