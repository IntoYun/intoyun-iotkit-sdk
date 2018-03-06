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

#ifndef __IOTX_COMM_API_H__
#define __IOTX_COMM_API_H__

#include "utils_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define INTOYUN_MQTT_SERVER_DOMAIN        "iot.intoyun.com"
#define INTOYUN_MQTT_SERVER_PORT          1883

#define INTOYUN_COAP_SERVER_DOMAIN        "iot.intoyun.com"
#define INTOYUN_COAP_SERVER_PORT          1883

#define INTOYUN_HTTP_SERVER_DOMAIN        "www.intoyun.com"
#define INTOYUN_HTTP_SERVER_PORT          80

/* Minimum interval of MQTT reconnect in millisecond */
#define IOTX_CONN_RECONNECT_INTERVAL_MIN_MS       (1000)

/* Maximum interval of MQTT reconnect in millisecond */
#define IOTX_CONN_RECONNECT_INTERVAL_MAX_MS       (60000)


#define PASSWORD_LEN                      (64)
#define RANDOM_STR_LEN                    (16)
#define APPSKEY_LEN                       (16)   /* appskey 密钥长度 */
#define NWKSKEY_LEN                       (16)   /* nwkskey 密钥长度 */
#define TOPIC_NAME_LEN                    (64)

/* State of MQTT client */
typedef enum {
    IOTX_CONN_STATE_INVALID = 0,                    /* MQTT in invalid state */
    IOTX_CONN_STATE_INITIALIZED = 1,                /* MQTT in initializing state */
    IOTX_CONN_STATE_CONNECTED = 2,                  /* MQTT in connected state */
    IOTX_CONN_STATE_DISCONNECTED = 3,               /* MQTT in disconnected state */
    IOTX_CONN_STATE_DISCONNECTED_RECONNECTING = 4,  /* MQTT in reconnecting state */
} iotx_conn_state_t;

/* Reconnected parameter*/
typedef struct {
    iotx_time_t     reconnect_next_time;         /* the next time point of reconnect */
    uint32_t        reconnect_time_interval_ms;  /* time interval of this reconnect */
} iotx_conn_reconnect_param_t;

typedef struct {
    uint16_t        port;
    char            *host_name;
    char            *client_id;
    char            *username;
    char            password[PASSWORD_LEN + 1];
    const char      *pub_key;
    char            random_str[RANDOM_STR_LEN];
    uint8_t         appskey[APPSKEY_LEN];
    uint8_t         nwkskey[NWKSKEY_LEN];
    uint16_t        up_packetid;
    uint16_t        down_packetid;
    void            *pclient;
    char            topic_name_tx[TOPIC_NAME_LEN];
    char            topic_name_action[TOPIC_NAME_LEN];

    void            *lock_generic; /* generic lock */
    iotx_conn_state_t conn_state;
    iotx_conn_reconnect_param_t reconnect_param;
} iotx_conn_info_t, *iotx_conn_info_pt;

iotx_conn_info_pt iotx_conn_info_get(void);

int IOT_Comm_Connect(void);

bool IOT_Comm_IsConnected(void);

int IOT_Comm_Disconnect(void);

int IOT_Comm_SendData(const uint8_t *data, uint16_t dataLen);

int IOT_Comm_Yield(void);

#ifdef __cplusplus
}
#endif

#endif

