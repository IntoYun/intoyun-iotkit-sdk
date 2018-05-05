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

#ifndef __IOTX_DATAPOINT_API_H__
#define __IOTX_DATAPOINT_API_H__

#include "sdk_config.h"

#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1

#ifdef __cplusplus
extern "C"
{
#endif

#define DPID_DEFAULT_BOOL_RESET                   0x7F80        //默认数据点  复位
#define DPID_DEFAULT_BOOL_GETALLDATAPOINT         0x7F81        //默认数据点  获取所有数据点

// transmit
typedef enum {
    DP_TRANSMIT_MODE_MANUAL = 0,       // 用户控制发送
    DP_TRANSMIT_MODE_AUTOMATIC,        // 系统自动发送
} dp_transmit_mode_t;

// Permission
typedef enum {
    DP_PERMISSION_UP_ONLY = 0,   //只上送
    DP_PERMISSION_DOWN_ONLY,     //只下发
    DP_PERMISSION_UP_DOWN        //可上送下发
} dp_permission_t;

typedef enum{
    DATA_TYPE_BOOL = 0,   //bool型
    DATA_TYPE_NUM,        //数值型
    DATA_TYPE_ENUM,       //枚举型
    DATA_TYPE_STRING,     //字符串型
    DATA_TYPE_BINARY      //透传型
} data_type_t;

typedef enum{
    RESULT_DATAPOINT_OLD  = 0,   // 旧数据
    RESULT_DATAPOINT_NEW  = 1,   // 新收取数据
    RESULT_DATAPOINT_NONE = 2,   // 没有该数据点
} read_datapoint_result_t;

//float型属性
typedef struct {
    double minValue;
    double maxValue;
    int resolution;
} number_property_t;

//透传型属性
typedef struct {
    uint8_t *value;
    uint16_t len;
} binary_property_t;

// Property configuration
typedef struct {
    uint16_t dpID;
    data_type_t dataType;
    dp_permission_t permission;
    long runtime;
    bool change; //数据是否改变 true 数据有变化
    read_datapoint_result_t readFlag;
    number_property_t numberProperty;
    bool boolValue;
    int32_t numberIntValue;
    double numberDoubleValue;
    int enumValue;
    char *stringValue;
    binary_property_t binaryValue;
} property_conf;

//datapoint control
typedef struct {
    dp_transmit_mode_t datapoint_transmit_mode;  // 数据点发送类型
    uint32_t datapoint_transmit_lapse;           // 数据点自动发送 时间间隔
    long runtime;                                // 数据点间隔发送的运行时间
} datapoint_control_t;

void IOT_DataPoint_Control(dp_transmit_mode_t mode, uint32_t lapse);

void IOT_DataPoint_DefineBool(const uint16_t dpID, dp_permission_t permission, const bool value);
void IOT_DataPoint_DefineNumber(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value);
void IOT_DataPoint_DefineEnum(const uint16_t dpID, dp_permission_t permission, const int value);
void IOT_DataPoint_DefineString(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const char *value);
void IOT_DataPoint_DefineBinary(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const uint8_t *value, const uint16_t len);

read_datapoint_result_t IOT_DataPoint_ReadBool(const uint16_t dpID, bool *value);
read_datapoint_result_t IOT_DataPoint_ReadNumberInt32(const uint16_t dpID, int32_t *value);
read_datapoint_result_t IOT_DataPoint_ReadNumberDouble(const uint16_t dpID, double *value);
read_datapoint_result_t IOT_DataPoint_ReadEnum(const uint16_t dpID, int *value);
read_datapoint_result_t IOT_DataPoint_ReadString(const uint16_t dpID, char **value);
read_datapoint_result_t IOT_DataPoint_ReadBinary(const uint16_t dpID, uint8_t **value, uint16_t *len);

void IOT_DataPoint_WriteBool(const uint16_t dpID, bool value);
void IOT_DataPoint_WriteNumberInt32(const uint16_t dpID, int32_t value);
void IOT_DataPoint_WriteNumberDouble(const uint16_t dpID, double value);
void IOT_DataPoint_WriteEnum(const uint16_t dpID, int value);
void IOT_DataPoint_WriteString(const uint16_t dpID, const char *value);
void IOT_DataPoint_WriteBinary(const uint16_t dpID, const uint8_t *value, uint16_t len);

int IOT_DataPoint_SendBool(const uint16_t dpID, bool value);
int IOT_DataPoint_SendNumberInt32(const uint16_t dpID, int32_t value);
int IOT_DataPoint_SendNumberDouble(const uint16_t dpID, double value);
int IOT_DataPoint_SendEnum(const uint16_t dpID, int value);
int IOT_DataPoint_SendString(const uint16_t dpID, const char *value);
int IOT_DataPoint_SendBinary(const uint16_t dpID, const uint8_t *value, uint16_t len);

void IOT_DataPoint_ParseReceiveDatapoints(const uint8_t *payload, uint32_t len);
int IOT_DataPoint_SendAllDatapointManual(void);
int IOT_DataPoint_SendDatapointAutomatic(void);

#ifdef __cplusplus
}
#endif

#endif
#endif

