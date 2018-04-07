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
#include "sdkconfig.h"
#include "lite-log.h"
#include "lite-utils.h"
#include "utils_timer.h"
#include "utils_cJSON.h"
#include "iotx_comm_if_api.h"
#include "iotx_system_api.h"
#include "iotx_datapoint_api.h"

static iotx_conn_info_t iotx_conn_info;

static int iotx_get_device_info(char *buf, uint16_t buflen)
{
    cJSON* root = NULL;
    char* json_out = NULL;
    uint16_t json_len = 0;
    iotx_device_info_pt pdev_info = iotx_device_info_get();

    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "productId", cJSON_CreateString(pdev_info->product_id));
    cJSON_AddItemToObject(root, "bc", cJSON_CreateNumber(1));
    cJSON_AddItemToObject(root, "swVer", cJSON_CreateString(pdev_info->software_version));
    cJSON_AddItemToObject(root, "hwVer", cJSON_CreateString(pdev_info->hardware_version));
    cJSON_AddItemToObject(root, "online", cJSON_CreateBool(true));
    json_out = cJSON_Print(root);
    json_len = strlen(json_out);
    log_debug("info = %s", json_out);

    if(json_len > buflen) {
        log_err("memory len to short");
    }
    memset(buf, 0, buflen);
    memcpy(buf, json_out, json_len);
    HAL_Free(json_out);
    cJSON_Delete(root);
    return json_len;
}

iotx_conn_info_pt iotx_conn_info_get(void)
{
    return &iotx_conn_info;
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

    HAL_MutexLock(pconn_info->lock_generic);
    pconn_info->conn_state = newState;
    HAL_MutexUnlock(pconn_info->lock_generic);
}

#if CONFIG_CLOUD_CHANNEL == 1     //MQTT
#include "iotx_comm_mqtt.c"
#elif CONFIG_CLOUD_CHANNEL == 2   //COAP
#include "iotx_comm_coap.c"
#else
#error "NOT support yet!"
#endif

int IOT_Comm_Init(void)
{
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

    iotx_time_init(&iotx_conn_info.reconnect_param.reconnect_next_time);

    iotx_conn_info.lock_generic = HAL_MutexCreate();

    // 添加默认数据点
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_RESET, DP_PERMISSION_UP_DOWN, false);               //reboot
    IOT_DataPoint_DefineBool(DPID_DEFAULT_BOOL_GETALLDATAPOINT, DP_PERMISSION_UP_DOWN, false);     //get all datapoint

    log_info("conn_info created successfully!");
    return 0;
}

int IOT_Comm_Connect(void)
{
    if(IOTX_CONN_STATE_CONNECTED == iotx_get_conn_state()) {
        return true;
    }

    int rst = iotx_comm_connect();
    if(rst < 0) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    } else {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
    }

    return rst;
}

bool IOT_Comm_IsConnected(void)
{
    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return false;
    }

    bool rst = iotx_comm_isconnected();
    if(rst) {
        iotx_set_conn_state(IOTX_CONN_STATE_CONNECTED);
    } else {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }
    return rst;
}

int IOT_Comm_Disconnect(void)
{
    iotx_set_conn_state(IOTX_CONN_STATE_INITIALIZED);
    return iotx_comm_disconnect();
}

int IOT_Comm_SendData(const uint8_t *data, uint16_t datalen)
{
    log_debug("IOT_Comm_SendData");
    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return -1;
    }

    int rst = iotx_comm_senddata(data, datalen);
    if(rst < 0) {
        iotx_set_conn_state(IOTX_CONN_STATE_DISCONNECTED);
    }
    return rst;
}

int IOT_Comm_ReportProgress(uint8_t type, iotx_ota_reply_t reply, uint8_t progress)
{
    log_debug("IOT_Comm_SendData");
    if(IOTX_CONN_STATE_CONNECTED != iotx_get_conn_state()) {
        return -1;
    }

    int rst = iotx_comm_reportprogress(type, reply, progress);
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

    log_info("start reconnect");

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
            pconn_info->reconnect_param.reconnect_time_interval_ms = IOTX_MC_RECONNECT_INTERVAL_MAX_MS;
        }
    }

    interval_ms = pconn_info->reconnect_param.reconnect_time_interval_ms;
    interval_ms += HAL_Random(pconn_info->reconnect_param.reconnect_time_interval_ms);
    if (IOTX_CONN_RECONNECT_INTERVAL_MAX_MS < interval_ms) {
        interval_ms = IOTX_CONN_RECONNECT_INTERVAL_MAX_MS;
    }
    utils_time_countdown_ms(&(pconn_info->reconnect_param.reconnect_next_time), interval_ms);

    log_err("reconnect failed rc = %d", rc);
    return rc;
}

int IOT_Comm_Yield(void)
{
    int rc = 0;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    iotx_comm_yield();

    if(IOT_Comm_IsConnected()) {
        intoyunSendDatapointAutomatic();
        return 0;
    }

    iotx_conn_state_t connState = iotx_get_conn_state();
    do {
        /* if Exceeds the maximum delay time, then return reconnect timeout */
        if (IOTX_CONN_STATE_DISCONNECTED_RECONNECTING == connState) {
            rc = iotx_conn_handle_reconnect();
            if (SUCCESS_RETURN != rc) {
                //log_debug("reconnect network fail, rc = %d", rc);
            } else {
                log_info("network is reconnected!");
                //iotx_mc_reconnect_callback();
                pconn_info->reconnect_param.reconnect_time_interval_ms = IOTX_MC_RECONNECT_INTERVAL_MIN_MS;
            }

            break;
        }

        /* If network suddenly interrupted, stop pinging packet, try to reconnect network immediately */
        if (IOTX_CONN_STATE_DISCONNECTED == connState) {
            log_err("network is disconnected!");
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

