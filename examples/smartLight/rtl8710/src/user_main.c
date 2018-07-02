/*
 * Copyright (c) 2013-2018 Molmc Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_misc.h"
#include "esp_sta.h"
#include "esp_softap.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "iot_export.h"
#include "network.h"

//#define CONFIG_WIFI_SSID      "TP-LINK_3816"
#define CONFIG_WIFI_SSID      "MOLMC_HUAWEI"
#define CONFIG_WIFI_PASSWORD  "26554422"

static os_timer_t timer;
extern int userMain(void);

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32_t user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32_t rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;

        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

static void wait_for_connection_ready(uint8_t flag)
{
    os_timer_disarm(&timer);

    if (wifi_station_connected()) {
        printf("connected\n");
        Network.setState(IOTX_NETWORK_STATE_CONNECTED);
    } else {
        printf("reconnect after 2s\n");
        os_timer_setfn(&timer, (os_timer_func_t*)wait_for_connection_ready, NULL);
        os_timer_arm(&timer, 2000, 0);
    }
}

static void on_wifi_connect(void)
{
    os_timer_disarm(&timer);
    os_timer_setfn(&timer, (os_timer_func_t *)wait_for_connection_ready, NULL);
    os_timer_arm(&timer, 100, 0);
}

static void on_wifi_disconnect(uint8_t reason)
{
    printf("disconnect %d\n", reason);
    Network.setState(IOTX_NETWORK_STATE_DISCONNECTED);
}

static void intoyun_iot_task(void *param)
{
    while(!wifi_station_connected()) {
        vTaskDelay(100);
    }
    userMain();
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());

    set_on_station_connect(on_wifi_connect);
    set_on_station_disconnect(on_wifi_disconnect);
    init_esp_wifi();
    stop_wifi_ap();
    start_wifi_station(CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD);

    Log.setLogLevel("*", MOLMC_LOG_VERBOSE);
    Log.setLogLevel("user:project", MOLMC_LOG_VERBOSE);
    Log.setLogLevel("user:ota", MOLMC_LOG_VERBOSE);

    if (xTaskCreate(&intoyun_iot_task, "intoyun_iot_task", 4096, NULL, 5, NULL) != pdPASS) {
        printf("create intoyun_iot_task task error!");
    }
}

