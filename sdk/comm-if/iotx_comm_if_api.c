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
#include "sdk_config.h"
#include "iotx_comm_if_api.h"
#include "iotx_system_api.h"
#include "iotx_datapoint_api.h"
#include "iotx_comm_if.h"

const static char *TAG = "sdk:comm-if";

static int iotx_conninfo_inited = 0;
static iotx_conn_info_t iotx_conn_info;

int iotx_get_device_info(char *buf, uint16_t buflen)
{
    cJSON* root = NULL;
    char* json_out = NULL;
    uint16_t json_len = 0;
    iotx_device_info_pt pdev_info = iotx_device_info_get();

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "productId", cJSON_CreateString(pdev_info->product_id));
    cJSON_AddItemToObject(root, "bc", cJSON_CreateNumber(1));  //暂时去掉， esp8266 JSON处理有问题
    cJSON_AddItemToObject(root, "swVer", cJSON_CreateString(pdev_info->software_version));
    cJSON_AddItemToObject(root, "hwVer", cJSON_CreateString(pdev_info->hardware_version));
    cJSON_AddItemToObject(root, "online", cJSON_CreateBool(true));
    json_out = cJSON_Print(root);
    json_len = strlen(json_out);
    MOLMC_LOGD(TAG, "info = %s", json_out);

    if(json_len >= buflen) {
        MOLMC_LOGE(TAG, "memory len to short");
        return -1;
    }
    memset(buf, 0, buflen);
    memcpy(buf, json_out, json_len);
    HAL_Free(json_out);
    cJSON_Delete(root);
    return json_len;
}

static int iotx_get_action_reply(char *buf, uint16_t buflen, uint8_t fileType, iotx_ota_reply_t reply, uint8_t progress)
{
    char temp[128] = {0};

    switch(reply) {
        case IOTX_OTA_REPLY_PROGRESS:           /* 下载中 */
            snprintf(temp, sizeof(temp), "{\"type\":\"%d\",\"status\":\"10\",\"progress\":\"%d\"}", fileType, progress);
            break;
        case IOTX_OTA_REPLY_FETCH_FAILED:       /* 获取文件失败 */
            snprintf(temp, sizeof(temp), "{\"type\":\"%d\",\"status\":\"11\"}", fileType);
            break;
        case IOTX_OTA_REPLY_FETCH_SUCCESS:      /* 获取文件成功 */
            snprintf(temp, sizeof(temp), "{\"type\":\"%d\",\"status\":\"15\"}", fileType);
            break;
        case IOTX_OTA_REPLY_BURN_FAILED:        /* 烧录程序失败 */
            snprintf(temp, sizeof(temp), "{\"type\":\"%d\",\"status\":\"13\"}", fileType);
            break;
        case IOTX_OTA_REPLY_BURN_SUCCESS:       /* 烧录程序成功 */
            snprintf(temp, sizeof(temp), "{\"type\":\"%d\",\"status\":\"14\"}", fileType);
            break;
        default:
            MOLMC_LOGE(TAG, "reply type error");
            return -1;
            break;
    }
    MOLMC_LOGD(TAG, "reply = %s", temp);

    uint16_t templen = strlen(temp);
    if(templen >= buflen) {
        MOLMC_LOGE(TAG, "memory len to short");
        return -1;
    }
    memset(buf, 0, buflen);
    memcpy(buf, temp, templen);
    return templen;
}

iotx_conn_info_pt iotx_conn_info_get(void)
{
    IOT_Comm_Init();
    return &iotx_conn_info;
}

/* get state of network */
static iotx_network_state_t iotx_get_network_state(void)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_network_state_t state;

    HAL_MutexLock(pconn_info->lock_generic);
    state = pconn_info->network_state;
    HAL_MutexUnlock(pconn_info->lock_generic);

    return state;
}

/* set state of network */
static void iotx_set_network_state(iotx_network_state_t newState)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if(pconn_info->network_state != newState) {
        switch(newState) {
            case IOTX_NETWORK_STATE_DISCONNECTED:   /* disconnected state */
                IOT_SYSTEM_NotifyEvent(event_network_status, ep_network_status_disconnected, NULL, 0);
                break;
            case IOTX_NETWORK_STATE_CONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_network_status, ep_network_status_connected, NULL, 0);
                break;
            default:
                break;
        }
    }

    HAL_MutexLock(pconn_info->lock_generic);
    pconn_info->network_state = newState;
    HAL_MutexUnlock(pconn_info->lock_generic);
}

/* get state of conn */
static iotx_conn_state_t iotx_get_conn_state(void)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_conn_state_t state;

    HAL_MutexLock(pconn_info->lock_generic);
    state = pconn_info->conn_state;
    HAL_MutexUnlock(pconn_info->lock_generic);

    return state;
}

/* set state of conn */
static void iotx_set_conn_state(iotx_conn_state_t newState)
{
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if(pconn_info->conn_state != newState) {
        switch(newState) {
            case IOTX_CONN_STATE_INITIALIZED:    /* initializing state */
            case IOTX_CONN_STATE_DISCONNECTED:   /* disconnected state */
                if(pconn_info->conn_state == IOTX_CONN_STATE_CONNECTED) {
                    IOT_SYSTEM_NotifyEvent(event_cloud_status, ep_cloud_status_disconnected, NULL, 0);
                }
                break;
            case IOTX_CONN_STATE_CONNECTED:      /* connected state */
                IOT_SYSTEM_NotifyEvent(event_cloud_status, ep_cloud_status_connected, NULL, 0);
                break;
            default:
                break;
        }
    }

    HAL_MutexLock(pconn_info->lock_generic);
    pconn_info->conn_state = newState;
    HAL_MutexUnlock(pconn_info->lock_generic);
}

bool IOT_Network_IsConnected(void)
{
    if(IOTX_NETWORK_STATE_CONNECTED != iotx_get_network_state()) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
        return false;
    }
    return true;
}

void IOT_Network_SetState(iotx_network_state_t state)
{
    iotx_set_network_state(state);
}

int IOT_Comm_Init(void)
{
    if (iotx_conninfo_inited) {
        return 0;
    }

    iotx_device_info_pt pdev_info = iotx_device_info_get();
    memset(&iotx_conn_info, 0x0, sizeof(iotx_conn_info_t));

#if CONFIG_CLOUD_CHANNEL == 1     //MQTT
    iotx_conn_info.port = INTOYUN_MQTT_SERVER_PORT;
    iotx_conn_info.host_name = INTOYUN_MQTT_SERVER_DOMAIN;
#elif CONFIG_CLOUD_CHANNEL == 2   //COAP
    iotx_conn_info.port = INTOYUN_COAP_SERVER_PORT;
    iotx_conn_info.host_name = INTOYUN_COAP_SERVER_DOMAIN;
#else
#error "NOT support yet!"
#endif
    iotx_conn_info.client_id = pdev_info->device_id;
    iotx_conn_info.username = pdev_info->device_id;
    iotx_conn_info.conn_state = IOTX_CONN_STATE_INVALID;
    iotx_conn_info.network_state = IOTX_NETWORK_STATE_DISCONNECTED;
    iotx_time_init(&iotx_conn_info.reconnect_param.reconnect_next_time);
    iotx_conn_info.lock_generic = HAL_MutexCreate();

#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    // 添加默认数据点
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_RESET, DP_PERMISSION_UP_DOWN, false);               //reboot
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_GETALLDATAPOINT, DP_PERMISSION_UP_DOWN, false);     //get all datapoint
#endif

    iotx_conninfo_inited = 1;
    return 0;
}

void IOT_Comm_Connect(void)
{
    if(!IOT_Network_IsConnected()) {
        return;
    }

    if(IOTX_CONN_STATE_CONNECTED == iotx_get_conn_state()) {
        return;
    }

    MOLMC_LOGI(TAG, "iotx_comm_connect");
    int rst = iotx_comm_connect();
    if(rst < 0) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    } else {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
    }
}

bool IOT_Comm_IsConnected(void)
{
    if(!IOT_Network_IsConnected()) {
        return false;
    }

    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return false;
    }

    //MOLMC_LOGI(TAG, "IOT_Comm_IsConnected");
    bool rst = iotx_comm_isconnected();
    if(rst) {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
    } else {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }
    return rst;
}

void IOT_Comm_Disconnect(void)
{
    MOLMC_LOGI(TAG, "IOT_Comm_Disconnect");
    iotx_set_conn_state(IOTX_CONN_STATE_INITIALIZED);
    iotx_comm_disconnect();
}

int IOT_Comm_SendData(const uint8_t *data, uint16_t datalen)
{
    MOLMC_LOGD(TAG, "IOT_Comm_SendData");
    if(!IOT_Network_IsConnected()) {
        return -1;
    }

    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return -1;
    }

    int rst = iotx_comm_send(IOTX_CONN_SEND_DATA, data, datalen);
    if(rst < 0) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }
    return rst;
}

int IOT_Comm_SendActionReply(uint8_t fileType, iotx_ota_reply_t reply, uint8_t progress)
{
    char temp[128] = {0};

    MOLMC_LOGD(TAG, "IOT_Comm_SendActionReply");
    if(!IOT_Network_IsConnected()) {
        return -1;
    }

    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return -1;
    }

    int templen = iotx_get_action_reply(temp, sizeof(temp), fileType, reply, progress);
    if(templen <= 0) {
        return -1;
    }

    int rst = iotx_comm_send(IOTX_CONN_SEND_ACTION_REPLY, (uint8_t *)temp, templen);
    if(rst < 0) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }

    return rst;
}

/* reconnect */
static int iotx_conn_handle_reconnect(void)
{
    int             rc = FAIL_RETURN;
    uint32_t        interval_ms = 0;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if (!utils_time_is_expired(&(pconn_info->reconnect_param.reconnect_next_time))) {
        /* Timer has not expired. Not time to attempt reconnect yet. Return attempting reconnect */
        return FAIL_RETURN;
    }

    MOLMC_LOGI(TAG, "start reconnect");

    rc = iotx_comm_connect();
    if (SUCCESS_RETURN == rc) {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
        return SUCCESS_RETURN;
    } else {
        /* if reconnect network failed, then increase currentReconnectWaitInterval */
        /* e.g. init currentReconnectWaitInterval=1s, reconnect failed, then 2s..4s..8s */
        if (IOTX_CONN_RECONNECT_INTERVAL_MAX_MS > pconn_info->reconnect_param.reconnect_time_interval_ms) {
            pconn_info->reconnect_param.reconnect_time_interval_ms *= 2;
        } else {
            pconn_info->reconnect_param.reconnect_time_interval_ms = IOTX_CONN_RECONNECT_INTERVAL_MIN_MS;
        }
    }

    interval_ms = pconn_info->reconnect_param.reconnect_time_interval_ms;
    interval_ms += HAL_Random(pconn_info->reconnect_param.reconnect_time_interval_ms);
    if (IOTX_CONN_RECONNECT_INTERVAL_MAX_MS < interval_ms) {
        interval_ms = IOTX_CONN_RECONNECT_INTERVAL_MAX_MS;
    }
    utils_time_countdown_ms(&(pconn_info->reconnect_param.reconnect_next_time), interval_ms);

    MOLMC_LOGE(TAG, "reconnect failed rc = %d", rc);
    return rc;
}

int IOT_Comm_Yield(void)
{
    int rc = 0;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if(!IOT_Network_IsConnected()) {
        return -1;
    }

    iotx_comm_yield();

#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    if(IOT_Comm_IsConnected()) {
        IOT_DataPoint_SendDatapointAutomatic();
        return 0;
    }
#endif

    iotx_conn_state_t connState = iotx_get_conn_state();
    do {
        /* if Exceeds the maximum delay time, then return reconnect timeout */
        if (IOTX_CONN_STATE_DISCONNECTED_RECONNECTING == connState) {
            rc = iotx_conn_handle_reconnect();
            if (SUCCESS_RETURN != rc) {
                //MOLMC_LOGD(TAG, "reconnect network fail, rc = %d", rc);
            } else {
                MOLMC_LOGI(TAG, "comm is reconnected!");
                //iotx_mc_reconnect_callback();
                pconn_info->reconnect_param.reconnect_time_interval_ms = IOTX_CONN_RECONNECT_INTERVAL_MAX_MS;
            }

            break;
        }

        /* If network suddenly interrupted, stop pinging packet, try to reconnect network immediately */
        if (IOTX_CONN_STATE_DISCONNECTED == connState) {
            MOLMC_LOGE(TAG, "comm is disconnected!");
            //iotx_mc_disconnect_callback(pClient);

            pconn_info->reconnect_param.reconnect_time_interval_ms = IOTX_CONN_RECONNECT_INTERVAL_MIN_MS;
            utils_time_countdown_ms(&(pconn_info->reconnect_param.reconnect_next_time),
                                    pconn_info->reconnect_param.reconnect_time_interval_ms);

            iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED_RECONNECTING);
            break;
        }

    } while (0);

    return 0;
}

