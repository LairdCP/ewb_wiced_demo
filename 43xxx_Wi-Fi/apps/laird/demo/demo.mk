#
# Laird Proprietary and Confidential. Copyright 2017 Laird
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Laird Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Laird Corporation.
#

NAME := Sterling_Wifi_BLE_Demo

$(NAME)_PREBUILT_LIBRARY := sensor/libalgobsec.a

$(NAME)_SOURCES := sterling_demo.c  \
                   ble/profiles/wifi_config_profile.c \
                   ble/profiles/aws_config_profile.c \
                   ble/profiles/simple_sensor_profile.c \
                   ble/ble_callbacks.c \
                   ble/gatt_db.c \
                   ble/wiced_bt_cfg.c \
                   aws_iot/sd_aws.c \
                   sensor/bme680.c \
                   sensor/bsec_integration.c \
                   sensor/bsec_thread.c \
                   ui/sd_user_io.c \
                   sd_sterling_demo_dct.c \
                   sd_mem_diag.c

$(NAME)_INCLUDES   := . \
                      ble \
                      ble/profiles \
                      aws_iot \
                      sensor \
                      ui

# The following line allows you to hard-code wifi configuration
# in the DCT. Be sure to update the contents of wifi_config_dct.h
# with your AP details before enabling the following line. 
# To enable use of the AP information hard-coded into 
# wifi_config_dct.h, uncomment the following line:
#WIFI_CONFIG_DCT_H  := wifi_config_dct.h

APPLICATION_DCT := sd_sterling_demo_dct.c

$(NAME)_COMPONENTS := libraries/drivers/bluetooth/low_energy protocols/AWS protocols/SNTP

# define for testing with the BCM94343WWCD1 platform files
# do not define for the sterling platform
GLOBAL_DEFINES += EXCLUDE_CUSTOM_PLATFORM
GLOBAL_DEFINES += APPLICATION_STACK_SIZE=8000

VALID_PLATFORMS =  BCM94343WWCD1
VALID_PLATFORMS += LAIRD_EWB
