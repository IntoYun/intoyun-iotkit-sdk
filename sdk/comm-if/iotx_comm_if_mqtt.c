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
#include "utils_list.h"
#include "lite-utils.h"
#include "iotx_comm_if_api.h"
#include "iot_import_mqtt.h"
#include "iotx_mqtt_client.h"
#include "iotx_guider_api.h"
#include "iotx_crypto_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if.h"
#include "iotx_datapoint_api.h"

#if CONFIG_CLOUD_CHANNEL == 1     //MQTT

const static char *TAG = "sdk:comm-if-mqtt";

#define MQTT_MSGLEN                   (1024)

#define SUB_TX_TOPIC                  "tx"
#define PUB_RX_TOPIC                  "rx"
#define PUB_INFO_TOPIC                "info"
#define SUB_ACTION_TOPIC              "action"
#define PUB_REPLY_TOPIC               "reply"

extern int iotx_get_device_info(char *buf, uint16_t buflen);

static int gen_mqt_topic(char *buf, uint16_t buf_len, const char *name)
{
    iotx_device_info_pt pdev_info = iotx_device_info_get();

    int ret = snprintf(buf, buf_len, "v2/device/%s/%s", pdev_info->device_id, name);

    LITE_ASSERT(ret < buf_len);

    if (ret < 0) {
        MOLMC_LOGE(TAG, "snprintf failed");
        return -1;
    }

    return 0;
}

static void cloud_data_receive_callback(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    MOLMC_LOGD(TAG, "cloud_data_receive_callback");
#if CONFIG_CLOUD_DATAPOINT_ENABLED == 1
    IOT_DataPoint_ParseReceiveDatapoints((uint8_t *)ptopic_info->payload, ptopic_info->payload_len);
#endif
    IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_data, (uint8_t *)ptopic_info->payload, ptopic_info->payload_len);
}

static void cloud_action_callback(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    MOLMC_LOGD(TAG, "cloud_action_callback");
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    IOT_SYSTEM_NotifyEvent(event_cloud_comm, ep_cloud_comm_ota, (uint8_t *)ptopic_info->payload, ptopic_info->payload_len);
}

static int iotx_mqtt_pre_auth_process(void)
{
    return iotx_guider_authenticate();
}

static int iotx_mqtt_post_auth_process(void)
{
    return iotx_guider_auth_gen_keys();
}

static int iotx_mqtt_up_process(char *topic, iotx_mqtt_topic_info_pt topic_msg)
{
    int ret = -1;
    uint8_t *out_buf = NULL;
    uint16_t out_buf_len = topic_msg->payload_len + 6;

    MOLMC_LOGD(TAG, "iotx_mqtt_up_process");

    out_buf = (uint8_t *)HAL_Malloc(out_buf_len);
    if(NULL == out_buf) {
        MOLMC_LOGE(TAG, "malloc failed!");
        return -1;
    }

    out_buf_len = iotx_comm_payload_encrypt(out_buf, out_buf_len, (uint8_t *)topic_msg->payload, topic_msg->payload_len);
    if (out_buf_len <= 0) {
        MOLMC_LOGE(TAG, "payload encrypt error!");
        ret = -1;
        goto exit;
    }

    memset((void *)topic_msg->payload, 0, topic_msg->payload_len);
    memcpy((void *)topic_msg->payload, out_buf, out_buf_len);
    topic_msg->payload_len = out_buf_len;

    //MOLMC_LOG_BUFFER_HEX(TAG, topic_msg->payload, topic_msg->payload_len);
    ret = 0;
exit:
    HAL_Free(out_buf);
    return ret;
}

static int iotx_mqtt_down_process(iotx_mqtt_topic_info_pt topic_msg)
{
    int ret = -1;
    uint8_t *out_buf = NULL;
    uint16_t out_buf_len = topic_msg->payload_len;

    MOLMC_LOGD(TAG, "iotx_mqtt_down_process\r\n");

    out_buf = (uint8_t *)HAL_Malloc(out_buf_len);
    if(NULL == out_buf) {
        MOLMC_LOGE(TAG, "malloc failed!");
        return -1;
    }

    out_buf_len = iotx_comm_payload_decrypt(out_buf, out_buf_len, (uint8_t *)topic_msg->payload, topic_msg->payload_len);
    if (out_buf_len <= 0) {
        MOLMC_LOGE(TAG, "payload encrypt error!");
        ret = -1;
        goto exit;
    }

    memset((void *)topic_msg->payload, 0, topic_msg->payload_len);
    memcpy((void *)topic_msg->payload, out_buf, out_buf_len);
    topic_msg->payload_len = out_buf_len;

    //MOLMC_LOG_BUFFER_HEX(TAG, topic_msg->payload, topic_msg->payload_len);
    ret = 0;
exit:
    HAL_Free(out_buf);
    return ret;
}

int iotx_comm_connect(void)
{
    int rc = 0;
    char *msg_writebuf = NULL, *msg_readbuf = NULL;
    void *pclient = NULL;
    char topic_name[TOPIC_NAME_LEN] = {0}, msg_buf[256] = {0};
    iotx_mqtt_topic_info_t topic_msg;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_mqtt_param_t mqtt_params;

    iotx_comm_disconnect();

    if (NULL == (msg_writebuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        MOLMC_LOGI(TAG, "not enough memory");
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        MOLMC_LOGI(TAG, "not enough memory");
        goto do_exit;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_writebuf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.mqtt_pre_auth_process = iotx_mqtt_pre_auth_process;
    mqtt_params.mqtt_post_auth_process = iotx_mqtt_post_auth_process;
    mqtt_params.mqtt_up_process = iotx_mqtt_up_process;
    mqtt_params.mqtt_down_process = iotx_mqtt_down_process;

    mqtt_params.reconnect_open = false;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        MOLMC_LOGI(TAG, "MQTT construct failed");
        goto do_exit;
    }

    //订阅数据点接收
    if (gen_mqt_topic(pconn_info->topic_name_tx, TOPIC_NAME_LEN, SUB_TX_TOPIC) < 0) {
        MOLMC_LOGE(TAG, "generate topic name of info failed");
        goto do_exit;
    }

    rc = IOT_MQTT_Subscribe(pclient, pconn_info->topic_name_tx, IOTX_MQTT_QOS0, cloud_data_receive_callback, NULL);
    if (rc < 0) {
        MOLMC_LOGI(TAG, "IOT_MQTT_Subscribe() failed, rc = %d", rc);
        goto do_exit;
    }

    //订阅命令控制
    if (gen_mqt_topic(pconn_info->topic_name_action, TOPIC_NAME_LEN, SUB_ACTION_TOPIC) < 0) {
        MOLMC_LOGE(TAG, "generate topic name of info failed");
        goto do_exit;
    }

    rc = IOT_MQTT_Subscribe(pclient, pconn_info->topic_name_action, IOTX_MQTT_QOS0, cloud_action_callback, NULL);
    if (rc < 0) {
        MOLMC_LOGI(TAG, "IOT_MQTT_Subscribe() failed, rc = %d", rc);
        goto do_exit;
    }

    //发送info
    if (iotx_get_device_info(msg_buf, sizeof(msg_buf)) < 0) {
        MOLMC_LOGE(TAG, "generate info failed");
        goto do_exit;
    }

    if (gen_mqt_topic(topic_name, TOPIC_NAME_LEN, PUB_INFO_TOPIC) < 0) {
        MOLMC_LOGE(TAG, "generate topic name of info failed");
        goto do_exit;
    }

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 1;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_buf;
    topic_msg.payload_len = strlen(msg_buf);

    rc = IOT_MQTT_Publish(pclient, topic_name, &topic_msg) < 0 ? -1 : 0;
    if (rc < 0) {
        MOLMC_LOGI(TAG, "publish info failed, rc = %d", rc);
        goto do_exit;
    }

    pconn_info->pclient = pclient;
    return 0;

do_exit:

    if (NULL != msg_writebuf) {
        HAL_Free(msg_writebuf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    if(NULL != pclient) {
        IOT_MQTT_Destroy(&pclient);
    }
    return -1;
}

bool iotx_comm_isconnected(void)
{
    void *pclient = iotx_conn_info_get()->pclient;

    if(NULL == pclient) {
        return false;
    }

    return IOT_MQTT_CheckStateNormal(pclient);
}

int iotx_comm_disconnect(void)
{
    iotx_mc_client_t *pclient = iotx_conn_info_get()->pclient;
    char *msg_writebuf = NULL, *msg_readbuf = NULL;

    if(NULL != pclient) {
        msg_writebuf = pclient->buf_send;
        msg_readbuf = pclient->buf_read;

        IOT_MQTT_Destroy((void **)&pclient);

        if (NULL != msg_writebuf) {
            HAL_Free(msg_writebuf);
        }
        if (NULL != msg_readbuf) {
            HAL_Free(msg_readbuf);
        }
    }
    return 0;
}

int iotx_comm_send(iotx_conn_send_t sendType, const uint8_t *data, uint16_t datalen)
{
    char topic_name[TOPIC_NAME_LEN] = {0}, topic_tail[16] = {0};
    iotx_mqtt_topic_info_t topic_msg;

    void *pclient = iotx_conn_info_get()->pclient;
    if(NULL == pclient) {
        MOLMC_LOGE(TAG, "not connect");
        return -1;
    }

    if(sendType == IOTX_CONN_SEND_DATA) {
        strcpy(topic_tail, PUB_RX_TOPIC);
    } else if(sendType == IOTX_CONN_SEND_ACTION_REPLY) {
        strcpy(topic_tail, PUB_REPLY_TOPIC);
    }

    if (gen_mqt_topic(topic_name, TOPIC_NAME_LEN, topic_tail) < 0) {
        MOLMC_LOGE(TAG, "generate topic name of info failed");
        return -1;
    }

    uint16_t payloadlen = datalen + 6; //预留填充 package(2个字节) + mic(4个字节)
    uint8_t *payload = (uint8_t *)HAL_Malloc(payloadlen);
    if(NULL == payload) {
        MOLMC_LOGE(TAG, "mem malloc failed");
        return -1;
    }
    memset(payload, 0, payloadlen);
    memcpy(payload, data, datalen);

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = datalen;

    int rc = IOT_MQTT_Publish(pclient, topic_name, &topic_msg);
    HAL_Free(payload);
    return rc;
}

int iotx_comm_yield(void)
{
    void *pclient = iotx_conn_info_get()->pclient;

    if(NULL != pclient) {
        IOT_MQTT_Yield(pclient, 200);
    }
    return 0;
}

#endif

