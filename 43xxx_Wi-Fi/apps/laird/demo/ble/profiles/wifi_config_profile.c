/***************************************************************************************************

 File:     wifi_config_profile.c
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

#include "wifi_config_profile.h"
#include "gatt_db.h"

/* Wi-Fi Config Profile - Scan Service Attributes */
uint8_t wifi_config_char_scan_mode = 0;
uint16_t wifi_config_char_scan_mode_cfg = 0;
uint8_t wifi_config_char_ap_count = 0;
wifi_config_ap_details_t wifi_config_char_ap_details;
uint16_t wifi_config_char_ap_details_cfg = 0;

static wiced_result_t wifi_scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result );

/* Wi-Fi Config Profile - Connect Service Attributes */
uint16_t wifi_config_char_connection_state_cfg = 0;
wifi_config_ap_params_t wifi_config_char_ap_parameters;

// Keep a reference to the connection id when the peer writes to the cfg desc
static uint16_t last_conn_id = 0;

typedef struct
{
    uint32_t            result_count;   /* Count to measure the total scan results    */
} app_scan_data_t;

// local function prototypes
static wiced_result_t _save_wifi_network_params_to_dct( wifi_config_ap_params_t * pWiFiNetParams );

/*
 * Write request handler for the Wi-Fi Config Profile
 */
wiced_bt_gatt_status_t wifi_config_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data )
{
    wiced_bt_gatt_status_t result    = WICED_BT_GATT_SUCCESS;
    uint8_t                *p_attr   = p_data->p_val;
    last_conn_id = conn_id;

    switch ( p_data->handle )
    {
    /* By writing into Characteristic Client Configuration descriptor
     * peer can enable or disable notification or indication */
    case HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_CFG_DESC:
        if ( p_data->val_len != sizeof(wifi_config_char_scan_mode_cfg) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        wifi_config_char_scan_mode_cfg = p_attr[0] | ( p_attr[1] << 8 );
        break;

    case HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL:
        if ( p_data->val_len != sizeof(wifi_config_char_scan_mode) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        wifi_config_char_scan_mode = p_attr[0];
        wifi_config_scan_mode_write_handler();
        break;

	/* By writing into Characteristic Client Configuration descriptor
	 * peer can enable or disable notification or indication */
	case HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_CFG_DESC:
		if ( p_data->val_len != sizeof(wifi_config_char_ap_details_cfg) )
		{
			return WICED_BT_GATT_INVALID_ATTR_LEN;
		}
		wifi_config_char_ap_details_cfg = p_attr[0] | ( p_attr[1] << 8 );
		break;

    case HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_CFG_DESC:
    	if (p_data->val_len != sizeof(wifi_config_char_connection_state_cfg) )
    	{
    		return WICED_BT_GATT_INVALID_ATTR_LEN;
    	}
    	wifi_config_char_connection_state_cfg = p_attr[0] | ( p_attr[1] << 8 );
    	break;

    case HANDLE_WCONF_CONNECT_SERVICE_CHAR_AP_PARAMETERS_VAL:
    	// Copy the received block of the attribute into the destination
    	memcpy(&((uint8_t *)&wifi_config_char_ap_parameters)[p_data->offset], p_data->p_val, p_data->val_len);

    	// If this is the last block write, call the handler
    	if(p_data->offset + p_data->val_len >= sizeof(wifi_config_char_ap_parameters))
    	{
    		wifi_config_ap_parameters_write_handler();
    	}
    	break;

    default:
        result = WICED_BT_GATT_INVALID_HANDLE;
        break;
    }
    return result;
}

/*
 * Check if client has registered for notification/indication
 * and send message if appropriate
 */
static void wifi_config_send_message( ble_connection_flags_t *ble_connection_flags, uint16_t handle )
{
    WPRINT_BT_APP_INFO( ( "%s: Client's Characteristic configuration:%d\n",
        __func__, wifi_config_char_scan_mode_cfg ) );

    /* If client has registered for notification, send it */
    if ((handle == HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL) &&
        ((wifi_config_char_scan_mode_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification (
            ble_connection_flags->conn_id,
            HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL,
            sizeof(wifi_config_char_scan_mode),
            &wifi_config_char_scan_mode);
    }
    else if ((handle == HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL) &&
             ((wifi_config_char_connection_state_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
        wiced_bt_gatt_send_notification (
            ble_connection_flags->conn_id,
            HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL,
            sizeof(wifi_connection_state),
            &wifi_connection_state);
    }
    else if ((handle == HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL) &&
             ((wifi_config_char_ap_details_cfg & GATT_CLIENT_CONFIG_NOTIFICATION) != 0))
    {
    	wiced_bt_gatt_send_notification (
    	    ble_connection_flags->conn_id,
    	    HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL,
    	    sizeof(wifi_config_char_ap_details),
    	    (uint8_t *)&wifi_config_char_ap_details);
    }
}

/*
 * Handle a confirmation message from a peer
 */
wiced_bt_gatt_status_t wifi_config_confirmation_handler( ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle )
{
    if ( !ble_connection_flags->flag_indication_sent )
    {
        WPRINT_BT_APP_INFO(("Wrong Confirmation!"));
        return WICED_BT_GATT_SUCCESS;
    }
    ble_connection_flags->flag_indication_sent = 0;

    WPRINT_BT_APP_INFO(( "WIFI_CONFIG_CONFIRMATION_HANDLER...\n" ));
    wifi_config_send_message(ble_connection_flags, handle);

    return WICED_BT_GATT_SUCCESS;
}

/*
 * Callback function to handle scan results
 */
static wiced_result_t wifi_scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    /* Validate the input arguments */
    wiced_assert("Bad args", malloced_scan_result != NULL);

    if ( malloced_scan_result != NULL )
    {
        app_scan_data_t* scan_data  = (app_scan_data_t*)malloced_scan_result->user_data;

        malloc_transfer_to_curr_thread( malloced_scan_result );

        if ( malloced_scan_result->status == WICED_SCAN_INCOMPLETE )
        {
            wiced_scan_result_t* record = &malloced_scan_result->ap_details;

            // Update the Wi-Fi scan result structure
            wifi_config_char_ap_details.channel = record->channel;
            wifi_config_char_ap_details.freqBand = record->band;
            wifi_config_char_ap_details.rssi = (int8_t)record->signal_strength;
            wifi_config_char_ap_details.securityMode = record->security;
            memcpy(wifi_config_char_ap_details.ssid, record->SSID.value, sizeof(wifi_config_char_ap_details.ssid));
            wifi_config_char_ap_details.ssidLength = record->SSID.length;
            wifi_config_char_ap_details.state = SSID_DETAILS_STATE_CURRENT;

    	    wifi_config_char_scan_mode = WIFI_CONFIG_SCAN_MODE_SCAN_RUNNING;

            WPRINT_BT_APP_INFO( ( "%3ld ", scan_data->result_count ) );
            print_scan_result(record);
            scan_data->result_count++;
    	    wifi_config_char_ap_count = scan_data->result_count;

            // Notify the BLE peer that a new entry has been assigned
            if(last_conn_id != 0 && (wifi_config_char_ap_details_cfg & GATT_CLIENT_CONFIG_NOTIFICATION))
            {
            	wiced_bt_gatt_send_notification( last_conn_id, HANDLE_WCONF_SCAN_SERVICE_CHAR_AP_DETAILS_VAL, sizeof(wifi_config_char_ap_details), (uint8_t *)&wifi_config_char_ap_details );
            }
        }
        else
        {
    	    wifi_config_char_scan_mode = WIFI_CONFIG_SCAN_MODE_SCAN_DONE;
        	wifi_config_char_ap_details.state = SSID_DETAILS_STATE_STALE;
    	    WPRINT_BT_APP_INFO(( "Scan Complete!\n" ));
    	    if(last_conn_id != 0 && (wifi_config_char_scan_mode_cfg & GATT_CLIENT_CONFIG_NOTIFICATION))
    	    {
    	    	wiced_bt_gatt_send_notification( last_conn_id, HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL, sizeof(wifi_config_char_scan_mode), (uint8_t *)&wifi_config_char_scan_mode );
    	    	last_conn_id = 0;
    	    }
        }

        free( malloced_scan_result );
    }
    return WICED_SUCCESS;
}

/*
 * Write request handler for the "Scanning Mode" characteristic
 */
void wifi_config_scan_mode_write_handler(void)
{
    app_scan_data_t scan_data;

    // TODO: Kick off a Wi-Fi scan here if a 0x01 is written
	if(wifi_config_char_scan_mode == WIFI_CONFIG_SCAN_MODE_START_SCAN)
	{
		WPRINT_BT_APP_INFO(("Wi-Fi Scan Started...\n"));

	    scan_data.result_count = 0;
	    wifi_config_char_ap_count = scan_data.result_count;
	    WPRINT_BT_APP_INFO( ( "Waiting for scan results...\n" ) );
	    WPRINT_BT_APP_INFO( ("  # Type  BSSID             RSSI  Rate Chan  Security         SSID\n" ) );
	    WPRINT_BT_APP_INFO( ("----------------------------------------------------------------------------------------------\n" ) );

	    /* Start the scan */
	    wifi_config_char_scan_mode = WIFI_CONFIG_SCAN_MODE_SCAN_RUNNING;
    	if(last_conn_id != 0 && (wifi_config_char_scan_mode_cfg & GATT_CLIENT_CONFIG_NOTIFICATION))
    	{
    		wiced_bt_gatt_send_notification( last_conn_id, HANDLE_WCONF_SCAN_SERVICE_CHAR_SCAN_MODE_VAL, sizeof(wifi_config_char_scan_mode), (uint8_t *)&wifi_config_char_scan_mode );
    	}

	    wiced_wifi_scan_networks(wifi_scan_result_handler, &scan_data );
	}

}

void wifi_config_connect_state_change (void)
{
    if (last_conn_id != 0)
    {
        wiced_bt_gatt_send_notification(
            last_conn_id,
            HANDLE_WCONF_CONNECT_SERVICE_CHAR_CONNECTION_STATE_VAL,
            sizeof(wifi_connection_state), &wifi_connection_state );
    }
}

/*
 * Write request handler for the "AP Parameters" characteristic
 */
void wifi_config_ap_parameters_write_handler(void)
{
	wiced_result_t result;

	// Display a message indicating Wi-Fi parameters have been received
	WPRINT_BT_APP_INFO(("Saving Wi-Fi configuration...\n"));
	WPRINT_BT_APP_INFO(( "         SSID: %s\n", wifi_config_char_ap_parameters.ssid ));
	WPRINT_BT_APP_INFO(( "Security Mode: "));
	switch(wifi_config_char_ap_parameters.securityMode)
	{
		case WICED_SECURITY_OPEN:                                                                   /**< Open security                                         */
			WPRINT_BT_APP_INFO(( "Open\n" )); break;
		case WICED_SECURITY_WEP_PSK:                                                                /**< WEP PSK Security with open authentication             */
			WPRINT_BT_APP_INFO(( "WEP PSK\n" )); break;
		case WICED_SECURITY_WEP_SHARED:                                                             /**< WEP PSK Security with shared authentication           */
			WPRINT_BT_APP_INFO(( "WEP Shared\n" )); break;
		case WICED_SECURITY_WPA_TKIP_PSK:                                                           /**< WPA PSK Security with TKIP                            */
			WPRINT_BT_APP_INFO(( "WPA TKIP\n" )); break;
		case WICED_SECURITY_WPA_AES_PSK:                                                            /**< WPA PSK Security with AES                             */
			WPRINT_BT_APP_INFO(( "WPA AES\n" )); break;
		case WICED_SECURITY_WPA_MIXED_PSK:                                                          /**< WPA PSK Security with AES & TKIP                      */
			WPRINT_BT_APP_INFO(( "WPA Mixed\n" )); break;
		case WICED_SECURITY_WPA2_AES_PSK:                                                           /**< WPA2 PSK Security with AES                            */
			WPRINT_BT_APP_INFO(( "WPA2 AES\n" )); break;
		case WICED_SECURITY_WPA2_TKIP_PSK:                                                          /**< WPA2 PSK Security with TKIP                           */
			WPRINT_BT_APP_INFO(( "WPA2 TKIP\n" )); break;
		case WICED_SECURITY_WPA2_MIXED_PSK:                                                         /**< WPA2 PSK Security with AES & TKIP                     */
			WPRINT_BT_APP_INFO(( "WPA2 Mixed\n" )); break;

		case WICED_SECURITY_WPA_TKIP_ENT:                                                           /**< WPA Enterprise Security with TKIP                     */
			WPRINT_BT_APP_INFO(( "WPA TKIP Ent\n" )); break;
		case WICED_SECURITY_WPA_AES_ENT:                                                            /**< WPA Enterprise Security with AES                      */
			WPRINT_BT_APP_INFO(( "WPA AES Ent\n" )); break;
		case WICED_SECURITY_WPA_MIXED_ENT:                                                          /**< WPA Enterprise Security with AES & TKIP               */
			WPRINT_BT_APP_INFO(( "WPA Mixed Ent\n" )); break;
		case WICED_SECURITY_WPA2_TKIP_ENT:                                                          /**< WPA2 Enterprise Security with TKIP                    */
			WPRINT_BT_APP_INFO(( "WPA2 TKIP Ent\n" )); break;
		case WICED_SECURITY_WPA2_AES_ENT:                                                           /**< WPA2 Enterprise Security with AES                     */
			WPRINT_BT_APP_INFO(( "WPA2 AES Ent\n" )); break;
		case WICED_SECURITY_WPA2_MIXED_ENT:                                                         /**< WPA2 Enterprise Security with AES & TKIP              */
			WPRINT_BT_APP_INFO(( "WPA2 Mixed Ent\n" )); break;

		case WICED_SECURITY_IBSS_OPEN:                                                              /**< Open security on IBSS ad-hoc network         */
			WPRINT_BT_APP_INFO(( "IBSS Open\n" )); break;
		case WICED_SECURITY_WPS_OPEN:                                                               /**< WPS with open security                                */
			WPRINT_BT_APP_INFO(( "WPS Open\n" )); break;
		case WICED_SECURITY_WPS_SECURE:                                                             /**< WPS with AES security                                 */
			WPRINT_BT_APP_INFO(( "WPS Secure\n" )); break;

		default: WPRINT_BT_APP_INFO(( "Unknown\n" )); break;
	}


	// save the wifi network AP access information to the dct so the wifi code will load it
	result = _save_wifi_network_params_to_dct( &wifi_config_char_ap_parameters);

	if(result == WICED_SUCCESS)
	{
		WPRINT_BT_APP_INFO(( "Saved WiFi Configuration\n\n" ));
	}
	else
	{
		WPRINT_BT_APP_INFO(( "WiFi Config Save Failed \n\n" ));
	}

}



/********************************************************************************

 @brief  Saves the params needed to join a WiFi network to the DCT wiced_network_up( )
         pulls the params from the DCT automatically

 @param  pWiFiNetParams contains the wifi config params used be the ble service

 @return WICED_SUCCESS if no problems

 *******************************************************************************/
wiced_result_t _save_wifi_network_params_to_dct( wifi_config_ap_params_t * pWiFiNetParams )
{
    wiced_result_t result;
    wiced_config_ap_entry_t* pApEntry;
    platform_dct_wifi_config_t*     pDctWifiConfig          = NULL;

    // get the wi-fi config section for modifying, any memory allocation required is done inside wiced_dct_read_lock()
    result = wiced_dct_read_lock( (void**) &pDctWifiConfig, WICED_TRUE, DCT_WIFI_CONFIG_SECTION, 0, sizeof( *pDctWifiConfig ) );

    if( result == WICED_SUCCESS)
    {
    	pApEntry = &(pDctWifiConfig->stored_ap_list[0]);

        // copy the security key length and content
        pApEntry->security_key_length = pWiFiNetParams->passphraseLength;
        memcpy(pApEntry->security_key, pWiFiNetParams->passphrase, pWiFiNetParams->passphraseLength);

        // Copy the AP details struct content
        pApEntry->details.SSID.length = pWiFiNetParams->ssidLength;
        memcpy(pApEntry->details.SSID.value , pWiFiNetParams->ssid , pWiFiNetParams->ssidLength);
        pApEntry->details.security = pWiFiNetParams->securityMode;

        pDctWifiConfig ->device_configured = true;

        // save the content to the DCT
        result = wiced_dct_write( (const void*) pDctWifiConfig, DCT_WIFI_CONFIG_SECTION, 0, sizeof(platform_dct_wifi_config_t) );

        /* release the read lock */
        result = wiced_dct_read_unlock( pDctWifiConfig, WICED_TRUE );
    }

    return result;
}
