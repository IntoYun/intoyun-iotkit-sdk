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

#define DEVICE_ID_DEF                             "0dvo0bdoy00000000000068f"         //设备标识
#define DEVICE_SECRET_DEF                         "c08e66a8b08fd8436dac0dce9cc3bca9" //设备密钥

#define DPID_NUMBER_SOIL_HUMIDITY                 1  //数值型            土壤湿度
#define DPID_NUMBER_AIR_HUMIDITY                  2  //数值型            空气湿度
#define DPID_NUMBER_TEMPERATURE                   3  //数值型            温度
#define DPID_NUMBER_ILLUMINATION                  4  //数值型            光照强度
#define DPID_NUMBER_CO2                           5  //数值型            二氧化碳浓度
#define DPID_ENUM_BIRDS                           6  //枚举型            鸟类危害程度
#define DPID_BOOL_SPRINKLER_SWITCH                7  //布尔型            洒水器开关

double dpDoubleSoil_humidity;                     // 土壤湿度
double dpDoubleAir_humidity;                      // 空气湿度
double dpDoubleTemperature;                       // 温度
double dpDoubleIllumination;                      // 光照强度
int dpIntCO2;                                     // 二氧化碳浓度
int dpEnumBirds;                                  // 鸟类危害程度
bool dpBoolSprinkler_switch;                      // 洒水器开关

void eventProcess(iotx_system_event_t event, iotx_system_events_param_t param, uint8_t *data, uint32_t len)
{
    if(event == event_cloud_comm) {
        switch(param){
            case ep_cloud_comm_data:
                //光照强度
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointNumberDouble(DPID_NUMBER_ILLUMINATION, &dpDoubleIllumination)) {
                    //用户代码
                    log_info("dpDoubleIllumination = %f\r\n", dpDoubleIllumination);
                }
                //洒水器开关
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, &dpBoolSprinkler_switch)) {
                    //用户代码
                    log_info("dpBoolSprinkler_switch = %d\r\n", dpBoolSprinkler_switch);
                }
                break;
            case ep_cloud_comm_ota:
                otaUpdate(data, len);
                break;
            default:
                break;
        }
    } else if(event == event_network_status) {
        switch(param){
            case ep_network_status_disconnected:  //模组已断开路由器
                log_info("event network disconnect router\r\n");
                break;
            case ep_network_status_connected:     //模组已连接路由器
                log_info("event network connect router\r\n");
                break;
            case ep_cloud_status_disconnected:  //模组已断开平台
                log_info("event cloud disconnect server\r\n");
                break;
            case ep_cloud_status_connected:     //模组已连接平台
                log_info("event cloud connect server\r\n");
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

    //添加数据点定义
    Cloud.defineDatapointNumber(DPID_NUMBER_SOIL_HUMIDITY, DP_PERMISSION_UP_ONLY, 0, 100, 1, 0); //土壤湿度
    Cloud.defineDatapointNumber(DPID_NUMBER_AIR_HUMIDITY, DP_PERMISSION_UP_ONLY, 0, 100, 1, 0); //空气湿度
    Cloud.defineDatapointNumber(DPID_NUMBER_TEMPERATURE, DP_PERMISSION_UP_ONLY, -50, 50, 1, 0); //温度
    Cloud.defineDatapointNumber(DPID_NUMBER_ILLUMINATION, DP_PERMISSION_UP_DOWN, 0, 100, 1, 0); //光照强度
    Cloud.defineDatapointNumber(DPID_NUMBER_CO2, DP_PERMISSION_UP_ONLY, 0, 100, 0, 0); //二氧化碳浓度
    Cloud.defineDatapointEnum(DPID_ENUM_BIRDS, DP_PERMISSION_UP_ONLY, 0); //鸟类危害程度
    Cloud.defineDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, DP_PERMISSION_UP_DOWN, false); //洒水器开关

    Cloud.connect();
}

void userHandle(void)
{
    if(Cloud.connected()) {
        //处理需要上送到云平台的数据
        dpDoubleSoil_humidity += 0.1;
        dpDoubleAir_humidity += 0.1;
        dpDoubleTemperature += 0.1;
        dpDoubleIllumination += 0.1;
        dpIntCO2 += 1;

        Cloud.writeDatapointNumberDouble(DPID_NUMBER_SOIL_HUMIDITY, dpDoubleSoil_humidity);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_AIR_HUMIDITY, dpDoubleAir_humidity);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_TEMPERATURE, dpDoubleTemperature);
        Cloud.writeDatapointNumberDouble(DPID_NUMBER_ILLUMINATION, dpDoubleIllumination);
        Cloud.writeDatapointNumberInt32(DPID_NUMBER_CO2, dpIntCO2);
        Cloud.writeDatapointEnum(DPID_ENUM_BIRDS, dpEnumBirds);
        Cloud.writeDatapointBool(DPID_BOOL_SPRINKLER_SWITCH, dpBoolSprinkler_switch);
        delay(20000);
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

