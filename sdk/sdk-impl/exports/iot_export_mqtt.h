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


#ifndef __IOT_EXPORT_MQTT_H__
#define __IOT_EXPORT_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_mqtt_api.h"

typedef struct
{
    void *(*construct)(iotx_mqtt_param_t *pInitParams);
    int (*destroy)(void **phandle);
    int (*yield)(void *handle, int timeout_ms);
    int (*checkStateNormal)(void *handle);
    int (*subscribe)(void *handle, const char *topic_filter, iotx_mqtt_qos_t qos, iotx_mqtt_event_handle_func_fpt topic_handle_func, void *pcontext);
    int (*unsubscribe)(void *handle, const char *topic_filter);
    int (*publish)(void *handle, const char *topic_name, iotx_mqtt_topic_info_pt topic_msg);
} iot_mqtt_client_if_t;

extern const iot_mqtt_client_if_t MQTTClient;

#ifdef __cplusplus
}
#endif

#endif

