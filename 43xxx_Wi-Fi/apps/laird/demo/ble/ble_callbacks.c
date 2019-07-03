/***************************************************************************************************

 File:     ble_callbacks.c
 Author:   Laird Technologies
 Version:  0.1

 Description:  application-specific callbacks from the BLE stack

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
#include "ble_callbacks.h"

#include <string.h>
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"
#include "wiced.h"
#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"

#include "wiced_bt_cfg.h"
#include "gatt_db.h"


/* Invoked on connection up/down event */
static wiced_bt_gatt_status_t   gatts_connection_status_handler                ( wiced_bt_gatt_connection_status_t *p_status );
/* Invoked when connection is established */
static wiced_bt_gatt_status_t   gatts_connection_up                            ( wiced_bt_gatt_connection_status_t *p_status );
/* Invoked when connection is lost */
static wiced_bt_gatt_status_t   gatts_connection_down                          ( wiced_bt_gatt_connection_status_t *p_status );
/* Invoked when a peer reads a characteristic from this device */
static wiced_bt_gatt_status_t   gatt_server_read_request_handler               ( uint16_t conn_id, wiced_bt_gatt_read_t * p_read_data );
/* General GATT server event callback */
static wiced_bt_gatt_status_t   gatts_callback                                 ( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data);
/* Invoked when advertisements have stopped */
static                   void   advertisement_stopped                          ( void );
/* Invoked when a peer makes a write request for a characteristic to this device */
static wiced_bt_gatt_status_t   gatt_server_write_request_handler              ( uint16_t conn_id, wiced_bt_gatt_write_t * p_data );
/* Invoked when a peer makes a write and execute request to this device */
static wiced_bt_gatt_status_t   gatt_server_write_and_execute_request_handler  ( uint16_t conn_id, wiced_bt_gatt_exec_flag_t exec_flag );

/* Store peer connection status flags */
static ble_connection_flags_t   ble_connection_flags;
/* Store bonded peer host information */
static ble_host_info_t          ble_hostinfo;
/* Flag indicating whether the server is connected to a peer */
static wiced_bool_t             is_connected = FALSE;
/* Specify the primary service UUID length */
static uint8_t primaryServiceUuidLen = LEN_UUID_128;
/* Specify the primary service UUID */
static uint8_t primaryServiceUuid[LEN_UUID_128]; // NOTE: set this to the primary service to include in the advertisement
/* Indicates whether the stack has been initialized */
static bool ble_stack_initialized = WICED_FALSE;

/*
 * Set the primary service UUID for this application
 */
void ble_set_primary_service_uuid(uint8_t *uuid, uint8_t len) {
	primaryServiceUuidLen = len;
	memcpy(primaryServiceUuid, uuid, len);
}

/*
 * Setup advertisement data with 16 byte UUID and device name
 */
void set_advertisement_data(void)
{
    wiced_bt_ble_advert_elem_t  adv_elem[4];
    uint8_t ble_advertisement_flag_value        = BTM_BLE_GENERAL_DISCOVERABLE_FLAG | BTM_BLE_BREDR_NOT_SUPPORTED;
    uint8_t num_elem                            = 0;
    extern uint8_t g_sd_ble_adv_status_flags;

    // If the BLE stack has not yet been initialized, don't set the ad data yet
    if(!ble_stack_initialized)
    	return;

    /* 3 */
    adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_FLAG;
    adv_elem[num_elem].len          = 1;
    adv_elem[num_elem].p_data       = &ble_advertisement_flag_value;
    num_elem ++;

    /* 18 */
    switch(primaryServiceUuidLen)
    {
    	case LEN_UUID_16:
    		adv_elem[num_elem].advert_type = BTM_BLE_ADVERT_TYPE_16SRV_COMPLETE;
    		break;
    	case LEN_UUID_32:
    		adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_32SRV_COMPLETE;
    		break;
    	case LEN_UUID_128:
    	default:
    		adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_128SRV_COMPLETE;
    		break;
    }
    adv_elem[num_elem].len          = primaryServiceUuidLen;
    adv_elem[num_elem].p_data       = primaryServiceUuid;
    num_elem++;

    /* 5 */
    adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_NAME_COMPLETE;
    adv_elem[num_elem].len          = strlen((const char *)g_wiced_bt_cfg_settings.device_name);
    adv_elem[num_elem].p_data       = (uint8_t *)g_wiced_bt_cfg_settings.device_name;
    num_elem++;

    /* 5 */
    uint8_t manu_data[3] = { 0xff, 0x02, g_sd_ble_adv_status_flags };
    adv_elem[num_elem].advert_type  = BTM_BLE_ADVERT_TYPE_MANUFACTURER;
    adv_elem[num_elem].len          = 3;
    adv_elem[num_elem].p_data       = manu_data;
    num_elem++;

	wiced_bt_ble_set_raw_advertisement_data( num_elem, adv_elem );
}

/*
 * This function is executed in the BTM_ENABLED_EVT management callback.
 */
static void ble_init_gatt_db(void)
{
	ble_stack_initialized = WICED_TRUE;

    /* Register with stack to receive GATT callback */
    wiced_bt_gatt_register( gatts_callback );

    /*  Tell stack to use our GATT database */
    wiced_bt_gatt_db_init( gatt_db_get(), gatt_db_size());

    /* Set the advertising parameters and make the device discoverable */
    set_advertisement_data();

    /* Start advertisements */
    wiced_bt_start_advertisements( BTM_BLE_ADVERT_UNDIRECTED_LOW, 0, NULL );

    /*
     * Set flag_stay_connected to remain connected after all messages are sent
     * Reset flag to 0, to disconnect
     */
    ble_connection_flags.flag_stay_connected = 1;
}

/*
 * bt/ble link management callback
 */
wiced_result_t ble_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data )
{
    wiced_result_t                    result = WICED_BT_SUCCESS;
    wiced_bt_dev_ble_pairing_info_t  *p_info;
    wiced_bt_ble_advert_mode_t       *p_mode;

    switch( event )
    {
		/* Bluetooth  stack enabled */
		case BTM_ENABLED_EVT:
			ble_init_gatt_db();
			sd_notify_ble_enabled( true );
			WPRINT_BT_APP_INFO(( "BLE Enabled\n" ));
			break;

		case BTM_DISABLED_EVT:
			break;

		case BTM_PAIRING_IO_CAPABILITIES_BLE_REQUEST_EVT:
			p_event_data->pairing_io_capabilities_ble_request.local_io_cap  = BTM_IO_CAPABILITIES_NONE;
			p_event_data->pairing_io_capabilities_ble_request.oob_data      = BTM_OOB_NONE;
			p_event_data->pairing_io_capabilities_ble_request.auth_req      = BTM_LE_AUTH_REQ_BOND | BTM_LE_AUTH_REQ_MITM;
			p_event_data->pairing_io_capabilities_ble_request.max_key_size  = 0x10;
			p_event_data->pairing_io_capabilities_ble_request.init_keys     = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
			p_event_data->pairing_io_capabilities_ble_request.resp_keys     = BTM_LE_KEY_PENC | BTM_LE_KEY_PID;
			break;

		case BTM_PAIRING_COMPLETE_EVT:
			p_info = &p_event_data->pairing_complete.pairing_complete_info.ble;
			WPRINT_BT_APP_INFO(( "Pairing Complete: %d",p_info->reason));
			break;

		case BTM_SECURITY_REQUEST_EVT:
			wiced_bt_ble_security_grant( p_event_data->security_request.bd_addr, WICED_BT_SUCCESS );
			break;

		case BTM_BLE_ADVERT_STATE_CHANGED_EVT:
			p_mode = &p_event_data->ble_advert_state_changed;
			if ( *p_mode == BTM_BLE_ADVERT_OFF )
			{
				advertisement_stopped();
			}
			break;

		default:
			break;
    }

    return result;
}

/*
 * This function is invoked when advertisements stop.  If we are configured to stay connected,
 * disconnection was caused by the peer, start low advertisements, so that peer can connect
 * when it wakes up
 */
static void advertisement_stopped( void )
{
    if ( ble_connection_flags.flag_stay_connected && !ble_connection_flags.conn_id )
    {
        wiced_bt_start_advertisements( BTM_BLE_ADVERT_UNDIRECTED_LOW, 0, NULL );
    }
}

/*
 * Find attribute description by handle
 */
static attribute_t * get_attribute( uint16_t handle )
{
	return gatt_db_get_attribute(handle);
}

/*
 * Process Read request or command from peer device
 */
static wiced_bt_gatt_status_t gatt_server_read_request_handler( uint16_t conn_id, wiced_bt_gatt_read_t * p_read_data )
{
    wiced_bt_gatt_status_t result    = WICED_BT_GATT_SUCCESS;
    attribute_t *puAttribute;
    int          attr_len_to_copy;

    /* AWS config has a special read handler to support the chunked certificate attributes */
    result = aws_config_read_request_handler(conn_id, p_read_data);
    if (result == WICED_BT_GATT_SUCCESS)
    {
        return WICED_BT_GATT_SUCCESS;
    }

    if ( ( puAttribute = get_attribute(p_read_data->handle) ) == NULL)
    {
        WPRINT_BT_APP_INFO(("read_hndlr attr not found hdl:%x\n", p_read_data->handle ));
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    attr_len_to_copy = puAttribute->attr_len;

    //WPRINT_BT_APP_INFO(("read_hndlr conn_id:%d hdl:%x offset:%d len:%d\n", conn_id, p_read_data->handle, p_read_data->offset, attr_len_to_copy ));

    if ( p_read_data->offset >= puAttribute->attr_len )
    {
        attr_len_to_copy = 0;
    }

    if ( attr_len_to_copy != 0 )
    {
        uint8_t *from;
        int      to_copy = attr_len_to_copy - p_read_data->offset;


        if ( to_copy > *p_read_data->p_val_len )
        {
            to_copy = *p_read_data->p_val_len;
        }

        from = ((uint8_t *)puAttribute->p_attr) + p_read_data->offset;
        *p_read_data->p_val_len = to_copy;

        memcpy( p_read_data->p_val, from, to_copy);
    }

    return WICED_BT_GATT_SUCCESS;
}

/*
 * Process write request or write command from peer device
 */
static wiced_bt_gatt_status_t gatt_server_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data )
{
    wiced_bt_gatt_status_t result    = WICED_BT_GATT_SUCCESS;

    //WPRINT_BT_APP_INFO(("write_handler: conn_id:%d hdl:0x%x prep:%d offset:%d len:%d\n ", conn_id, p_data->handle, p_data->is_prep, p_data->offset, p_data->val_len ));

    // Call handler for wifi_config profile
    result = wifi_config_write_request_handler(conn_id, p_data);
    // If this was not a success, try the next profile
    if(result != WICED_BT_GATT_SUCCESS)
    	result = aws_config_write_request_handler(conn_id, p_data);
    // If this was not a success, try the next profile
    if(result != WICED_BT_GATT_SUCCESS)
    	result = simple_sensor_write_request_handler(conn_id, p_data);

    // Add other profiles here as needed...

    return result;
}

/*
 * Process a value confirmation request
 */
static wiced_bt_gatt_status_t gatt_server_confirmation_handler( uint16_t conn_id, uint16_t handle )
{
	wiced_bt_gatt_status_t result = WICED_BT_GATT_SUCCESS;

	// TODO: Determine by value of 'handle' which profile to invoke confirmation_handler on

    //WPRINT_BT_APP_INFO(( "indication_confirmation, conn %d hdl %d\n", conn_id, handle ));

    // For now, assume the wifi_config_profile confirmation handler should be called...
	result = wifi_config_confirmation_handler(&ble_connection_flags, conn_id, handle);
	// If this was not a success, try the next profile
	if(result != WICED_BT_GATT_SUCCESS)
		result = aws_config_confirmation_handler(&ble_connection_flags, conn_id, handle);
	// If this was not a success, try the next profile
	if(result != WICED_BT_GATT_SUCCESS)
		result = simple_sensor_confirmation_handler(&ble_connection_flags, conn_id, handle);

    return result;
}


/*
 * Write Execute Procedure
 */
static wiced_bt_gatt_status_t gatt_server_write_and_execute_request_handler( uint16_t conn_id, wiced_bt_gatt_exec_flag_t exec_flag )
{
    WPRINT_BT_APP_INFO(("write exec: flag:%d\n", exec_flag));
    return WICED_BT_GATT_SUCCESS;
}

/*
 * Process MTU request from the peer
 */
static wiced_bt_gatt_status_t gatt_server_mtu_request_handler( uint16_t conn_id, uint16_t mtu)
{
    WPRINT_BT_APP_INFO(("gatt_req_mtu: %d\n", mtu));
    return WICED_BT_GATT_SUCCESS;
}

/*
 * This function is invoked when connection is established
 */
static wiced_bt_gatt_status_t gatts_connection_up( wiced_bt_gatt_connection_status_t *p_status )
{
    WPRINT_BT_APP_INFO( ( "BLE connection id: %d\n", p_status->conn_id) );

    /* Update the connection handler.  Save address of the connected device. */
    ble_connection_flags.conn_id = p_status->conn_id;
    memcpy(ble_connection_flags.remote_addr, p_status->bd_addr, sizeof(BD_ADDR));

    /* Stop advertising */
    wiced_bt_start_advertisements( BTM_BLE_ADVERT_OFF, 0, NULL );

    WPRINT_BT_APP_INFO( ( "Stopping Advertisements\n" ) );

    memcpy( ble_hostinfo.bdaddr, p_status->bd_addr, sizeof( BD_ADDR ) );

    return WICED_BT_GATT_SUCCESS;
}

/*
 * This function is invoked when connection is lost
 */
static wiced_bt_gatt_status_t gatts_connection_down( wiced_bt_gatt_connection_status_t *p_status )
{
    WPRINT_BT_APP_INFO( ( "connection_down  conn_id:%d reason:%d\n", p_status->conn_id, p_status->reason ) );

    /* Resetting the device info */
    memset( ble_connection_flags.remote_addr, 0, 6 );
    ble_connection_flags.conn_id = 0;

    /*
     * If we are configured to stay connected, disconnection was
     * caused by the peer, start low advertisements, so that peer
     * can connect when it wakes up
     */
    if ( ble_connection_flags.flag_stay_connected )
    {
        wiced_bt_start_advertisements( BTM_BLE_ADVERT_UNDIRECTED_LOW, 0, NULL );
    }
    return WICED_BT_SUCCESS;
}

/*
 * Connection up/down event
 */
static wiced_bt_gatt_status_t gatts_connection_status_handler( wiced_bt_gatt_connection_status_t *p_status )
{
    is_connected = p_status->connected;
    if ( p_status->connected )
    {
	    // Initialize the BLE profiles
	    ble_aws_config_profile_init( );
	    ble_simple_sensor_profile_init( );

        return gatts_connection_up( p_status );
    }

    return gatts_connection_down( p_status );
}

/*
 * Process GATT request from the peer
 */
static wiced_bt_gatt_status_t gatt_server_request_handler( wiced_bt_gatt_attribute_request_t *p_data )
{
    wiced_bt_gatt_status_t result = WICED_BT_GATT_INVALID_PDU;

    //WPRINT_BT_APP_INFO(( "gatt_server_request_handler. conn %d, type %d\n", p_data->conn_id, p_data->request_type ));

    switch ( p_data->request_type )
    {
		case GATTS_REQ_TYPE_READ:
			result = gatt_server_read_request_handler( p_data->conn_id, &(p_data->data.read_req) );
			break;

		case GATTS_REQ_TYPE_WRITE:
			result = gatt_server_write_request_handler( p_data->conn_id, &(p_data->data.write_req) );
			break;

		case GATTS_REQ_TYPE_WRITE_EXEC:
			result = gatt_server_write_and_execute_request_handler( p_data->conn_id, p_data->data.exec_write );
			break;

		case GATTS_REQ_TYPE_MTU:
			result = gatt_server_mtu_request_handler( p_data->conn_id, p_data->data.mtu );
			break;

		case GATTS_REQ_TYPE_CONF:
			result = gatt_server_confirmation_handler( p_data->conn_id, p_data->data.handle );
			break;

		default:
			break;
    }
    return result;
}

/*
 * Callback for various GATT events.  As this application performs only as a GATT server, some of
 * the events are ommitted.
 */
wiced_bt_gatt_status_t gatts_callback( wiced_bt_gatt_evt_t event, wiced_bt_gatt_event_data_t *p_data)
{
    wiced_bt_gatt_status_t result = WICED_BT_GATT_INVALID_PDU;

    switch(event)
    {
		case GATT_CONNECTION_STATUS_EVT:
			result = gatts_connection_status_handler( &p_data->connection_status );
			break;

		case GATT_ATTRIBUTE_REQUEST_EVT:
			result = gatt_server_request_handler( &p_data->attribute_request );
			break;

		default:
			break;
    }

    return result;
}
