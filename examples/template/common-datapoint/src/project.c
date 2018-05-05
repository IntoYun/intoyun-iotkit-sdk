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

#include "iot_export.h"
#include "project_config.h"
#include "ota_update.h"

const static char *TAG = "user:project";

#define DEVICE_ID_DEF                             "0dvo0bdoy00000000000068f"         //设备标识
#define DEVICE_SECRET_DEF                         "c08e66a8b08fd8436dac0dce9cc3bca9" //设备密钥

//定义数据点ID
//格式：DPID_数据类型_数据点英文名. 如果英文名相同的，则在后面添加_1 _2 _3形式。
//说明：布尔型: BOOL. 数值型: INT32或者DOUBLE. 枚举型: ENUM. 字符串型: STRING. 透传型: BINARY
#define DPID_BOOL_SWITCH                      1  //布尔型     开关(可上送可下发)
#define DPID_DOUBLE_TEMPERATURE               2  //浮点型     温度(可上送可下发)
#define DPID_INT32_HUMIDITY                   3  //整型       湿度(可上送可下发)
#define DPID_ENUM_COLOR                       4  //枚举型     颜色模式(可上送可下发)
#define DPID_STRING_DISPLAY                   5  //字符串型   显示字符串(可上送可下发)
#define DPID_BINARY_LOCATION                  6  //透传型     位置(可上送可下发)

//定义数据点变量
//格式：数据类型+数据点英文名. 如果英文名相同的，则在后面添加_1 _2 _3形式。
//布尔型为bool
//枚举型为int
//整型为int32_t
//浮点型为double
//字符型为String
//透传型为uint8_t*型
//透传型长度为uint16_t型
bool dpBoolSwitch;              //开关
double dpDoubleTemperature;     //温度
int32_t dpInt32Humidity;        //湿度
int dpEnumColor;                //颜色模式
char *dpStringDisplay;         //显示字符串
uint8_t *dpBinaryLocation;      //位置
uint16_t dpBinaryLocationLen;   //位置长度

uint32_t timerID;

void eventProcess(int event, int param, uint8_t *data, uint32_t datalen)
{
    if(event == event_cloud_comm) {
        switch(param){
            case ep_cloud_comm_data:
                //开关
                if(RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SWITCH, &dpBoolSwitch)) {
                    MOLMC_LOGI(TAG, "dpBoolSwitch = %d", dpBoolSwitch);
                }
                //温度
                if(RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberDouble(DPID_DOUBLE_TEMPERATURE, &dpDoubleTemperature)) {
                    MOLMC_LOGI(TAG, "dpDoubleTemperature = %f", dpDoubleTemperature);
                }
                //湿度
                if(RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberInt32(DPID_INT32_HUMIDITY, &dpInt32Humidity)) {
                    MOLMC_LOGI(TAG, "dpInt32Humidity = %d", dpInt32Humidity);
                }
                // 颜色模式
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointEnum(DPID_ENUM_COLOR, &dpEnumColor)) {
                    MOLMC_LOGI(TAG, "dpEnumColor = %d", dpEnumColor);
                }
                // 显示字符串
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointString(DPID_STRING_DISPLAY, &dpStringDisplay)) {
                    MOLMC_LOGI(TAG, "dpStringDisplay = %s", dpStringDisplay);
                }
                // 位置信息
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBinary(DPID_BINARY_LOCATION, &dpBinaryLocation, &dpBinaryLocationLen)) {
                    MOLMC_LOGI(TAG, "dpBinaryLocationLen = %d", dpBinaryLocationLen);
                    MOLMC_LOG_BUFFER_HEX(TAG, dpBinaryLocation, dpBinaryLocationLen);
                }
                break;
            case ep_cloud_comm_ota:
                otaUpdate(data, datalen);
                break;
            default:
                break;
        }
    } else if(event == event_network_status) {
        switch(param){
            case ep_network_status_disconnected:  //模组已断开路由器
                MOLMC_LOGI(TAG, "event network disconnect router");
                break;
            case ep_network_status_connected:     //模组已连接路由器
                MOLMC_LOGI(TAG, "event network connect router");
                break;
            default:
                break;
        }
    } else if(event == event_cloud_status) {
        switch(param){
            case ep_cloud_status_disconnected:    //模组已断开平台
                MOLMC_LOGI(TAG, "event cloud disconnect server");
                break;
            case ep_cloud_status_connected:       //模组已连接平台
                MOLMC_LOGI(TAG, "event cloud connect server");
                break;
            default:
                break;
        }
    }
}

void userInit(void)
{
    //初始设备信息
    System.init();
    System.setDeviceInfo(DEVICE_ID_DEF, DEVICE_SECRET_DEF, PRODUCT_ID_DEF, PRODUCT_SECRET_DEF, HARDWARE_VERSION_DEF, SOFTWARE_VERSION_DEF);
    System.setEventCallback(eventProcess);
    //定义产品数据点
    Cloud.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false);                            //开关
    Cloud.defineDatapointNumber(DPID_DOUBLE_TEMPERATURE, DP_PERMISSION_UP_DOWN, 0, 100, 1, 0);            //温度
    Cloud.defineDatapointNumber(DPID_INT32_HUMIDITY, DP_PERMISSION_UP_DOWN, 0, 100, 0, 0);                //湿度
    Cloud.defineDatapointEnum(DPID_ENUM_COLOR, DP_PERMISSION_UP_DOWN, 1);                                 //颜色模式
    Cloud.defineDatapointString(DPID_STRING_DISPLAY, DP_PERMISSION_UP_DOWN, 255, "hello! intoyun!");      //显示字符串
    Cloud.defineDatapointBinary(DPID_BINARY_LOCATION, DP_PERMISSION_UP_DOWN, 255, "\x12\x34\x56\x78", 4); //位置信息
    /*************此处修改和添加用户初始化代码**************/
    Cloud.connect();
    timerID = timerGetId();
    /*******************************************************/
}

void userHandle(void)
{
    if(Cloud.connected()) {
        if(timerIsEnd(timerID, 10000)) {
            timerID = timerGetId();

            //更新数据点数据（数据点具备：上送属性）
            Cloud.writeDatapointBool(DPID_BOOL_SWITCH, dpBoolSwitch);
            Cloud.writeDatapointNumberDouble(DPID_DOUBLE_TEMPERATURE, dpDoubleTemperature);
            Cloud.writeDatapointNumberInt32(DPID_INT32_HUMIDITY, dpInt32Humidity);
            Cloud.writeDatapointEnum(DPID_ENUM_COLOR, dpEnumColor);
            Cloud.writeDatapointString(DPID_STRING_DISPLAY, "hello! intoyun!");
            Cloud.writeDatapointBinary(DPID_BINARY_LOCATION, "\x12\x34\x56\x78", 4);
        }
    }
}

int userMain(void)
{
    userInit();
    while(1) {
        userHandle();
        System.loop();
    }
    return 0;
}

