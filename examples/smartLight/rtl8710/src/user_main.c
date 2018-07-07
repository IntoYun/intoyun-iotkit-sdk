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
#include "device.h"
#include "wifi_constants.h"
#include "lwip_netconf.h"

//#define CONFIG_WIFI_SSID      "TP-LINK_3816"
#define CONFIG_WIFI_SSID      "MOLMC_HUAWEI"
#define CONFIG_WIFI_PASSWORD  "26554422"

extern int userMain(void);

static void on_wifi_no_network(char* buf, int buf_len, int flags, void* userdata)
{
    printf("on_wifi_no_network\n");
    Network.setState(IOTX_NETWORK_STATE_DISCONNECTED);

}

static void on_wifi_connect( char* buf, int buf_len, int flags, void* userdata)
{
    printf("on_wifi_connect\n");
    Network.setState(IOTX_NETWORK_STATE_CONNECTED);
}

static void on_wifi_disconnect( char* buf, int buf_len, int flags, void* userdata)
{
    printf("disconnect\n");
    Network.setState(IOTX_NETWORK_STATE_DISCONNECTED);
}

static void intoyun_iot_task(void *param)
{
    wlan_network();     

    vTaskDelay(2000);	

    wifi_reg_event_handler(WIFI_EVENT_NO_NETWORK, on_wifi_no_network, NULL);
	wifi_reg_event_handler(WIFI_EVENT_CONNECT, on_wifi_connect, NULL);
	wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, on_wifi_disconnect, NULL);
   
    if(wifi_connect(CONFIG_WIFI_SSID, RTW_SECURITY_WPA2_AES_PSK, \
            CONFIG_WIFI_PASSWORD, strlen(CONFIG_WIFI_SSID), strlen(CONFIG_WIFI_PASSWORD), -1, NULL) == RTW_SUCCESS) {
        LwIP_DHCP(0, DHCP_START);
    }

    userMain();
}

int main(void)
{
/*
	ReRegisterPlatformLogUart();
    wlan_network();   
*/

    Log.setLogLevel("*", MOLMC_LOG_VERBOSE);
    Log.setLogLevel("user:project", MOLMC_LOG_VERBOSE);
    Log.setLogLevel("user:ota", MOLMC_LOG_VERBOSE);

	if(xTaskCreate(intoyun_iot_task, (char const *)"intoyun_iot_task", 4096 * 2, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS){
		printf("\n\r[%s] Create update task failed", __FUNCTION__);
	}

	vTaskStartScheduler();
}

