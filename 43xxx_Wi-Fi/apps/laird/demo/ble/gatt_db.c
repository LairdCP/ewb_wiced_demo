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
#include <string.h>
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"
#include "wiced.h"

#include "sd_common_defs.h"

#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"
#include "gatt_db.h"

/******************************************************************************
 *                          GATT Attributes
 ******************************************************************************/
uint8_t ble_device_name[]              = "Sterling WICED";
uint8_t ble_appearance_name[2]         = { BIT16_TO_8(APPEARANCE_GENERIC_TAG) };

/* Device Info Service */
uint8_t ble_char_system_id_value[]     = { 0xbb, 0xb8, 0xa1, 0x80, 0x5f, 0x9f, 0x91, 0x71 };
char ble_char_mfr_name_value[]         = { 'L', 'a', 'i', 'r', 'd', 0, };
char ble_char_model_num_value[]        = { 'E', 'W', 'B', 0,   0,   0,   0,   0 };
uint8_t primary_service_uuid[]         = { UUID_SCAN_SERVICE };


/* Wi-Fi Config Profile - Scan Service Attribute Declarations (see wifi_config_profile.h) */
WIFI_CONFIG_SCAN_SERVICE_ATTRIBUTE_DECLARATIONS

/* Wi-Fi Config Profile - Scan Service Attribute Declarations (see wifi_config_profile.h) */
WIFI_CONFIG_CONNECT_SERVICE_ATTRIBUTE_DECLARATIONS

/* AWS Config Profile - Provisioning Service Attribute Declarations (see lift_config_profile.h) */
AWS_CONFIG_PROV_SERVICE_ATTRIBUTE_DECLARATIONS

/* AWS Config Profile - Device Info Service Attribute Declarations (see lift_config_profile.h) */
AWS_CONFIG_DEVINFO_SERVICE_ATTRIBUTE_DECLARATIONS

/* Simple Sensor Profile - environment Service Attribute Declarations (see simple_sensor_profile.h) */
SIMPLE_SENSOR_ENVIRONMENT_SERVICE_ATTRIBUTE_DECLARATIONS

/* Simple Sensor Profile - Status LED Service Attribute Declarations (see simple_sensor_profile.h) */
SIMPLE_SENSOR_STATUS_LED_SERVICE_ATTRIBUTE_DECLARATIONS

/*
 * GATT Attribute Table Definition
 * -------------------------------
 * This table specifies the size and location of the BLE Attribute values
 * for each of the services. This table is setup in such a way that all
 * devices will implement the GAP and Device Info service Attributes with
 * optional application specific entries added at the end.
 */
attribute_t gatt_user_attributes[] =
{
	/* GAP Service Attributes */
	{ HANDLE_GAP_SERVICE_CHAR_DEV_NAME_VAL,            sizeof( ble_device_name ),         ble_device_name },
    { HANDLE_GAP_SERVICE_CHAR_DEV_APPEARANCE_VAL,      sizeof( ble_appearance_name ),     ble_appearance_name },

	/* Device Info Service Attributes */
    { HANDLE_DEV_INFO_SERVICE_CHAR_MFR_NAME_VAL,  sizeof(ble_char_mfr_name_value),   ble_char_mfr_name_value },
    { HANDLE_DEV_INFO_SERVICE_CHAR_MODEL_NUM_VAL, sizeof(ble_char_model_num_value),  ble_char_model_num_value },
    { HANDLE_DEV_INFO_SERVICE_CHAR_SYSTEM_ID_VAL, sizeof(ble_char_system_id_value),  ble_char_system_id_value },

	/* Wi-Fi Config Profile - Scan Service Attributes (see wifi_config_profile.h) */
	WIFI_CONFIG_SCAN_SERVICE_ATTRIBUTE_TABLE_ENTRIES,

	/* Wi-Fi Config Profile - Connect Service Attributes (see wifi_config_profile.h) */
	WIFI_CONFIG_CONNECT_SERVICE_ATTRIBUTE_TABLE_ENTRIES,

	/* AWS Config Profile - Provisioning Service Attributes (see aws_config_profile.h) */
	AWS_CONFIG_PROV_SERVICE_ATTRIBUTE_TABLE_ENTRIES,

	/* AWS Config Profile - Device Info Service Attributes (see aws_config_profile.h) */
	AWS_CONFIG_DEVINFO_SERVICE_ATTRIBUTE_TABLE_ENTRIES,

	/* Simple Sensor Profile - eCompass Service Attributes (see simple_sensor_profile.h) */
	SIMPLE_SENSOR_ENVIRONMENT_SERVICE_ATTRIBUTE_TABLE_ENTRIES,

	/* Simple Sensor Profile - Status LED Service Attributes (see simple_sensor_profile.h) */
	SIMPLE_SENSOR_STATUS_LED_SERVICE_ATTRIBUTE_TABLE_ENTRIES,
};


/******************************************************************************
 *                                GATT DATABASE
 ******************************************************************************/
/*
 * This is the GATT database for the Wi-Fi Config application.  It defines
 * services, characteristics and descriptors supported by the sensor.  Each
 * attribute in the database has a handle, (characteristic has two, one for
 * characteristic itself, another for the value).  The handles are used by
 * the peer to access attributes, and can be used locally by application for
 * example to retrieve data written by the peer.  Definition of characteristics
 * and descriptors has GATT Properties (read, write, notify...) but also has
 * permissions which identify if and how peer is allowed to read or write
 * into it.  All handles do not need to be sequential, but need to be in
 * ascending order.
 */
uint8_t ble_gatt_database[]=
{
    /* Declare mandatory GATT service */
    PRIMARY_SERVICE_UUID16( HANDLE_GATT_SERVICE, UUID_SERVCLASS_GATT_SERVER ),

    /* Declare mandatory GAP service. Device Name and Appearance are mandatory
     * characteristics of GAP service                                        */
    PRIMARY_SERVICE_UUID16( HANDLE_GAP_SERVICE, UUID_SERVCLASS_GAP_SERVER ),

        /* Declare mandatory GAP service characteristic: Dev Name */
        CHARACTERISTIC_UUID16( HANDLE_GAP_SERVICE_CHAR_DEV_NAME, HANDLE_GAP_SERVICE_CHAR_DEV_NAME_VAL,
                GATT_UUID_GAP_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

        /* Declare mandatory GAP service characteristic: Appearance */
        CHARACTERISTIC_UUID16( HANDLE_GAP_SERVICE_CHAR_DEV_APPEARANCE, HANDLE_GAP_SERVICE_CHAR_DEV_APPEARANCE_VAL,
                GATT_UUID_GAP_ICON, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

	/* Declare Device info service */
	PRIMARY_SERVICE_UUID16( HANDLE_DEV_INFO_SERVICE, UUID_SERVCLASS_DEVICE_INFO ),

		/* Handle 0x4e: characteristic Manufacturer Name, handle 0x4f characteristic value */
		CHARACTERISTIC_UUID16( HANDLE_DEV_INFO_SERVICE_CHAR_MFR_NAME, HANDLE_DEV_INFO_SERVICE_CHAR_MFR_NAME_VAL,
				GATT_UUID_MANU_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

		/* Handle 0x50: characteristic Model Number, handle 0x51 characteristic value */
		CHARACTERISTIC_UUID16( HANDLE_DEV_INFO_SERVICE_CHAR_MODEL_NUM, HANDLE_DEV_INFO_SERVICE_CHAR_MODEL_NUM_VAL,
				GATT_UUID_MODEL_NUMBER_STR, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

		/* Handle 0x52: characteristic System ID, handle 0x53 characteristic value */
		CHARACTERISTIC_UUID16( HANDLE_DEV_INFO_SERVICE_CHAR_SYSTEM_ID, HANDLE_DEV_INFO_SERVICE_CHAR_SYSTEM_ID_VAL,
				GATT_UUID_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE ),

	/* Declare proprietary Wi-Fi Scan Service with 128 byte UUID (see wifi_config_profile.h) */
	WIFI_CONFIG_SCAN_SERVICE_DECLARATION,

	/* Declare proprietary Wi-Fi Connect Service with 128 byte UUID (see wifi_config_profile.h) */
	WIFI_CONFIG_CONNECT_SERVICE_DECLARATION,

	/* Declare proprietary AWS Provisioning Service with 128 byte UUID (see lift_config_profile.h) */
	AWS_CONFIG_PROV_SERVICE_DECLARATION,

	/* Declare proprietary AWS Device Info Service with 128 byte UUID (see lift_config_profile.h) */
	AWS_CONFIG_DEVINFO_SERVICE_DECLARATION,

	/* Declare proprietary Simple Sensor environment Service with 128 byte UUID (see simple_sensor_profile.h) */
	SIMPLE_SENSOR_ENVIRONMENT_SERVICE_DECLARATION,

	/* Declare proprietary Simple Sensor Status LED Service with 128 byte UUID (see simple_sensor_profile.h) */
	SIMPLE_SENSOR_STATUS_LED_SERVICE_DECLARATION,
};

/*
 * Get a pointer to the primary service UUID
 */
uint8_t * gatt_db_get_primary_service_uuid(void)
{
	return primary_service_uuid;
}

/*
 * Get the length of the primary service uuid
 * Returns one of:
 * 		LEN_UUID_16
 * 		LEN_UUID_32
 * 		or
 * 		LEN_UUID_128
 */
uint8_t gatt_db_get_primary_service_uuid_len(void)
{
	return sizeof(primary_service_uuid);
}

/*
 * Returns the size in bytes of the gatt database
 */
uint32_t gatt_db_size(void)
{
	return sizeof(ble_gatt_database);
}

/*
 * Returns a pointer to the gatt database
 */
uint8_t *gatt_db_get(void)
{
	return ble_gatt_database;
}

/*
 * Get a pointer to the user attribute table
 */
attribute_t *gatt_db_get_user_attributes(void)
{
	return gatt_user_attributes;
}

/*
 * Get a pointer to a specific user attribute by its assigned handle
 */
attribute_t *gatt_db_get_attribute(uint16_t handle)
{
    int i;
    for ( i = 0; i <  sizeof( gatt_user_attributes ) / sizeof( gatt_user_attributes[0] ); i++ )
    {
        if ( gatt_user_attributes[i].handle == handle )
        {
            return ( &gatt_user_attributes[i] );
        }
    }
    WPRINT_BT_APP_INFO(( "attribute not found:%x\n", handle ));
    return NULL;
}
