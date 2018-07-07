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
#include "device.h"
#include "gpio_api.h"   // mbed
#include "analogin_api.h"

const static char *TAG = "user:project";

#define DEVICE_ID_DEF                   "0dvo0bdoy00000000000068f"         //设备标识
#define DEVICE_SECRET_DEF               "c08e66a8b08fd8436dac0dce9cc3bca9" //设备密钥


#define DPID_BOOL_SWITCH                1  //布尔型            开关
#define DPID_DOUBLE_ILLUMINATION        2  //数值型            光照强度

#define GPIO_LED_PIN                    PA_0

bool dpBoolSwitch;                      // 开关
double dpDoubleIllumination = 100;      // 光照强度

uint32_t timerID;
gpio_t gpio_led;

void eventProcess(int event, int param, uint8_t *data, uint32_t datalen)
{
    if(event == event_cloud_comm) {
        switch(param) {
            case ep_cloud_comm_data:
                if (RESULT_DATAPOINT_NEW == Cloud.readDatapointBool(DPID_BOOL_SWITCH, &dpBoolSwitch)) {
                    MOLMC_LOGI(TAG, "dpBoolSwitch = %d", dpBoolSwitch);
                    if(dpBoolSwitch) {
                        gpio_write(&gpio_led, 0);
                    } else {
                        gpio_write(&gpio_led, 1);
                    }
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

    //添加数据点定义
    Cloud.defineDatapointBool(DPID_BOOL_SWITCH, DP_PERMISSION_UP_DOWN, false); //灯开关
    Cloud.defineDatapointNumber(DPID_DOUBLE_ILLUMINATION, DP_PERMISSION_UP_ONLY, 0, 10000, 1, 0); //光照强度

    /*************此处修改和添加用户初始化代码**************/
    Cloud.connect();
    timerID = timerGetId();

    //初始化led管脚
    gpio_init(&gpio_led, GPIO_LED_PIN);
    gpio_dir(&gpio_led, PIN_OUTPUT);    // Direction: Output
    gpio_mode(&gpio_led, PullNone);     // No pull
    gpio_write(&gpio_led, 1);           
    /*******************************************************/
}

/* Vbat channel */
#define OFFSET		0x492							
double getLightSensor(void)
{
	uint16_t adc_read = 0;
	analogin_t   adc_vbat;

	analogin_init(&adc_vbat, AD_2);
    adc_read = (analogin_read_u16(&adc_vbat) >> 4) - OFFSET ;
	analogin_deinit(&adc_vbat);

    MOLMC_LOGI(TAG, "adc_read = %d\n", adc_read); 
    return adc_read;
}

void userHandle(void)
{
    if(Cloud.connected()) {
        if(timerIsEnd(timerID, 10000)) {
            timerID = timerGetId();
            
            dpDoubleIllumination = getLightSensor();
            Cloud.writeDatapointNumberDouble(DPID_DOUBLE_ILLUMINATION, dpDoubleIllumination);
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
}

