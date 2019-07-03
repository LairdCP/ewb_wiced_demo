#pragma once

#include "wiced.h"

/******************************************************************************
* Type definitions
******************************************************************************/

/* Type definition for data captured from the sensor */
typedef struct
{
    /* Mutex to prevent multiple access to the structure */
    wiced_mutex_t mutex;

    /* Data */
    wiced_utc_time_ms_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    float iaq;
    uint8_t iaq_accuracy;
} BSEC_DATA_T;

/******************************************************************************
* Global variables
******************************************************************************/

/* Global variable holding thread data */
extern BSEC_DATA_T bsec_thread_data;

/******************************************************************************
* Function prototypes
******************************************************************************/

/* Sensor thread */
void bsec_thread (wiced_thread_arg_t arg);
