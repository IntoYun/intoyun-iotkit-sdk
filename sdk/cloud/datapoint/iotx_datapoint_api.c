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

#include "iot_import.h"
#include "iotx_datapoint_api.h"

const static char *TAG = "sdk:ota";

#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1

property_conf *properties[CONFIG_PROPERTIES_MAX];

int properties_count = 0;

//初始化设置参数 自动发送 发送间隔时间  发送运行时间
volatile datapoint_control_t g_datapoint_control = {DP_TRANSMIT_MODE_AUTOMATIC, CONFIG_AUTOMATIC_INTERVAL, 0};

static uint16_t intoyunFormSingleDatapoint(int property_index, uint8_t *buffer, uint16_t len);
static int intoyunTransmitData(const uint8_t *buffer, uint16_t len);
static int intoyunSendSingleDatapoint(const uint16_t dpID);
static int IOT_DataPoint_SendAll(void);

static int getPropertyIndex(const uint16_t dpID)
{
    for (int index = 0; index < properties_count; index++) {
        if (properties[index]->dpID == dpID) {
            return index;
        }
    }
    return -1;
}

static bool isPropertyChanged(void)
{
    for (int i = 0; i < properties_count; i++) {
        if (properties[i]->change) {
            return true;
        }
    }
    return false;
}

static uint8_t getPropertyCount(void)
{
    return properties_count;
}

static void clearPropertyChangeFlag(void)
{
    for (int i = 0; i < properties_count; i++) {
        if (properties[i]->change) {
            properties[i]->change = false;
        }
    }
}

static double _pow(double base, int exponent)
{
    double result = 1.0;
    int i = 0;

    for (i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}

static dp_transmit_mode_t getDatapointTransmitMode(void)
{
    return g_datapoint_control.datapoint_transmit_mode;
}

void IOT_DataPoint_Control(dp_transmit_mode_t mode, uint32_t lapse)
{
    g_datapoint_control.datapoint_transmit_mode = mode;
    if(DP_TRANSMIT_MODE_AUTOMATIC == g_datapoint_control.datapoint_transmit_mode) {
        if(lapse < CONFIG_AUTOMATIC_INTERVAL) {
            g_datapoint_control.datapoint_transmit_lapse = CONFIG_AUTOMATIC_INTERVAL;
        } else {
            g_datapoint_control.datapoint_transmit_lapse = lapse;
        }
    }
}

static void setDatapointValueInit(uint16_t count, uint16_t dpID, data_type_t dataType, dp_permission_t permission)
{
    properties[count]=(property_conf*)HAL_Malloc(sizeof(property_conf));

    properties[count]->dpID       = dpID;
    properties[count]->dataType   = dataType;
    properties[count]->permission = permission;
    properties[count]->runtime    = 0;
    properties[count]->readFlag   = RESULT_DATAPOINT_OLD;
}

void IOT_DataPoint_DefineBool(const uint16_t dpID, dp_permission_t permission, const bool value)
{
    if (-1 == getPropertyIndex(dpID)) {
        setDatapointValueInit(properties_count,dpID,DATA_TYPE_BOOL,permission);
        properties[properties_count]->boolValue=value;
        properties_count++; // count the number of properties
    }
}

void IOT_DataPoint_DefineNumber(const uint16_t dpID, dp_permission_t permission, const double minValue, const double maxValue, const int resolution, const double value)
{
    if (-1 == getPropertyIndex(dpID)) {
        setDatapointValueInit(properties_count,dpID,DATA_TYPE_NUM,permission);

        double defaultValue = value;
        if(defaultValue < minValue) {
            defaultValue = minValue;
        } else if(defaultValue > maxValue) {
            defaultValue = maxValue;
        }

        if(resolution == 0) {
            properties[properties_count]->numberIntValue=value;
        } else {
            properties[properties_count]->numberDoubleValue=value;
        }
        properties[properties_count]->numberProperty.minValue = minValue;
        properties[properties_count]->numberProperty.maxValue = maxValue;
        properties[properties_count]->numberProperty.resolution = resolution;
        properties_count++; // count the number of properties
    }
}

void IOT_DataPoint_DefineEnum(const uint16_t dpID, dp_permission_t permission, const int value)
{
    if (-1 == getPropertyIndex(dpID)) {
        int defaultValue = value;
        if(defaultValue < 0) {
            defaultValue = 0;
        }

        setDatapointValueInit(properties_count,dpID,DATA_TYPE_ENUM,permission);
        properties[properties_count]->enumValue = defaultValue;
        properties_count++; // count the number of properties
    }
}

void IOT_DataPoint_DefineString(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const char *value)
{
    if (-1 == getPropertyIndex(dpID)) {
        setDatapointValueInit(properties_count,dpID,DATA_TYPE_STRING,permission);
        properties[properties_count]->stringValue = (char *)HAL_Malloc(strlen(value)+1);
        strncpy(properties[properties_count]->stringValue,value,strlen(value)+1);
        properties_count++; // count the number of properties
    }
}

void IOT_DataPoint_DefineBinary(const uint16_t dpID, dp_permission_t permission, const uint16_t maxLen, const uint8_t *value, const uint16_t len)
{
    if (-1 == getPropertyIndex(dpID)) {
        setDatapointValueInit(properties_count,dpID,DATA_TYPE_BINARY,permission);
        properties[properties_count]->binaryValue.value = (uint8_t *)HAL_Malloc(len);
        for(uint8_t i=0;i<len;i++) {
            properties[properties_count]->binaryValue.value[i] = value[i];
        }
        properties[properties_count]->binaryValue.len = (uint16_t)len;
        properties_count++; // count the number of properties
    }
}

read_datapoint_result_t IOT_DataPoint_ReadBool(const uint16_t dpID, bool *value)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->boolValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t IOT_DataPoint_ReadNumberInt32(const uint16_t dpID, int32_t *value)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->numberIntValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t IOT_DataPoint_ReadNumberDouble(const uint16_t dpID, double *value)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->numberDoubleValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t IOT_DataPoint_ReadEnum(const uint16_t dpID, int *value)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->enumValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t IOT_DataPoint_ReadString(const uint16_t dpID, char **value)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->stringValue;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;
    return readResult;
}

read_datapoint_result_t IOT_DataPoint_ReadBinary(const uint16_t dpID, uint8_t **value, uint16_t *len)
{
    int index = getPropertyIndex(dpID);
    if (index == -1) {
        return RESULT_DATAPOINT_NONE;
    }

    (*value) = properties[index]->binaryValue.value;
    (*len) = properties[index]->binaryValue.len;
    read_datapoint_result_t readResult = properties[index]->readFlag;
    properties[index]->readFlag = RESULT_DATAPOINT_OLD;

    return readResult;
}

// type   0: 平台控制写数据   1：用户写数据
static void writeDatapointBool(const uint16_t dpID, bool value, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_BOOL) {
        return;
    } else {
        if(properties[index]->boolValue != value) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->boolValue = value;
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

static void writeDatapointNumberInt32(const uint16_t dpID, int32_t value, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_NUM || properties[index]->numberProperty.resolution != 0) {
        return;
    } else {
        int32_t tmp = value;
        if(tmp < properties[index]->numberProperty.minValue) {
            tmp = properties[index]->numberProperty.minValue;
        } else if(tmp > properties[index]->numberProperty.maxValue) {
            tmp = properties[index]->numberProperty.maxValue;
        }

        if(properties[index]->numberIntValue != value) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->numberIntValue = tmp;
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

static void writeDatapointNumberDouble(const uint16_t dpID, double value, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_NUM || properties[index]->numberProperty.resolution == 0) {
        return;
    } else {
        int32_t tmp;
        double d = value;
        if(d < properties[index]->numberProperty.minValue) {
            d = properties[index]->numberProperty.minValue;
        } else if(d > properties[index]->numberProperty.maxValue) {
            d = properties[index]->numberProperty.maxValue;
        }

        //保证小数点位数
        tmp = (int32_t)(d * _pow(10, properties[index]->numberProperty.resolution));

        if(properties[index]->numberDoubleValue != d) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }

            properties[index]->numberDoubleValue = tmp/(double)_pow(10, properties[index]->numberProperty.resolution);
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

static void writeDatapointEnum(const uint16_t dpID, int value, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_ENUM) {
        return;
    } else {
        if(properties[index]->enumValue != value) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->enumValue = value;
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

static void writeDatapointString(const uint16_t dpID, const char *value, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_STRING) {
        return;
    } else {
        if(strcmp(properties[index]->stringValue,value) != 0) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
            properties[index]->stringValue = (char *)HAL_Malloc(strlen(value)+1);
            strncpy(properties[index]->stringValue,value,strlen(value)+1);
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

static void writeDatapointBinary(const uint16_t dpID, const uint8_t *value, uint16_t len, uint8_t type)
{
    int index = getPropertyIndex(dpID);
    if(index == -1) {
        return;
    }

    if(properties[index]->dataType != DATA_TYPE_BINARY) {
        return;
    } else {

        if((properties[index]->binaryValue.len != len) || memcmp(properties[index]->binaryValue.value, value, len)) {
            properties[index]->change = true;
            if(type) { //用户操作
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }

            properties[index]->binaryValue.value = (uint8_t *)HAL_Malloc(len);
            memcpy(properties[index]->binaryValue.value, value, len);
            properties[index]->binaryValue.len = (uint16_t)len;
        } else {
            if(type) { //用户操作
                properties[index]->change = false;
                properties[index]->readFlag = RESULT_DATAPOINT_OLD;
            } else {
                properties[index]->change = true;
                properties[index]->readFlag = RESULT_DATAPOINT_NEW;
            }
        }
    }
}

void IOT_DataPoint_WriteBool(const uint16_t dpID, bool value)
{
    writeDatapointBool(dpID, value, 1);
}

void IOT_DataPoint_WriteNumberInt32(const uint16_t dpID, int32_t value)
{
    writeDatapointNumberInt32(dpID, value, 1);
}

void IOT_DataPoint_WriteNumberDouble(const uint16_t dpID, double value)
{
    writeDatapointNumberDouble(dpID, value, 1);
}

void IOT_DataPoint_WriteEnum(const uint16_t dpID, int value)
{
    writeDatapointEnum(dpID, value, 1);
}

void IOT_DataPoint_WriteString(const uint16_t dpID, const char *value)
{
    writeDatapointString(dpID, value, 1);
}

void IOT_DataPoint_WriteBinary(const uint16_t dpID, const uint8_t *value, uint16_t len)
{
    writeDatapointBinary(dpID, value, len, 1);
}

int IOT_DataPoint_SendBool(const uint16_t dpID, bool value)
{
    int index = getPropertyIndex(dpID);

    if (index == -1) {
        return -1;
    }

    writeDatapointBool(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if( properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

int IOT_DataPoint_SendNumberInt32(const uint16_t dpID, int32_t value)
{
    int index = getPropertyIndex(dpID);

    if(index == -1) {
        return -1;
    }

    writeDatapointNumberInt32(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if( properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

int IOT_DataPoint_SendNumberDouble(const uint16_t dpID, double value)
{
    int index = getPropertyIndex(dpID);

    if (index == -1) {
        // not found, nothing to do
        return -1;
    }

    writeDatapointNumberDouble(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if( properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

int IOT_DataPoint_SendEnum(const uint16_t dpID, int value)
{
    int index = getPropertyIndex(dpID);

    if(index == -1) {
        // not found, nothing to do
        return -1;
    }

    writeDatapointEnum(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if( properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

int IOT_DataPoint_SendString(const uint16_t dpID, const char *value)
{
    int index = getPropertyIndex(dpID);

    if (index == -1) {
        // not found, nothing to do
        return -1;
    }

    writeDatapointString(dpID, value, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if(properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

int IOT_DataPoint_SendBinary(const uint16_t dpID, const uint8_t *value, uint16_t len)
{
    int index = getPropertyIndex(dpID);

    if(index == -1) {
        // not found, nothing to do
        return -1;
    }

    writeDatapointBinary(dpID, value, len, 1);

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    //只允许下发
    if( properties[index]->permission == DP_PERMISSION_DOWN_ONLY) {
        return -1;
    }

    return intoyunSendSingleDatapoint(index);
}

void IOT_DataPoint_ParseReceiveDatapoints(const uint8_t *payload, uint32_t len)
{
    int32_t index = 0;
    uint16_t dpID = 0;
    uint8_t dataType;
    uint16_t dataLength=0;

    //0x31 dpid(1-2 bytes)+data type(1 byte)+data len(1-2 bytes)+data(n bytes)
    //大端表示，如果最高位是1，则表示两个字节，否则是一个字节
    if((payload[0] != 0x31 ) || (len == 0)) {
        return;
    }

    index++;
    while(index < len) {
        dpID = payload[index++] & 0xff;
        if(dpID >= 0x80) {
            //数据点有2个字节
            dpID = dpID & 0x7f; //去掉最高位
            dpID = (dpID << 8) | payload[index++];
        }

        dataType = payload[index++];
        switch(dataType) {
            case DATA_TYPE_BOOL:
                {
                    dataLength = payload[index++] & 0xff;
                    bool value = payload[index++];
                    if(getPropertyIndex(dpID) != -1) {
                        //数据点存在
                        writeDatapointBool(dpID, value, 0);
                    }
                }
                break;

            case DATA_TYPE_NUM:
                {
                    dataLength = payload[index++] & 0xff;
                    int32_t value = payload[index++] & 0xff;
                    if(dataLength == 2) {
                        value = (value << 8) | payload[index++];
                    } else if(dataLength == 3) {
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                    } else if(dataLength == 4) {
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                    }

                    uint8_t id = getPropertyIndex(dpID);
                    if(properties[id]->numberProperty.resolution == 0) {
                        //此数据点为int
                        value = value + properties[id]->numberProperty.minValue;
                        if(getPropertyIndex(dpID) != -1) {
                            //数据点存在
                            writeDatapointNumberInt32(dpID, value, 0);
                        }
                    } else {
                        double dValue = (value/(double)_pow(10, properties[id]->numberProperty.resolution)) + properties[id]->numberProperty.minValue;
                        if(getPropertyIndex(dpID) != -1) {
                            //数据点存在
                            writeDatapointNumberDouble(dpID, dValue, 0);
                        }
                    }
                }
                break;

            case DATA_TYPE_ENUM:
                {
                    dataLength = payload[index++] & 0xff;
                    int value = payload[index++] & 0xff;
                    if(dataLength == 2) {
                        value = (value << 8) | payload[index++];
                    } else if(dataLength == 3) {
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                    } else if(dataLength == 4) {
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                        value = (value << 8) | payload[index++];
                    }

                    if(getPropertyIndex(dpID) != -1) {
                        //数据点存在
                        writeDatapointEnum(dpID, value, 0);
                    }
                }
                break;

            case DATA_TYPE_STRING:
                {
                    dataLength = payload[index++] & 0xff;
                    if(dataLength >= 0x80) {
                        //数据长度有2个字节
                        dataLength = dataLength & 0x7f;
                        dataLength = (dataLength) << 8 | payload[index++];
                    }
                    char *str = (char *)HAL_Malloc(dataLength+1);
                    if(NULL != str) {
                        memset(str, 0, dataLength+1);
                        memcpy(str, &payload[index], dataLength);
                    }
                    index += dataLength;
                    if(getPropertyIndex(dpID) != -1) {
                        //数据点存在
                        writeDatapointString(dpID, str, 0);
                    }
                    free(str);
                }
                break;

            case DATA_TYPE_BINARY:
                {
                    dataLength = payload[index++] & 0xff;
                    if(dataLength >= 0x80) {
                        //数据长度有2个字节
                        dataLength = dataLength & 0x7f;
                        dataLength = (dataLength) << 8 | payload[index++];
                    }

                    if(getPropertyIndex(dpID) != -1) {
                        //数据点存在
                        writeDatapointBinary(dpID, &payload[index], dataLength, 0);
                    }

                    index += dataLength;
                }
                break;

            default:
                break;
        }
    }
    bool dpReset = false;
    bool dpGetAllDatapoint = false;
    if (RESULT_DATAPOINT_NEW == IOT_DataPoint_ReadBool(DPID_DEFAULT_BOOL_RESET, &dpReset)) {
        HAL_SystemReboot();
    } else if (RESULT_DATAPOINT_NEW == IOT_DataPoint_ReadBool(DPID_DEFAULT_BOOL_GETALLDATAPOINT, &dpGetAllDatapoint)) {
        IOT_DataPoint_SendAll();
    }
}

//组织数据点数据
static uint16_t intoyunFormDataPointBinary(int property_index, uint8_t* buffer)
{
    int32_t index = 0;

    if(properties[property_index]->dpID < 0x80) {
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    } else {
        buffer[index++] = (properties[property_index]->dpID >> 8) | 0x80;
        buffer[index++] = properties[property_index]->dpID & 0xFF;
    }

    switch(properties[property_index]->dataType) {
        case DATA_TYPE_BOOL:       //bool型
            buffer[index++] = 0x00;  //类型
            buffer[index++] = 0x01;  //长度
            buffer[index++] = (bool)(properties[property_index]->boolValue);
            break;

        case DATA_TYPE_NUM:        //数值型 int型
            {
                buffer[index++] = 0x01;
                int32_t value;
                if(properties[property_index]->numberProperty.resolution == 0) {
                    value = (int32_t)properties[property_index]->numberIntValue;
                } else {
                    value = (properties[property_index]->numberDoubleValue - properties[property_index]->numberProperty.minValue) \
                            * _pow(10, properties[property_index]->numberProperty.resolution);
                }

                if(value & 0xFFFF0000) {
                    buffer[index++] = 0x04;
                    buffer[index++] = (value >> 24) & 0xFF;
                    buffer[index++] = (value >> 16) & 0xFF;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else if(value & 0xFFFFFF00) {
                    buffer[index++] = 0x02;
                    buffer[index++] = (value >> 8) & 0xFF;
                    buffer[index++] = value & 0xFF;
                } else {
                    buffer[index++] = 0x01;
                    buffer[index++] = value & 0xFF;
                }
            }
            break;

        case DATA_TYPE_ENUM:       //枚举型
            buffer[index++] = 0x02;
            buffer[index++] = 0x01;
            buffer[index++] = (uint8_t)properties[property_index]->enumValue & 0xFF;
            break;

        case DATA_TYPE_STRING:     //字符串型
            {
                uint16_t strLength = strlen(properties[property_index]->stringValue);

                buffer[index++] = 0x03;
                if(strLength < 0x80) {
                    buffer[index++] = strLength & 0xFF;
                } else {
                    buffer[index++] = (strLength >> 8) | 0x80;
                    buffer[index++] = strLength & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->stringValue, strLength);
                index+=strLength;
                break;
            }

        case DATA_TYPE_BINARY:     //二进制型
            {
                uint16_t len = properties[property_index]->binaryValue.len;
                buffer[index++] = DATA_TYPE_BINARY;
                if(len < 0x80) {
                    buffer[index++] = len & 0xFF;
                } else {
                    buffer[index++] = (len >> 8) | 0x80;
                    buffer[index++] = len & 0xFF;
                }
                memcpy(&buffer[index], properties[property_index]->binaryValue.value, len);
                index+=len;
                break;
            }

        default:
            break;
    }
    return index;
}

//组织单个数据点的数据
static uint16_t intoyunFormSingleDatapoint(int property_index, uint8_t *buffer, uint16_t len)
{
    int32_t index = 0;

    buffer[index++] = 0x31;
    index += intoyunFormDataPointBinary(property_index, buffer+index);
    return index;
}

//发送数据
static int intoyunTransmitData(const uint8_t *buffer, uint16_t len)
{
    int result = IOT_Comm_SendData(buffer, len);
    return result? 0 : 1;
}

//发送单个数据点的数据
static int intoyunSendSingleDatapoint(const uint16_t dpID)
{
    uint8_t buffer[256];
    uint16_t len;

    len = intoyunFormSingleDatapoint(dpID, buffer, sizeof(buffer));
    return intoyunTransmitData(buffer,len);
}

// type   false: 组织改变的数据点   true：组织全部的数据点
//组织所有数据点的数据
static uint16_t intoyunFormAllDatapoint(uint8_t *buffer, uint16_t len, uint8_t type)
{
    int32_t index = 0;

    buffer[index++] = 0x31;
    for (int i = 0; i < properties_count; i++) {
        //只允许下发  不上传
        if (properties[i]->permission == DP_PERMISSION_DOWN_ONLY) {
            continue;
        }

        //系统默认dpID  不上传
        if (properties[i]->dpID > 0x3F00) {
            continue;
        }

        if( type || ((!type) && properties[i]->change) ) {
            index += intoyunFormDataPointBinary(i, (uint8_t *)buffer+index);
        }
    }
    return index;
}

//发送所有数据点的数据
static int IOT_DataPoint_SendAll(void)
{
    uint8_t buffer[512];
    uint16_t len;

    if(0 == getPropertyCount()) {
        return -1;
    }

    len = intoyunFormAllDatapoint(buffer, sizeof(buffer), 1);
    clearPropertyChangeFlag();
    return intoyunTransmitData(buffer,len);
}

int IOT_DataPoint_SendAllDatapointManual(void)
{
    if(0 == getPropertyCount()) {
        return -1;
    }

    if(DP_TRANSMIT_MODE_AUTOMATIC == getDatapointTransmitMode()) {
        return -1;
    }

    clearPropertyChangeFlag();
    return IOT_DataPoint_SendAll();
}

int IOT_DataPoint_SendDatapointAutomatic(void)
{
    bool sendFlag = false;

    if(0 == getPropertyCount()) {
        return -1;
    }

    if(DP_TRANSMIT_MODE_MANUAL == getDatapointTransmitMode()) {
        return -1;
    }

    //当数值发生变化
    if(isPropertyChanged()) {
        sendFlag = true;
    } else {
        //发送时间间隔到
        uint32_t current_millis = HAL_UptimeMs();
        int32_t elapsed_millis = current_millis - g_datapoint_control.runtime;

        if (elapsed_millis < 0) {
            elapsed_millis =  0xFFFFFFFF - g_datapoint_control.runtime + current_millis;
        }
        //发送时间时间到
        if ( elapsed_millis >= g_datapoint_control.datapoint_transmit_lapse*1000 ) {
            sendFlag = true;
        }
    }

    if(sendFlag) {
        MOLMC_LOGI(TAG, "send datapoint automatic!");
        g_datapoint_control.runtime = HAL_UptimeMs();
        clearPropertyChangeFlag();
        return IOT_DataPoint_SendAll();
    }
    return -1;
}

#endif

