#pragma once

/******************************************************************************
* Constants
******************************************************************************/

#define SD_REPORT_CHANGED_TEMPERATURE  0x01
#define SD_REPORT_CHANGED_PRESSURE     0x02
#define SD_REPORT_CHANGED_HUMIDITY     0x04
#define SD_REPORT_CHANGED_IAQ          0x08
#define SD_REPORT_CHANGED_BUTTONS      0x10

typedef enum
{
    WIFI_CONNECTION_STATE_NOT_PROVISIONED,
    WIFI_CONNECTION_STATE_DISCONNECTED,
    WIFI_CONNECTION_STATE_CONNECTING,
    WIFI_CONNECTION_STATE_CONNECTED,
    WIFI_CONNECTION_STATE_DISCONNECTING,
} WIFI_CONNECTION_STATE_T;

/******************************************************************************
* Type definitions
******************************************************************************/

typedef struct
{
    uint8_t changed_flags;

    wiced_utc_time_ms_t timestamp;
    float temperature;
    float humidity;
    float pressure;
    float iaq;
    uint8_t iaq_accuracy;

    uint8_t button1;
    uint8_t button2;
} SD_REPORT_DATA_T;

/******************************************************************************
* Global variables
******************************************************************************/

extern WIFI_CONNECTION_STATE_T wifi_connection_state;

/******************************************************************************
* Function prototypes
******************************************************************************/

wiced_bool_t sd_is_wifi_up (void);
void sd_request_wifi (wiced_bool_t enable);
void sd_notify_ble_enabled (wiced_bool_t enabled);
