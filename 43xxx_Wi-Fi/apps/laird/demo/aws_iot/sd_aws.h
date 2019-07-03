#pragma once

#include "wiced.h"

#include "sterling_demo.h"
#include "sd_sterling_demo_dct.h"

/******************************************************************************
* Constants
******************************************************************************/

#define DEV_FW_NAME_BUF_SIZE    64
#define DEV_FW_VER_BUF_SIZE     16

typedef enum
{
    AWS_PROV_STATUS_UNPROVISIONED,
    AWS_PROV_STATUS_DISCONNECTED,
    AWS_PROV_STATUS_CONNECTED,
    AWS_PROV_STATUS_CONNECT_ERROR,
} AWS_PROV_STATUS_T;

/******************************************************************************
* Type definitions
******************************************************************************/

typedef struct
{
    uint8_t ca_cert[DEV_CERT_SIZE];

    uint8_t aws_endpoint[DEV_AWS_ENDPOINT_SIZE];
    uint8_t client_id[DEV_CLIENT_ID_SIZE];

    uint32_t aws_report_interval;
    uint32_t aws_report_queue;
    uint32_t aws_shadow_update_interval;

    uint8_t client_cert[DEV_CERT_SIZE];
    uint8_t client_key[DEV_KEY_SIZE];
} AWS_CONFIG_DATA_T;

typedef struct sd_device_data
{
   char firmwareName[DEV_FW_NAME_BUF_SIZE];
   char firmwareVer[DEV_FW_VER_BUF_SIZE];
} sd_device_data_t;

/******************************************************************************
* Global variables
******************************************************************************/

extern AWS_CONFIG_DATA_T sd_aws_config;
extern uint8_t aws_config_prov_status;

/******************************************************************************
* Function prototypes
******************************************************************************/

void sd_aws_init (void);
void sd_aws_config_updated (void);
void sd_aws_update_data(SD_REPORT_DATA_T *data);
