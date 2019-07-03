/***************************************************************************************************

 File:     ble_connection.h
 Author:   Laird Technologies
 Version:  0.1

 Description:  BLE connection information details

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


typedef struct
{
    BD_ADDR         remote_addr;                            /* remote peer device address */
    uint16_t        conn_id;                                /* connection ID referenced by the stack */
    uint8_t         flag_indication_sent;                   /* indicates waiting for confirmation */
    uint8_t         flag_stay_connected;                    /* stay connected or disconnect after all messages are sent */
} ble_connection_flags_t;


#define CHAR_DESCRIPTOR_UUID16_VARIABLE(handle, uuid, permission, size) \
    BIT16_TO_8((uint16_t)(handle)), \
    (uint8_t)(permission), \
    (uint8_t)(size), \
    BIT16_TO_8(uuid)

extern const wiced_bt_cfg_settings_t g_wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[];
