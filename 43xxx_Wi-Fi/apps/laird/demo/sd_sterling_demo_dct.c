/***************************************************************************************************

 File:     sd_sterling_demo_dct.c
 Author:   Laird Technologies
 Version:  0.1

 Description:  Device Configuration Table definitions and functions

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
#include "sd_mem_diag.h"

#include "sd_sterling_demo_dct.h"
#include "wiced_framework.h"

/******************************************************************************
* Global variables
******************************************************************************/

/* Defaults for the application DCT */
DEFINE_APP_DCT(sd_sterling_demo_dct_data_t)
{
    .bsec_state = { 0 },
    .ca_cert = { 0 },
    .aws_endpoint = { 0 },
    .client_id = { 0 },
    .aws_report_interval = 30,
    .aws_report_queue = 0,
    .aws_shadow_update_interval = 300
};

uint8_t g_sd_ble_adv_status_flags = SD_BLE_ADV_STATUS_FLAGS_CLEAR;

/******************************************************************************
* Global functions
******************************************************************************/

/*******************************************************************************
*
* @brief update the application-specific status flags in the BLE advertisement
*
*
*
* @return none
*
*****************************************************************************/
void sd_update_ble_adv_status_flags(void)
{
    bool serverConfigured = false;
    extern void set_advertisement_data(void); // reference to the BLE stack for updating ad data

    // get whether we have a wifi SSID and pass phrase set
    if(sd_network_is_configured_get())
        g_sd_ble_adv_status_flags |= SD_BLE_ADV_STATUS_FLAGS_WIFI_CONFIGURED;
    else
        g_sd_ble_adv_status_flags &= ~SD_BLE_ADV_STATUS_FLAGS_WIFI_CONFIGURED;

    // get whether wifi is connected
    if(wiced_network_is_up( WICED_STA_INTERFACE ))
        g_sd_ble_adv_status_flags |= SD_BLE_ADV_STATUS_FLAGS_WIFI_CONNECTED;
    else
        g_sd_ble_adv_status_flags &= ~SD_BLE_ADV_STATUS_FLAGS_WIFI_CONNECTED;

    // get whether server is configured
    sd_server_is_configured_get( &serverConfigured );
    if(serverConfigured)
        g_sd_ble_adv_status_flags |= SD_BLE_ADV_STATUS_FLAGS_AWS_CONFIGURED;
    else
        g_sd_ble_adv_status_flags &= ~SD_BLE_ADV_STATUS_FLAGS_AWS_CONFIGURED;

    // Update the advertisement data used by the BLE stack
    set_advertisement_data();
}

/**************************************************************************
 * This section demonstrates how to modify App section
 **************************************************************************/
wiced_result_t sd_app_dct_data_set( sd_sterling_demo_dct_data_t* pNewAppDct )
{
    sd_sterling_demo_dct_data_t *pAppDct = NULL;
    wiced_result_t result;

    result = wiced_dct_read_lock ((void**) &pAppDct, WICED_TRUE,
        DCT_APP_SECTION, 0, sizeof( *pAppDct));
    if (result != WICED_SUCCESS)
    {
        return result;
    }

    *pAppDct = *pNewAppDct;

    result = wiced_dct_write((const void*) pAppDct,
        DCT_APP_SECTION, 0, sizeof(sd_sterling_demo_dct_data_t));

    wiced_dct_read_unlock( pAppDct, WICED_TRUE );

    sd_update_ble_adv_status_flags();

    return result;
}


/********************************************************************************

 @brief  Gets the value of the bool that indicates whether we have a saved wifi
          configuration

 @param  pWiFiNetParams contains the wifi config params used be the ble service

 @return WICED_SUCCESS if no problems

 *******************************************************************************/
wiced_bool_t sd_network_is_configured_get (void)
{
    platform_dct_wifi_config_t *pDctWifiConfig = NULL;
    wiced_bool_t result;

    if (wiced_dct_read_lock((void**) &pDctWifiConfig, WICED_FALSE,
        DCT_WIFI_CONFIG_SECTION, 0, sizeof( *pDctWifiConfig)) != WICED_SUCCESS)
    {
        return WICED_FALSE;
    }

    result = pDctWifiConfig->device_configured;

    wiced_dct_read_unlock( pDctWifiConfig, WICED_FALSE );

    return result;
}


/********************************************************************************

 @brief  Returns whether we have saved server information in the DCT

 @param  none

 @return WICED_SUCCESS if no problems

 *******************************************************************************/
wiced_result_t sd_server_is_configured_get( bool * pServerConfig )
{
    sd_sterling_demo_dct_data_t *dct_app = NULL;

    if (pServerConfig == NULL)
    {
        return WICED_ERROR;
    }

    if (wiced_dct_read_lock ((void**) &dct_app, WICED_FALSE,
        DCT_APP_SECTION, 0, sizeof( *dct_app)) != WICED_SUCCESS)
    {
        return WICED_ERROR;
    }

    *pServerConfig = WICED_TRUE;
    if ((dct_app->client_id[0] == '\0') ||
        (dct_app->aws_endpoint[0] == '\0'))
    {
        *pServerConfig = WICED_FALSE;
    }

    wiced_dct_read_unlock (dct_app, WICED_FALSE);

    return WICED_SUCCESS;
}

/********************************************************************************

 @brief  prints (to the debug uart) the contents of the app section of the dct
          configuration

 @param  void

 @return WICED_SUCCESS if no problems

 *******************************************************************************/
wiced_result_t print_app_dct (void)
{
    sd_sterling_demo_dct_data_t *dct_app = NULL;

    if (wiced_dct_read_lock ((void**) &dct_app, WICED_FALSE,
        DCT_APP_SECTION, 0, sizeof( *dct_app)) != WICED_SUCCESS)
    {
        return WICED_ERROR;
    }

    WPRINT_APP_INFO( ( "Application DCT\r\n") );
    WPRINT_APP_INFO( ( "       client_id: %s\r\n", dct_app->client_id ) );
    WPRINT_APP_INFO( ( "    aws_endpoint: %s\r\n", dct_app->aws_endpoint ) );
    WPRINT_APP_INFO( ( " report interval: %ld\r\n", dct_app->aws_report_interval ));
    WPRINT_APP_INFO( ( "    report queue: %ld\r\n", dct_app->aws_report_queue ));
    WPRINT_APP_INFO( ( "   shadow update: %ld\r\n", dct_app->aws_shadow_update_interval ));

    WPRINT_APP_INFO( ("ca cert: %s\r\n", dct_app->ca_cert));

    wiced_dct_read_unlock (dct_app, WICED_FALSE);

    return WICED_SUCCESS;
}
