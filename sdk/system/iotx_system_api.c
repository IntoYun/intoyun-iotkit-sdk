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

#include <string.h>

#include "iot_import.h"
#include "iotx_log_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if_api.h"

//const static char *TAG = "sdk:system";

static iotx_device_info_t   iotx_device_info;
static int                  iotx_devinfo_inited = 0;
static event_handler_t      eventHandler = NULL;

iotx_device_info_pt iotx_device_info_get(void)
{
    IOT_SYSTEM_DeviceInit();
    return &iotx_device_info;
}

void IOT_SYSTEM_DeviceInit(void)
{
    if (iotx_devinfo_inited) {
        return;
    }

    memset(&iotx_device_info, 0x0, sizeof(iotx_device_info_t));
    iotx_devinfo_inited = 1;
}

void IOT_SYSTEM_SetDeviceInfo(char *deviceId, char *deviceSecret, char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion)
{
    iotx_device_info_pt pdev_info = iotx_device_info_get();
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    strncpy(pdev_info->device_id, deviceId, DEVICE_ID_LEN);
    strncpy(pdev_info->device_secret, deviceSecret, DEVICE_SECRET_LEN);
    strncpy(pdev_info->product_id, productID, PRODUCT_ID_LEN);
    strncpy(pdev_info->product_secret, productSecret, PRODUCT_SECRET_LEN);
    strncpy(pdev_info->hardware_version, hardwareVersion, HARDWARE_VERSION_LEN);
    strncpy(pdev_info->software_version, softwareVersion, SOFTWARE_VERSION_LEN);

    strncpy(pconn_info->password, pdev_info->device_secret, DEVICE_SECRET_LEN);
}

void IOT_SYSTEM_Init(void)
{
    HAL_SystemInit();
    IOT_SYSTEM_DeviceInit();
    IOT_Comm_Init();
}

void IOT_SYSTEM_Loop(void)
{
    IOT_Comm_Yield();
}

void IOT_SYSTEM_SetEventCallback(event_handler_t handler)
{
    if(handler != NULL) {
        eventHandler = handler;
    }
}

void IOT_SYSTEM_NotifyEvent(iotx_system_event_t event, iotx_system_events_param_t param, uint8_t *data, uint32_t len)
{
    if(eventHandler != NULL) {
        eventHandler(event, param, data, len);
    }
}

