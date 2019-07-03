/***************************************************************************************************

 File:     ble_callbacks.h
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
#pragma once

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

//
// Include the header files for the BLE profiles you want to include in this application here
//
#include "wifi_config_profile.h" // Wi-Fi Config profile
#include "aws_config_profile.h" // AWS Config profile
#include "simple_sensor_profile.h" // Simple Sensor profile

#pragma pack(push,1)
typedef PACKED struct
{
    BD_ADDR         bdaddr; /* BD address of the bonded host */
} ble_host_info_t;
#pragma pack(pop)


/* Set the contents of the advertisement data for this specific application */
void set_advertisement_data(void);
//void sd_notify_ble_enabled( bool enabled );

void ble_set_primary_service_uuid(uint8_t *uuid, uint8_t len);
wiced_result_t ble_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data );
