/***************************************************************************************************

 File:     wifi_config_dct.h
 Author:   Laird Technologies
 Version:  0.1

 Description: defines hard-coded Wi-Fi configuration defaults for DCT, useful during development

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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/*
 * If "WIFI_CONFIG_DCT_H := wifi_config_dct.h" is present
 * in the project .mk file, the AP settings in this file are
 * stored in the DCT. These settings may be overwritten at
 * manufacture when the DCT is written with the final
 * production configuration
 */

/* This is the soft AP used for device configuration */
#define CONFIG_AP_SSID       "NOT USED FOR THIS APP"
#define CONFIG_AP_PASSPHRASE "NOT USED FOR THIS APP"
#define CONFIG_AP_SECURITY   WICED_SECURITY_OPEN
#define CONFIG_AP_CHANNEL    1
#define CONFIG_AP_VALID      WICED_FALSE

/* This is the soft AP available for normal operation */
#define SOFT_AP_SSID         "NOT USED FOR THIS APP"
#define SOFT_AP_PASSPHRASE   "NOT USED FOR THIS APP"
#define SOFT_AP_SECURITY     WICED_SECURITY_WPA2_AES_PSK
#define SOFT_AP_CHANNEL      6
//#define SOFT_AP_VALID        WICED_TRUE

// If you'd like to hard-code Wi-Fi credentials at compile time,
// you can specify AP parameters here and they will become part of the default DCT
// NOTE: You must also uncomment the following line in the sterling_demo.mk file:
//       #WIFI_CONFIG_DCT_H  := wifi_config_dct.h
//
#define CLIENT_AP_SSID       "WPD_Madison"
#define CLIENT_AP_PASSPHRASE "1madisonresearch!"
#define CLIENT_AP_BSS_TYPE   WICED_BSS_TYPE_INFRASTRUCTURE
#define CLIENT_AP_SECURITY   WICED_SECURITY_WPA2_MIXED_PSK
#define CLIENT_AP_CHANNEL    1
#define CLIENT_AP_BAND       WICED_802_11_BAND_2_4GHZ


/* Override default country code */
#define WICED_COUNTRY_CODE    WICED_COUNTRY_UNITED_STATES
#define WICED_COUNTRY_AGGREGATE_CODE    WICED_COUNTRY_AGGREGATE_XV_0

/* This is the network interface the device will work with */
#define WICED_NETWORK_INTERFACE   WICED_STA_INTERFACE

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
