/***************************************************************************************************

 File:     sd_sterling_demo_dct.h
 Author:   Laird Technologies
 Version:  0.1

 Description:  public interface for   sd_sterling_demo_dct.c

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
#include "stdbool.h"

/******************************************************************************
* Constants
******************************************************************************/

#define DEV_CLIENT_ID_SIZE 	    32
#define DEV_CERT_SIZE           4096
#define DEV_KEY_SIZE            2048
#define DEV_AWS_ENDPOINT_SIZE   256

// Application-specific global BLE advertising status flags
#define SD_BLE_ADV_STATUS_FLAGS_CLEAR             0x00
#define SD_BLE_ADV_STATUS_FLAGS_WIFI_CONFIGURED   0x01
#define SD_BLE_ADV_STATUS_FLAGS_WIFI_CONNECTED    0x02
#define SD_BLE_ADV_STATUS_FLAGS_AWS_CONFIGURED    0x04

/******************************************************************************
* Type definitions
******************************************************************************/

typedef struct sd_sterling_demo_dct_data
{
    /* Saved state of the BME680 sensor */
    uint8_t bsec_state[512];
    uint16_t bsec_state_len;

    /* AWS IoT base configuration parameters */
    uint8_t ca_cert[DEV_CERT_SIZE];
    uint8_t aws_endpoint[DEV_AWS_ENDPOINT_SIZE];
    uint8_t client_id[DEV_CLIENT_ID_SIZE];

    /* AWS IoT device configuration parameters */
    uint32_t aws_report_interval;
    uint32_t aws_report_queue;
    uint32_t aws_shadow_update_interval;
} sd_sterling_demo_dct_data_t;

/******************************************************************************
* Function prototypes
******************************************************************************/

wiced_result_t print_app_dct (void);
wiced_result_t sd_app_dct_data_set (sd_sterling_demo_dct_data_t *);
wiced_bool_t sd_network_is_configured_get (void);
wiced_result_t sd_server_is_configured_get (bool *pServerConfig);
wiced_result_t sd_auth_is_configured_get (bool *pAuthConfig);
void sd_update_ble_adv_status_flags (void);

