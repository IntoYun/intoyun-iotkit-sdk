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

#ifndef __IOT_EXPORT_CLOUD_H__
#define __IOT_EXPORT_CLOUD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_config.h"
#include "iotx_datapoint_api.h"

typedef struct
{
    void (*connect)(void);
    bool (*connected)(void);
    void (*disconnect)(void);
    //发送自定义数据
    int (*sendCustomData)(const uint8_t *buffer, uint16_t length);
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    //数据点格式编程API接口
    //定义数据点
    void (*defineDatapointBool)(const uint16_t dpID, dp_permission_t permission, const bool value);
    void (*defineDatapointNumber)(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value);
    void (*defineDatapointEnum)(const uint16_t dpID, dp_permission_t permission, const int value);
    void (*defineDatapointString)(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const char *value);
    void (*defineDatapointBinary)(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const uint8_t *value, const uint16_t len);
    //读取数据点
    read_datapoint_result_t (*readDatapointBool)(const uint16_t dpID, bool *value);
    read_datapoint_result_t (*readDatapointNumberInt32)(const uint16_t dpID, int32_t *value);
    read_datapoint_result_t (*readDatapointNumberDouble)(const uint16_t dpID, double *value);
    read_datapoint_result_t (*readDatapointEnum)(const uint16_t dpID, int *value);
    read_datapoint_result_t (*readDatapointString)(const uint16_t dpID, char **value);
    read_datapoint_result_t (*readDatapointBinary)(const uint16_t dpID, uint8_t **value, uint16_t *len);
    //写数据点
    void (*writeDatapointBool)(const uint16_t dpID, bool value);
    void (*writeDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    void (*writeDatapointNumberDouble)(const uint16_t dpID, double value);
    void (*writeDatapointEnum)(const uint16_t dpID, int value);
    void (*writeDatapointString)(const uint16_t dpID, const char *value);
    void (*writeDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    //发送数据点值
    int (*sendDatapointBool)(const uint16_t dpID, bool value);
    int (*sendDatapointNumberInt32)(const uint16_t dpID, int32_t value);
    int (*sendDatapointNumberDouble)(const uint16_t dpID, double value);
    int (*sendDatapointEnum)(const uint16_t dpID, int value);
    int (*sendDatapointString)(const uint16_t dpID, const char *value);
    int (*sendDatapointBinary)(const uint16_t dpID, const uint8_t *value, uint16_t len);
    int (*sendDatapointAll)(void);
#endif
} iot_cloud_if_t;

extern const iot_cloud_if_t Cloud;

#ifdef __cplusplus
}
#endif

#endif

