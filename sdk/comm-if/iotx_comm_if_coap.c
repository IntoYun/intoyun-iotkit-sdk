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
#include "iot_import_coap.h"
#include "iotx_coap_client.h"
#include "iotx_guider_api.h"
#include "iotx_crypto_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if.h"
#include "iotx_datapoint_api.h"

#if CONFIG_CLOUD_CHANNEL == 2     //COAP

const static char *TAG = "sdk:comm-if-coap";

#define SUB_TX_TOPIC                  "tx"
#define PUB_RX_TOPIC                  "rx"
#define PUB_INFO_TOPIC                "info"
#define SUB_ACTION_TOPIC              "action"
#define KEEP_ALIVE_PING               "ping"
#define IOTX_AUTH_STR                 "auth"

static int send_mata_info(iotx_coap_context_t *coap_context);
static int send_device_auth(iotx_coap_context_t *coap_context, char re_auth);
static int sub_action_topic(iotx_coap_context_t *coap_context);
static int sub_data_topic(iotx_coap_context_t *coap_context);
static void parse_response_code_log(uint8_t code);
extern int iotx_get_device_info(char *buf, uint16_t buflen);

/**
 * 获取aes access token
 * */
static int iotx_coap_pre_auth_process(void)
{
    return iotx_guider_authenticate();
}

static int iotx_coap_post_auth_process(void)
{
    return iotx_guider_auth_gen_keys();
}

/**
 * 转换返回码为coap格式码
 * */
static void parse_response_code_log(uint8_t code)
{
    uint8_t x = (code & 0xe0) >> 5;
    uint8_t xx = code & 0x1f;
    MOLMC_LOGI(TAG, "Response Code : %d.%02d", x, xx);
}

/**
 * 设备认证请求回调
 * */
static void iotx_device_auth_callback(void *pcontext, void *p_message)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPMessage *message = (CoAPMessage *)p_message;

    p_iotx_coap = (iotx_coap_t *)pcontext;

    // heartbeat countdown time
    utils_time_countdown_ms(&(p_iotx_coap->heartbeat_timer), IOTX_COAP_PING_INTERVAL_S);

    // 计数设备auth计时，授权token过期时间为48小时
    utils_time_countdown_ms(&(p_iotx_coap->auth_timer), IOTX_COAP_AUTH_INTERVAL_S);

    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p",  message);
        return;
    }

    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, "Invalid paramter, pcontext %p",  pcontext);
        return;
    }

    parse_response_code_log(message->header.code);
    if(NULL != message->payload) {
        MOLMC_LOGD(TAG, "Receive auth response message: %s", message->payload);
    }

    switch (message->header.code) {
        case COAP_MSG_CODE_205_CONTENT:
            {
                iotx_coap_post_auth_process();

                strncpy(p_iotx_coap->p_auth_token, (char *)message->payload, p_iotx_coap->auth_token_len);
                p_iotx_coap->is_authed = IOT_TRUE;
                MOLMC_LOGI(TAG, "CoAP authenticate success token: %s", p_iotx_coap->p_auth_token);

                /**订阅action topic*/
                sub_action_topic((iotx_coap_context_t *) pcontext);

                /**订阅数据tx topic*/
                sub_data_topic((iotx_coap_context_t *) pcontext);

                /**发送设备mata信息到服务器*/
                send_mata_info((iotx_coap_context_t *) pcontext);
                break;
            }
        case COAP_MSG_CODE_500_INTERNAL_SERVER_ERROR:
            {
                MOLMC_LOGI(TAG, "CoAP internal server error, authenticate failed, will retry it");
                send_device_auth((iotx_coap_context_t *) pcontext, IOT_FALSE);
                break;
            }
        default:
            break;
    }
}

/**
 * 上送设备meta信息请求回调
 * */
static void iotx_send_meta_info_callback(void *pcontext, void *p_message)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPMessage *message = (CoAPMessage *)p_message;

    p_iotx_coap = (iotx_coap_t *)pcontext;

    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p",  message);
        return;
    }

    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, "Invalid paramter, pcontext %p",  pcontext);
        return;
    }

    parse_response_code_log(message->header.code);
    if (NULL != message->payload) {
      MOLMC_LOGD(TAG, "Receive meta info response message: %s", message->payload);
    }

    switch (message->header.code) {
        case COAP_MSG_CODE_205_CONTENT:
            {
                break;
            }
        case COAP_MSG_CODE_500_INTERNAL_SERVER_ERROR:
            {
                break;
            }
        default:
            break;
    }
}

/**
 * observe 订阅数据tx topic回调
 * */
static void cloud_data_receive_callback(void *pcontext, void *p_message)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPMessage *message = (CoAPMessage *)p_message;

    p_iotx_coap = (iotx_coap_t *)pcontext;

    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p",  message);
        return;
    }

    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, "Invalid paramter, pcontext %p",  pcontext);
        return;
    }

    MOLMC_LOGD(TAG, "cloud_data_receive_callback");
    parse_response_code_log(message->header.code);
    MOLMC_LOGD(TAG, "* Payload: ");
    if (message->payload == NULL)
    {
        return;
    }
    MOLMC_LOG_BUFFER_HEX(TAG, message->payload, message->payloadlen);

    switch (message->header.code) {
        case COAP_MSG_CODE_205_CONTENT:
            {
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
                IOT_DataPoint_ParseReceiveDatapoints(message->payload, message->payloadlen);
#endif
                IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_data, (uint8_t *)message->payload, message->payloadlen);
                break;
            }
        case COAP_MSG_CODE_500_INTERNAL_SERVER_ERROR:
            {
                break;
            }
        default:
            break;
    }
}

/**
 * observe 订阅控制action topic回调
 * */
static void cloud_action_callback(void *pcontext, void *p_message)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPMessage *message = (CoAPMessage *)p_message;

    p_iotx_coap = (iotx_coap_t *)pcontext;

    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p",  message);
        return;
    }

    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, "Invalid paramter, pcontext %p",  pcontext);
        return;
    }

    MOLMC_LOGD(TAG, "cloud_action_callback");
    parse_response_code_log(message->header.code);
    MOLMC_LOGD(TAG, "* Payload: ");
    if (message->payload == NULL)
    {
      return;
    }
    MOLMC_LOG_BUFFER_HEX(TAG, message->payload, message->payloadlen);
    switch (message->header.code) {
        case COAP_MSG_CODE_205_CONTENT:
            {
                IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_ota, (uint8_t *)message->payload, message->payloadlen);
                break;
            }
        case COAP_MSG_CODE_500_INTERNAL_SERVER_ERROR:
            {
                break;
            }
        default:
            break;
    }
}

/**
 * 发送数据请求回到
 * */
static void iotx_publish_callback(void *pcontext, void *p_message)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPMessage *message = (CoAPMessage *)p_message;

    p_iotx_coap = (iotx_coap_t *)pcontext;

    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p",  message);
        return;
    }

    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, "Invalid paramter, pcontext %p",  pcontext);
        return;
    }

    MOLMC_LOGD(TAG, "iotx_publish_callback");
    parse_response_code_log(message->header.code);
    if(message->payload != NULL) {
        MOLMC_LOGD(TAG, "* Payload: %s", message->payload);
    }
}

/**
 * 建立coap连接，初始化coap客户端
 * */
int iotx_comm_connect(void)
{
    int rc = 0;
    void *pclient = NULL;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_coap_config_t *coap_config = NULL;

    iotx_device_info_pt pdev_info = iotx_device_info_get();

    coap_config = (iotx_coap_config_t *)HAL_Malloc(sizeof(iotx_coap_config_t));
    if(NULL == coap_config) {
        MOLMC_LOGE(TAG, "Allocate memory for coap_config failed");
        goto do_exit;
    }
    memset(coap_config, 0x00, sizeof(iotx_coap_config_t));
    coap_config->p_host = pconn_info->host_name;
    coap_config->p_port = pconn_info->port;
    coap_config->wait_time_ms = 400;
    coap_config->event_handle = NULL;
    coap_config->p_devinfo = HAL_Malloc(sizeof(iotx_deviceinfo_t));
    memset(coap_config->p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    memcpy(coap_config->p_devinfo->device_id, pdev_info->device_id, DEVICE_ID_LEN);
    memcpy(coap_config->p_devinfo->device_secret, pdev_info->device_secret, IOTX_DEVICE_SECRET_LEN);

    pclient = IOT_CoAP_Init(coap_config);
    if(NULL == pclient) {
        MOLMC_LOGE(TAG, "IOT_CoAP_Init() failed");
        goto do_exit;
    }

    pconn_info->pclient = pclient;

    rc = iotx_coap_pre_auth_process();
    if (rc < 0) {
        MOLMC_LOGE(TAG, "IOT_CoAP_Auth() failed, rc = %d", rc);
        goto do_exit;
    }
    send_device_auth(pclient, IOT_FALSE);
    return 0;

do_exit:
    if(NULL != coap_config) {
        if(NULL != coap_config->p_devinfo) {
            HAL_Free(coap_config->p_devinfo);
        }
        HAL_Free(coap_config);
    }

    if(NULL != pclient) {
        IOT_CoAP_Deinit(&pclient);
    }
    return -1;
}

/**
 * 发送设备认证请求
 * */
static int send_device_auth(iotx_coap_context_t *coap_context, char re_auth){
    iotx_coap_t *pclient = (iotx_coap_t *)coap_context;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    if(re_auth == IOT_TRUE && 0 == utils_time_is_expired(&(pclient->auth_timer))){
        return IOTX_SUCCESS;
    }
    // 每隔45个小时
    utils_time_countdown_ms(&(pclient->auth_timer), IOTX_COAP_AUTH_INTERVAL_S);

    char uri[128] = {0};
    snprintf(uri, sizeof(uri), "/v2/device/%s/auth", pdev_info->device_id);
    char query[128] = {0};
    snprintf(query, sizeof(query), "&p=%s", pconn_info->password);

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_GET;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = NULL;
    message.payload_len = 0;
    message.resp_callback = iotx_device_auth_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    return IOT_CoAP_Auth(pclient, &message);
}

/**
 * 发送设备meata信息请求
 * */
static int send_mata_info(iotx_coap_context_t *coap_context){
    iotx_coap_t *pclient = (iotx_coap_t *)coap_context;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;

    char uri[128] = {0};
    snprintf(uri, sizeof(uri), "/v2/device/%s/info", pdev_info->device_id);
    char query[128] = {0};
    snprintf(query, sizeof(query), "&srt=%s", pclient->p_auth_token);

    //发送info
    char msg_buf[256] = {0};
    if (iotx_get_device_info(msg_buf, sizeof(msg_buf)) < 0) {
        MOLMC_LOGE(TAG, "generate info failed");
        return -1;
    }

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_POST;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = (uint8_t *)msg_buf;
    message.payload_len = strlen(msg_buf);
    message.resp_callback = iotx_send_meta_info_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    return IOT_CoAP_Client_Send(pclient, &message);
}

/**
 * observe 订阅action topic
 * */
static int sub_action_topic(iotx_coap_context_t *coap_context){
    iotx_coap_t *pclient = (iotx_coap_t *)coap_context;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;

    char uri[128] = {0};
    snprintf(uri, sizeof(uri), "/v2/device/%s/action", pdev_info->device_id);
    char query[128] = {0};
    snprintf(query, sizeof(query), "&srt=%s", pclient->p_auth_token);

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_GET;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = NULL;
    message.payload_len = 0;
    message.resp_callback = cloud_action_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    return IOT_CoAP_Client_Observe_Send(pclient, &message);
}

/**
 * observe 订阅数据tx topic
 * */
static int sub_data_topic(iotx_coap_context_t *coap_context){
    iotx_coap_t *pclient = (iotx_coap_t *)coap_context;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;

    char uri[128] = {0};
    snprintf(uri, sizeof(uri), "/v2/device/%s/tx", pdev_info->device_id);
    char query[128] = {0};
    snprintf(query, sizeof(query), "&srt=%s", pclient->p_auth_token);

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_GET;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = NULL;
    message.payload_len = 0;
    message.resp_callback = cloud_data_receive_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    return IOT_CoAP_Client_Observe_Send(pclient, &message);
}

/**
 * 维持心跳
 *
 * */
static int connect_heartbeat(iotx_coap_context_t *coap_context){
    iotx_coap_t *pclient = (iotx_coap_t *)coap_context;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;

    // 1min间隔时间未到，即退出
    if(0 == utils_time_is_expired(&(pclient->heartbeat_timer))){
        return FAIL_RETURN;
    }
    // 每隔1min发送一个心跳包
    utils_time_countdown_ms(&(pclient->heartbeat_timer), IOTX_COAP_PING_INTERVAL_S);

    char uri[128] = {0};
    snprintf(uri, sizeof(uri), "/v2/device/%s/ping", pdev_info->device_id);
    char query[128] = {0};
    snprintf(query, sizeof(query), "&srt=%s", pclient->p_auth_token);

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_GET;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = NULL;
    message.payload_len = 0;
    message.resp_callback = NULL;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    return IOT_CoAP_Client_Send(pclient, &message);
}

/**
 * 获取网络连接状态
 * */
bool iotx_comm_isconnected(void)
{
    iotx_coap_t *pclient = (iotx_coap_t *)iotx_conn_info_get()->pclient;
    if(NULL == pclient){
        return false;
    }
    return pclient->is_connected == IOT_TRUE;
}

/**
 * 断开coap连接
 * */
int iotx_comm_disconnect(void)
{
    iotx_coap_context_t *pclient = (iotx_coap_context_t *)iotx_conn_info_get()->pclient;

    if(NULL != pclient) {
        IOT_CoAP_Deinit(pclient);
    }
    return 0;
}

/**
 * 发送数据请求
 * */
int iotx_comm_send(iotx_conn_send_t sendType, const uint8_t *data, uint16_t datalen)
{
    int ret = COAP_SUCCESS;
    MOLMC_LOGD(TAG, "send type: %d, data: %s", sendType,(char *)data );
    /** return 0; */

    iotx_coap_t *pclient = (iotx_coap_t *)iotx_conn_info_get()->pclient;
    iotx_deviceinfo_t *pdev_info = pclient->p_devinfo;

    char uri[128] = {0};
    if(sendType == IOTX_CONN_SEND_DATA) {
        snprintf(uri, sizeof(uri), "/v2/device/%s/rx", pdev_info->device_id);
    } else if(sendType == IOTX_CONN_SEND_ACTION_REPLY) {
        snprintf(uri, sizeof(uri), "/v2/device/%s/reply", pdev_info->device_id);
    }

    char query[128] = {0};
    snprintf(query, sizeof(query), "&srt=%s", pclient->p_auth_token);

    iotx_message_t message;
    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_POST;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = (uint8_t *)query;
    message.p_payload = (uint8_t *)data;
    message.payload_len = datalen;
    message.resp_callback = iotx_publish_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    ret = IOT_CoAP_Client_Send((iotx_coap_context_t *)pclient, &message);
    // 重新设置心跳间隔时间
    utils_time_countdown_ms(&(pclient->heartbeat_timer), IOTX_COAP_PING_INTERVAL_S);
    return ret;
}

int iotx_comm_yield(void)
{
    iotx_coap_context_t *pclient = (iotx_coap_context_t *)iotx_conn_info_get()->pclient;

    if(NULL != pclient) {
        IOT_CoAP_Yield(pclient);

        // 心跳连接
        connect_heartbeat(pclient);

        // 重新认证
        send_device_auth(pclient, IOT_TRUE);
    }

    return 0;
}
#endif

