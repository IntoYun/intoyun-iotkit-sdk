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

#include <stdlib.h>
#include "iot_import.h"
#include "iotx_comm_if_api.h"
#include "iotx_coap_client.h"
#include "iotx_system_api.h"

const static char *TAG = "sdk:epoch_time";

typedef struct {
    bool get_time_stamp_success;
    char time_stamp_str[16];
} time_stamp_t;

static time_stamp_t s_time_stamp;

/**
 * 通过http协议获取时间戳
 * */
uint64_t utils_http_get_epoch_time(char copy[], int len)
{
#define HTTP_RESP_CONTENT_LEN   (64)
    int ret;
    char http_content[HTTP_RESP_CONTENT_LEN];
    char url[64];
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;
    cJSON *tsJson = NULL;

    ret = httpclient_init(&httpclient);
    if (0 != ret) {
        goto do_exit;
    }

    httpclient.header = "Accept: text/xml,text/html\r\n";
    httpclient.header = "Connection: close\r\n";

    memset(&httpclient_data, 0, sizeof(httpclient_data));
    memset(http_content, 0, sizeof(HTTP_RESP_CONTENT_LEN));

    httpclient_data.response_buf = http_content;
    httpclient_data.response_buf_len = HTTP_RESP_CONTENT_LEN;

    memset(url, 0, sizeof(url));
    snprintf(url, sizeof(url), "http://%s/v1/device?act=getts", INTOYUN_HTTP_SERVER_DOMAIN);

    ret = httpclient_common(&httpclient, url, 80, NULL, HTTPCLIENT_GET, 1000, &httpclient_data);
    if (0 != ret) {
        MOLMC_LOGE(TAG, "request epoch time from remote server failed.");
        goto do_exit;
    } else {
        tsJson = cJSON_Parse(http_content);
        if(NULL == tsJson) {
            goto do_exit;
        }
        cJSON *tsObject = cJSON_GetObjectItem(tsJson, "ts");
        if(NULL == tsObject) {
            goto do_exit;
        }
        uint32_t res = tsObject->valueint;
        cJSON_Delete(tsJson);
        snprintf(copy, len, "%u", res);
        return res;
    }

do_exit:
    if(NULL != tsJson) {
        cJSON_Delete(tsJson);
    }
    return 0;
#undef HTTP_RESP_CONTENT_LEN
}

/**
 * 获取时间戳回调
 * */
static void coap_get_epoch_time_callback(void *pcontext, void *p_message)
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

    if (NULL == message->payload) {
        return;
    }
    MOLMC_LOGD(TAG, "Receive auth response message: %s", message->payload);

    switch (message->header.code) {
        case COAP_MSG_CODE_205_CONTENT:
            if(message->payloadlen < (sizeof(s_time_stamp.time_stamp_str)-1)) {
                s_time_stamp.get_time_stamp_success = true;
                memcpy(s_time_stamp.time_stamp_str, message->payload, message->payloadlen);
            }
            break;
        default:
            break;
    }
}

/**
 * 通过coap协议获取时间戳
 * */
uint64_t utils_coap_get_epoch_time(char copy[], int len)
{
    void *pclient = NULL;
    iotx_coap_config_t coap_config;
    iotx_message_t message;
    char uri[] = "/v2/device/ts";
    uint32_t timerID = timerGetId();
    uint64_t res = 0;

    iotx_device_info_pt pdev_info = iotx_device_info_get();

    memset(&coap_config, 0x00, sizeof(iotx_coap_config_t));
    coap_config.p_host = INTOYUN_COAP_SERVER_DOMAIN;
    coap_config.p_port = INTOYUN_COAP_SERVER_PORT;
    coap_config.wait_time_ms = 400;
    coap_config.event_handle = NULL;
    coap_config.p_devinfo = HAL_Malloc(sizeof(iotx_deviceinfo_t));
    memset(coap_config.p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    memcpy(coap_config.p_devinfo->device_id, pdev_info->device_id, DEVICE_ID_LEN);
    memcpy(coap_config.p_devinfo->device_secret, pdev_info->device_secret, IOTX_DEVICE_SECRET_LEN);

    pclient = IOT_CoAP_Init(&coap_config);
    if(NULL == pclient) {
        MOLMC_LOGE(TAG, "IOT_CoAP_Init() failed");
        goto do_exit;
    }

    memset(&message, 0, sizeof(iotx_message_t));
    message.method = COAP_MSG_CODE_GET;
    message.p_url = (uint8_t *)uri;
    message.p_querystr = NULL;
    message.p_payload = NULL;
    message.payload_len = 0;
    message.resp_callback = coap_get_epoch_time_callback;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    if(COAP_SUCCESS != IOT_CoAP_Client_Send((iotx_coap_context_t *)pclient, &message)) {
        goto do_exit;
    }

    memset(&s_time_stamp, 0, sizeof(time_stamp_t));
    while (!s_time_stamp.get_time_stamp_success) {
        if(timerIsEnd(timerID, 3000)) {
            break;
        }
        IOT_CoAP_Yield(pclient);
    }

    if(s_time_stamp.get_time_stamp_success && (len > strlen(s_time_stamp.time_stamp_str))) {
        strcpy(copy, s_time_stamp.time_stamp_str);
        res = atoi(s_time_stamp.time_stamp_str);
    } else {
        goto do_exit;
    }

do_exit:
    if(NULL != coap_config.p_devinfo) {
        HAL_Free(coap_config.p_devinfo);
    }

    if(NULL != pclient) {
        IOT_CoAP_Deinit(&pclient);
    }
    return res;
}

