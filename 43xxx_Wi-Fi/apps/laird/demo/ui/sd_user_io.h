/***************************************************************************************************

 File:     sd_user_io.h
 Author:   Laird Technologies
 Version:  0.1

 Description:  Public interface for the content of user_io.c

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

/******************************************************************************
* Constants
******************************************************************************/

typedef enum
{
    LED_USER_1, /* red */
    LED_USER_2, /* blue */
    LED_USER_3, /* green */
    LED_STATUS, /* red */
    LED__NUM,
} LED_INDEX_T;

/******************************************************************************
* Global variables
******************************************************************************/

extern uint32_t sd_led_state[LED__NUM];

/******************************************************************************
* Function prototypes
******************************************************************************/

wiced_result_t sd_update_leds (void);
wiced_bool_t sd_button_get_1 (void);
wiced_bool_t sd_button_get_2 (void);

