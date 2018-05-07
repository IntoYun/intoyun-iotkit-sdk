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

#include <limits.h>
#include "iot_import.h"
#include "iot_export.h"
#include "iotx_comm_if_api.h"

uint32_t millis(void)
{
    return HAL_UptimeMs();
}

void delay(uint32_t ms)
{
    uint32_t start_millis = HAL_UptimeMs();
    uint32_t current_millis = 0, elapsed_millis = 0;

    if (ms == 0) return;
    while (1) {
        current_millis = HAL_UptimeMs();
        if (current_millis < start_millis){
            elapsed_millis =  UINT_MAX - start_millis + current_millis;
        } else {
            elapsed_millis = current_millis - start_millis;
        }

        if (elapsed_millis > ms) {
            break;
        }
        IOT_SYSTEM_Loop();
    }
}

uint32_t timerGetId(void)
{
    return HAL_UptimeMs();
}

bool timerIsEnd(uint32_t timerID, uint32_t time)
{
    uint32_t current_millis = HAL_UptimeMs();
    uint32_t elapsed_millis = 0;

    //Check for wrapping
    if (current_millis < timerID){
        elapsed_millis =  UINT_MAX-timerID + current_millis;
    } else {
        elapsed_millis = current_millis - timerID;
    }

    if (elapsed_millis >= time) {
        return true;
    }
    return false;
}

const iot_system_if_t System =
{
    IOT_SYSTEM_Init,
    IOT_SYSTEM_Loop,
    IOT_SYSTEM_SetDeviceInfo,
    IOT_SYSTEM_SetEventCallback,
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    IOT_DataPoint_Control
#endif
};

const iot_network_if_t Network =
{
    IOT_Network_IsConnected,
    IOT_Network_SetState,
};

const iot_cloud_if_t Cloud =
{
    IOT_Comm_Connect,
    IOT_Comm_IsConnected,
    IOT_Comm_Disconnect,
    IOT_Comm_SendData,
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    IOT_DataPoint_DefineBool,
    IOT_DataPoint_DefineNumber,
    IOT_DataPoint_DefineEnum,
    IOT_DataPoint_DefineString,
    IOT_DataPoint_DefineBinary,

    IOT_DataPoint_ReadBool,
    IOT_DataPoint_ReadNumberInt32,
    IOT_DataPoint_ReadNumberDouble,
    IOT_DataPoint_ReadEnum,
    IOT_DataPoint_ReadString,
    IOT_DataPoint_ReadBinary,

    IOT_DataPoint_WriteBool,
    IOT_DataPoint_WriteNumberInt32,
    IOT_DataPoint_WriteNumberDouble,
    IOT_DataPoint_WriteEnum,
    IOT_DataPoint_WriteString,
    IOT_DataPoint_WriteBinary,

    IOT_DataPoint_SendBool,
    IOT_DataPoint_SendNumberInt32,
    IOT_DataPoint_SendNumberDouble,
    IOT_DataPoint_SendEnum,
    IOT_DataPoint_SendString,
    IOT_DataPoint_SendBinary,
    IOT_DataPoint_SendAllDatapointManual,
#endif
};

const iot_coap_client_if_t CoAPClient =
{
    IOT_CoAP_Init,
    IOT_CoAP_Deinit,
    IOT_CoAP_Auth,
    IOT_CoAP_Yield,
    IOT_CoAP_SendMessage,
    IOT_CoAP_GetMessagePayload,
    IOT_CoAP_GetMessageCode,
};

const iot_mqtt_client_if_t MQTTClient =
{
    IOT_MQTT_Construct,
    IOT_MQTT_Destroy,
    IOT_MQTT_Yield,
    IOT_MQTT_CheckStateNormal,
    IOT_MQTT_Subscribe,
    IOT_MQTT_Unsubscribe,
    IOT_MQTT_Publish,
};

const iot_ota_if_t OTAUpdate =
{
    IOT_OTA_Init,
    IOT_OTA_Deinit,
    IOT_OTA_SetProgressCallback,
    IOT_OTA_Update,
    IOT_OTA_ReportProgress,
    IOT_OTA_GetLastError
};

const iot_log_if_t Log =
{
    molmc_log_level_set,
    molmc_log_set_output
};

#if CONFIG_SYSTEM_KEY_ENABLE == 1
const iot_keys_if_t Key =
{
    IOT_KEY_Init,
    IOT_KEY_SetParams,
    IOT_KEY_Register,
    IOT_KEY_ClickCb,
    IOT_KEY_DoubleClickCb,
    IOT_KEY_PressStartCb,
    IOT_KEY_PressStopCb,
    IOT_KEY_PressDuringCb,
    IOT_KEY_Loop,
};
#endif

#if CONFIG_SYSTEM_TIMER_ENABLE == 1
const iot_timers_if_t Timer =
{
    IOT_TIMER_Register,
    IOT_TIMER_ChangePeriod,
    IOT_TIMER_Start,
    IOT_TIMER_Stop,
    IOT_TIMER_Reset,
    IOT_TIMER_Loop,
};
#endif

