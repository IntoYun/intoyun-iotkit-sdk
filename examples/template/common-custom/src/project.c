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

void eventProcess(int event, int param, uint8_t *data, uint32_t len)
{
    if(event == event_cloud_comm) {
        switch(param){
            case ep_cloud_comm_data:
                //处理用户控制数据
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
            default:
                break;
        }
    } else if(event == event_cloud_status) {
        switch(param){
            case ep_cloud_status_disconnected:    //模组已断开平台
                log_info("event cloud disconnect server\r\n");
                break;
            case ep_cloud_status_connected:       //模组已连接平台
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
    Cloud.connect();
}

void userHandle(void)
{
    if(Cloud.connected()) {
        //处理需要上送到云平台的数据
        Cloud.sendCustomData(NULL, 0);
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

