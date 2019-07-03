/***************************************************************************************************

 File:     lift_config_profile.c
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
#include "wiced.h"
#include "sd_common_defs.h"
#include "sd_aws.h"

#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"

#include "mbedtls/sha256.h"

#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"

//#include "sd_evb_profile.h"
#include "sd_sterling_demo_dct.h"

#include "aws_config_profile.h"
#include "gatt_db.h"

/* AWS Config Profile - Provisioning Service Attributes */
uint8_t aws_config_prov_save_clear;
uint16_t aws_config_prov_status_cfg;

/* AWS Config Profile - Device Info Service Attributes */
sd_device_data_t sd_devinfo = {
    .firmwareName =    "Laird Sterling-EWB Demo",
    .firmwareVer =     "1.0.1"
};

char sd_bdaddr[18];
char sd_macaddr[18];

// Keep a reference to the connection id when the peer writes to the cfg desc
static uint16_t last_conn_id = 0;

#define AWS_CONFIG_PROV_SAVE_VAL  (1)
#define AWS_CONFIG_PROV_CLEAR_VAL (2)

/******************************************************************************
* Local functions
******************************************************************************/

/*
 * Save the LIFT provisioning info to the DCT
 */
static wiced_result_t aws_config_save_provisioning(void)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;
    platform_dct_security_t* dct_security = NULL;

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("aws_config_save_provisioning: failed to read user DCT\n"));
        return WICED_ERROR;
    }

    /* Copy things into the DCT */
    if (strlen((char *)sd_aws_config.client_id) > 0)
    {
        memcpy(dct_app->client_id, sd_aws_config.client_id, sizeof(dct_app->client_id));
    }
    if (strlen((char *)sd_aws_config.aws_endpoint) > 0)
    {
        memcpy(dct_app->aws_endpoint, sd_aws_config.aws_endpoint, sizeof(dct_app->aws_endpoint));
    }
    if (strlen((char *)sd_aws_config.ca_cert) > 0)
    {
        memcpy(dct_app->ca_cert, sd_aws_config.ca_cert, sizeof(dct_app->ca_cert));
    }
    dct_app->aws_report_interval = sd_aws_config.aws_report_interval;
    dct_app->aws_report_queue = sd_aws_config.aws_report_queue;
    dct_app->aws_shadow_update_interval = sd_aws_config.aws_shadow_update_interval;

    /* Write the user DCT */
    wiced_dct_write((const void*) dct_app, DCT_APP_SECTION,
            0, sizeof(*dct_app));
    wiced_dct_read_unlock(dct_app, WICED_TRUE);

    /* Lock the DCT to allow us to access the certificate and key */
    if (wiced_dct_read_lock (
            (void**)&dct_security, WICED_TRUE, DCT_SECURITY_SECTION,
            0, sizeof(*dct_security)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("aws_config_save_provisioning: failed to read security DCT\n"));
        return WICED_ERROR;
    }

    /* Copy things into the DCT */
    if (strlen((char *)sd_aws_config.client_cert) > 0)
    {
        memcpy(dct_security->certificate, sd_aws_config.client_cert, sizeof(sd_aws_config.client_cert));
    }
    if (strlen((char *)sd_aws_config.client_key) > 0)
    {
        memcpy(dct_security->private_key, sd_aws_config.client_key, sizeof(sd_aws_config.client_key));
    }

    /* Write the security DCT */
    wiced_dct_write((const void*) dct_security, DCT_SECURITY_SECTION,
            0, sizeof(*dct_security));
    wiced_dct_read_unlock(dct_security, WICED_TRUE);

    sd_aws_config_updated ();

    return WICED_SUCCESS;
}

/*
 * Clear the LIFT provisioning info from the DCT
 */
static wiced_result_t aws_config_clear_provisioning(void)
{
    sd_sterling_demo_dct_data_t* dct_app = NULL;
    platform_dct_security_t* dct_security = NULL;

    /* Read device info data from user DCT */
    if (wiced_dct_read_lock(
            (void**) &dct_app, WICED_TRUE, DCT_APP_SECTION,
            0, sizeof(*dct_app)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("aws_config_clear_provisioning: failed to read user DCT\n"));
        return WICED_ERROR;
    }

    /* Clear the parameters */
    memset(dct_app->client_id, 0, sizeof(dct_app->client_id));
    memset(dct_app->aws_endpoint, 0, sizeof(dct_app->aws_endpoint));
    memset(dct_app->ca_cert, 0, sizeof(dct_app->ca_cert));

    /* Write the user DCT */
    wiced_dct_write((const void*) dct_app, DCT_APP_SECTION,
            0, sizeof(*dct_app));
    wiced_dct_read_unlock(dct_app, WICED_TRUE);

    /* Lock the DCT to allow us to access the certificate and key */
    if (wiced_dct_read_lock (
            (void**)&dct_security, WICED_TRUE, DCT_SECURITY_SECTION,
            0, sizeof(*dct_security)) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("aws_config_clear_provisioning: failed to read security DCT\n"));
        return WICED_ERROR;
    }

    /* Clear the security DCT parameters */
    memset(dct_security->certificate, 0, sizeof(dct_security->certificate));
    memset(dct_security->private_key, 0, sizeof(dct_security->private_key));

    /* Write the security DCT */
    wiced_dct_write((const void*) dct_security, DCT_SECURITY_SECTION,
            0, sizeof(*dct_security));
    wiced_dct_read_unlock(dct_security, WICED_FALSE);

    sd_aws_config_updated ();

    return WICED_SUCCESS;
}

/******************************************************************************
* Global functions
******************************************************************************/

wiced_bt_gatt_status_t aws_config_read_request_handler (
    uint16_t conn_id, wiced_bt_gatt_read_t * p_read_data)
{
    uint8_t sha256[32];

    /* We only handle certain handles here */
    if (p_read_data->handle == HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_VAL)
    {
        mbedtls_sha256(sd_aws_config.ca_cert, strlen((char *)sd_aws_config.ca_cert), sha256, 0);
    }
    else if (p_read_data->handle == HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_VAL)
    {
        mbedtls_sha256(sd_aws_config.client_cert, strlen((char *)sd_aws_config.client_cert), sha256, 0);
    }
    else if (p_read_data->handle == HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_VAL)
    {
        mbedtls_sha256(sd_aws_config.client_key, strlen((char *)sd_aws_config.client_key), sha256, 0);
    }
    else
    {
        /* The default read handler will be used instead */
        return WICED_BT_GATT_INVALID_HANDLE;
    }

    if (*p_read_data->p_val_len < sizeof(sha256))
    {
        return WICED_BT_GATT_INVALID_ATTR_LEN;
    }

    *p_read_data->p_val_len = sizeof(sha256);
    memcpy (p_read_data->p_val, sha256, sizeof(sha256));

    return WICED_BT_GATT_SUCCESS;
}

/*
 * Write request handler for the LIFT Config Profile
 */
wiced_bt_gatt_status_t aws_config_write_request_handler( uint16_t conn_id, wiced_bt_gatt_write_t * p_data )
{
    wiced_bt_gatt_status_t result    = WICED_BT_GATT_SUCCESS;
    uint8_t *p_attr   = p_data->p_val;
    uint16_t data_len = p_data->val_len;
    uint32_t p_app_ofs;

    last_conn_id = conn_id;
    if (data_len >= sizeof(p_app_ofs))
    {
        p_app_ofs =
            (p_attr[3] << 24) |
            (p_attr[2] << 16) |
            (p_attr[1] <<  8) |
            (p_attr[0] <<  0);
    }

    switch ( p_data->handle )
    {
    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_ID_VAL:
    	memcpy(sd_aws_config.client_id, p_attr, MIN(p_data->val_len, sizeof(sd_aws_config.client_id)));
    	break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_ENDPOINT_VAL:
    	memcpy(sd_aws_config.aws_endpoint, p_attr, MIN(p_data->val_len, sizeof(sd_aws_config.aws_endpoint)));
    	break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CA_CERT_VAL:
        if (data_len >= sizeof(p_app_ofs))
        {
            p_attr += sizeof(p_app_ofs);
            data_len -= 4;

            if ((p_app_ofs + data_len) > sizeof(sd_aws_config.ca_cert))
            {
                return WICED_BT_GATT_INVALID_ATTR_LEN;
            }
            memcpy(sd_aws_config.ca_cert + p_app_ofs, p_attr, data_len);
            sd_aws_config.ca_cert[p_app_ofs + data_len] = '\0';
        }
        else
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_CERT_VAL:
        if (data_len >= sizeof(p_app_ofs))
        {
            p_attr += sizeof(p_app_ofs);
            data_len -= 4;

            if ((p_app_ofs + data_len) > sizeof(sd_aws_config.client_cert))
            {
                return WICED_BT_GATT_INVALID_ATTR_LEN;
            }
            memcpy(sd_aws_config.client_cert + p_app_ofs, p_attr, data_len);
            sd_aws_config.client_cert[p_app_ofs + data_len] = '\0';
        }
        else
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
    	break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_CLIENT_KEY_VAL:
        if (data_len >= sizeof(p_app_ofs))
        {
            p_attr += sizeof(p_app_ofs);
            data_len -= 4;

            if ((p_app_ofs + data_len) > sizeof(sd_aws_config.client_key))
            {
                return WICED_BT_GATT_INVALID_ATTR_LEN;
            }
            memcpy(sd_aws_config.client_key + p_app_ofs, p_attr, data_len);
            sd_aws_config.client_key[p_app_ofs + data_len] = '\0';
        }
        else
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
    	break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_SAVE_CLEAR_VAL:
        if ( p_data->val_len != sizeof(aws_config_prov_save_clear) )
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        aws_config_prov_save_clear = p_attr[0];
        if(aws_config_prov_save_clear == AWS_CONFIG_PROV_CLEAR_VAL)
        {
        	aws_config_clear_provisioning();
        }
        else if(aws_config_prov_save_clear == AWS_CONFIG_PROV_SAVE_VAL)
        {
        	aws_config_save_provisioning();
        }
        break;

    case HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_CFG_DESC:
        if (p_data->val_len != sizeof(aws_config_prov_status_cfg))
        {
            return WICED_BT_GATT_INVALID_ATTR_LEN;
        }
        aws_config_prov_status_cfg = p_attr[0] | ( p_attr[1] << 8 );
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
wiced_bt_gatt_status_t aws_config_confirmation_handler( ble_connection_flags_t *ble_connection_flags, uint16_t conn_id, uint16_t handle )
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
 * Initialize the structures for the LIFT Config profile
 */
wiced_result_t ble_aws_config_profile_init(void)
{
	wiced_mac_t    macAddr;
	wiced_bt_device_address_t bdAddr;

	// Update the BD Address and MAC Address
    wwd_wifi_get_mac_address(&macAddr, WWD_STA_INTERFACE );
    sprintf(sd_macaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
            macAddr.octet[0], macAddr.octet[1], macAddr.octet[2],
            macAddr.octet[3], macAddr.octet[4], macAddr.octet[5]);

    wiced_bt_dev_read_local_addr (bdAddr);
    sprintf(sd_bdaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
            bdAddr[0], bdAddr[1], bdAddr[2],
            bdAddr[3], bdAddr[4], bdAddr[5]);

	return WICED_SUCCESS;
}

void aws_config_status_notification (void)
{
    /* If there hasn't been a connection, do nothing */
    if (last_conn_id == 0)
    {
        return;
    }

    wiced_bt_gatt_send_notification(
            last_conn_id,
            HANDLE_AWS_CONFIG_PROV_SERVICE_CHAR_STATUS_VAL,
            sizeof(aws_config_prov_status), (uint8_t *)&(aws_config_prov_status));
}
