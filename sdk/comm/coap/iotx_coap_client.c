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

#include <stdio.h>

#include "lite-utils.h"
#include "iot_import_coap.h"
#include "iotx_coap_client.h"


#define IOTX_AUTH_TOKEN_LEN      (32+1)
#define IOTX_COAP_INIT_TOKEN     (0x01020304)
#define IOTX_LIST_MAX_ITEM       (10)

const static char *TAG = "sdk:coap";

static unsigned int iotx_get_coap_token(iotx_coap_t *p_iotx_coap, unsigned char *p_encoded_data)
{
    unsigned int value = p_iotx_coap->coap_token;
    p_encoded_data[0] = (unsigned char)((value & 0x00FF) >> 0);
    p_encoded_data[1] = (unsigned char)((value & 0xFF00) >> 8);
    p_encoded_data[2] = (unsigned char)((value & 0xFF0000) >> 16);
    p_encoded_data[3] = (unsigned char)((value & 0xFF000000) >> 24);
    p_iotx_coap->coap_token++;
    MOLMC_LOG_BUFFER_HEX(TAG, p_encoded_data, 4);
    return sizeof(unsigned int);
}

static int iotx_path_2_option(char *uri, CoAPMessage *message)
{
    char *ptr     = NULL;
    char *pstr    = NULL;
    char  path[COAP_MSG_MAX_PATH_LEN]  = {0};

    if (NULL == uri || NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter uri %p, p_message %p", uri, message);
        return IOTX_ERR_INVALID_PARAM;
    }
    if (IOTX_URI_MAX_LEN < strlen(uri)) {
        MOLMC_LOGE(TAG, "The uri length is too loog,len = %d", (int)strlen(uri));
        return IOTX_ERR_URI_TOO_LOOG;
    }
    MOLMC_LOGD(TAG, "The uri is %s", uri);
    ptr = pstr = uri;
    while ('\0' != *ptr) {
        if ('/' == *ptr) {
            if (ptr != pstr) {
                memset(path, 0x00, sizeof(path));
                strncpy(path, pstr, ptr - pstr);
                MOLMC_LOGD(TAG, "path: %s,len=%d", path, (int)(ptr - pstr));
                CoAPStrOption_add(message, COAP_OPTION_URI_PATH,
                        (unsigned char *)path, (int)strlen(path));
            }
            pstr = ptr + 1;
        }
        if ('\0' == *(ptr + 1) && '\0' != *pstr) {
            memset(path, 0x00, sizeof(path));
            strncpy(path, pstr, sizeof(path) - 1);
            MOLMC_LOGD(TAG, "topic: %s,len=%d", path, (int)strlen(path));
            CoAPStrOption_add(message, COAP_OPTION_URI_PATH,
                    (unsigned char *)path, (int)strlen(path));
            CoAPMessageTopic_set(message, (unsigned char *)path);
        }
        ptr ++;
    }
    return IOTX_SUCCESS;
}

static int iotx_querystr_2_option(char *querystr, CoAPMessage *message)
{
    char *ptr     = NULL;
    char *pstr    = NULL;
    char  query[COAP_MSG_MAX_PATH_LEN]  = {0};

    if (NULL == querystr || NULL == message) {
        return IOTX_ERR_INVALID_PARAM;
    }
    if (IOTX_URI_MAX_LEN < strlen(querystr)) {
        MOLMC_LOGE(TAG, "The uri length is too loog,len = %d", (int)strlen(querystr));
        return IOTX_ERR_URI_TOO_LOOG;
    }
    MOLMC_LOGD(TAG, "The querystr is %s", querystr);
    ptr = pstr = querystr;
    while ('\0' != *ptr) {
        if ('&' == *ptr) {
            if (ptr != pstr) {
                memset(query, 0x00, sizeof(query));
                strncpy(query, pstr, ptr - pstr);
                MOLMC_LOGD(TAG, "query: %s,len=%d", query, (int)(ptr - pstr));
                CoAPStrOption_add(message, COAP_OPTION_URI_QUERY, (unsigned char *)query, (int)strlen(query));
            }
            pstr = ptr + 1;
        }
        if ('\0' == *(ptr + 1) && '\0' != *pstr) {
            memset(query, 0x00, sizeof(query));
            strncpy(query, pstr, sizeof(query) - 1);
            MOLMC_LOGD(TAG, "query: %s,len=%d", query, (int)strlen(query));
            CoAPStrOption_add(message, COAP_OPTION_URI_QUERY, (unsigned char *)query, (int)strlen(query));
        }
        ptr ++;
    }
    return IOTX_SUCCESS;
}

void iotx_event_notifyer(unsigned int code, CoAPMessage *message)
{
    if (NULL == message) {
        MOLMC_LOGE(TAG, "Invalid paramter, message %p", message);
        return ;
    }

    MOLMC_LOGE(TAG, "Error code: 0x%x, payload: %s", code, message->payload);
    switch (code) {
        case COAP_MSG_CODE_402_BAD_OPTION:
        case COAP_MSG_CODE_401_UNAUTHORIZED:
            {
                iotx_coap_t *p_context = NULL;
                if (NULL != message->user) {
                    p_context = (iotx_coap_t *)message->user;
                    p_context->is_authed = IOT_FALSE;
                    MOLMC_LOGI(TAG, "IoTx token expired, will reauthenticate");
                }
                /* TODO: call event handle to notify application */
                /* p_context->event_handle(); */
                break;
            }
        default:
            break;
    }
}

int IOT_CoAP_Client_Observe_Send(iotx_coap_context_t *p_context, iotx_message_t *p_message)
{
    int ret = COAP_SUCCESS;
    CoAPMessage message;
    unsigned char tokenlen;
    unsigned char token[COAP_MSG_MAX_TOKEN_LEN] = {0};
    CoAPContext      *p_coap_ctx = NULL;
    iotx_coap_t      *p_iotx_coap = NULL;

    p_iotx_coap = (iotx_coap_t *)p_context;
    p_coap_ctx = (CoAPContext *)p_iotx_coap->p_coap_ctx;

    CoAPMessage_init(&message);
    CoAPMessageType_set(&message, COAP_MESSAGE_TYPE_CON);
    CoAPMessageCode_set(&message, p_message->method);
    CoAPMessageId_set(&message, CoAPMessageId_gen(p_coap_ctx));
    tokenlen = iotx_get_coap_token(p_iotx_coap, token);
    CoAPMessageToken_set(&message, token, tokenlen);
    CoAPMessageHandler_set(&message, p_message->resp_callback);
    CoAPMessageUserData_set(&message, (void *)p_iotx_coap);
    //注意option添加顺序，需按照option num.大小顺序依次添加
    CoAPUintOption_add(&message, COAP_OPTION_OBSERVE, 0);

    iotx_path_2_option((char *)p_message->p_url, &message);

    if (IOTX_CONTENT_TYPE_CBOR == p_message->content_type) {
        CoAPUintOption_add(&message, COAP_OPTION_CONTENT_FORMAT, COAP_CT_APP_CBOR);
    } else {
        CoAPUintOption_add(&message, COAP_OPTION_CONTENT_FORMAT, COAP_CT_APP_JSON);
    }

    iotx_querystr_2_option((char *) p_message->p_querystr, &message);

    CoAPUintOption_add(&message, COAP_OPTION_ACCEPT, COAP_CT_APP_OCTET_STREAM);

    if(NULL != p_message->p_payload) {
        CoAPMessagePayload_set(&message, p_message->p_payload, p_message->payload_len);
    }

    ret = CoAPMessage_send(p_coap_ctx, &message);

    CoAPMessage_destory(&message);

    return ret;
}

int IOT_CoAP_Client_Send(iotx_coap_context_t *p_context, iotx_message_t *p_message)
{
    int ret = COAP_SUCCESS;
    CoAPMessage message;
    unsigned char tokenlen;
    unsigned char token[COAP_MSG_MAX_TOKEN_LEN] = {0};
    CoAPContext      *p_coap_ctx = NULL;
    iotx_coap_t      *p_iotx_coap = NULL;

    p_iotx_coap = (iotx_coap_t *)p_context;
    p_coap_ctx = (CoAPContext *)p_iotx_coap->p_coap_ctx;

    CoAPMessage_init(&message);
    CoAPMessageType_set(&message, COAP_MESSAGE_TYPE_CON);
    CoAPMessageCode_set(&message, p_message->method);
    CoAPMessageId_set(&message, CoAPMessageId_gen(p_coap_ctx));
    tokenlen = iotx_get_coap_token(p_iotx_coap, token);
    CoAPMessageToken_set(&message, token, tokenlen);
    CoAPMessageHandler_set(&message, p_message->resp_callback);
    CoAPMessageUserData_set(&message, (void *)p_iotx_coap);

    //注意option添加顺序，需按照option num.大小顺序依次添加
    iotx_path_2_option((char *)p_message->p_url, &message);

    if (IOTX_CONTENT_TYPE_CBOR == p_message->content_type) {
        CoAPUintOption_add(&message, COAP_OPTION_CONTENT_FORMAT, COAP_CT_APP_CBOR);
    } else {
        CoAPUintOption_add(&message, COAP_OPTION_CONTENT_FORMAT, COAP_CT_APP_JSON);
    }

    iotx_querystr_2_option((char *)p_message->p_querystr, &message);

    CoAPUintOption_add(&message, COAP_OPTION_ACCEPT, COAP_CT_APP_OCTET_STREAM);


    if(NULL != p_message->p_payload) {
        MOLMC_LOGI(TAG, "payload: %s \n length: %d", p_message->p_payload, p_message->payload_len);
        CoAPMessagePayload_set(&message, p_message->p_payload, p_message->payload_len);
    }

    ret = CoAPMessage_send(p_coap_ctx, &message);

    CoAPMessage_destory(&message);

    return ret;
}


iotx_coap_context_t *IOT_CoAP_Init(iotx_coap_config_t *p_config)
{
    iotx_coap_t *p_iotx_coap = NULL;
    CoAPInitParam param;
    char url[128] = {0};

    if (NULL == p_config) {
        MOLMC_LOGE(TAG, "Invalid paramter p_config %p", p_config);
        return NULL;
    }
    if (NULL == p_config->p_devinfo) {
        MOLMC_LOGE(TAG, "Invalid paramter p_devinfo %p", p_config->p_devinfo);
        return NULL;
    }

    p_iotx_coap = (iotx_coap_t *)coap_malloc(sizeof(iotx_coap_t));
    if (NULL == p_iotx_coap) {
        MOLMC_LOGE(TAG, " Allocate memory for iotx_coap_context_t failed");
        return NULL;
    }
    memset(p_iotx_coap, 0x00, sizeof(iotx_coap_t));

    p_iotx_coap->p_auth_token = coap_malloc(IOTX_AUTH_TOKEN_LEN);
    if (NULL == p_iotx_coap->p_auth_token) {
        MOLMC_LOGE(TAG, " Allocate memory for auth token failed");
        goto err;
    }
    memset(p_iotx_coap->p_auth_token, 0x00, IOTX_AUTH_TOKEN_LEN);

    /*Set the client isn't authed*/
    p_iotx_coap->is_authed = IOT_FALSE;
    p_iotx_coap->auth_token_len = IOTX_AUTH_TOKEN_LEN;

    /*Get deivce information*/
    p_iotx_coap->p_devinfo = coap_malloc(sizeof(iotx_deviceinfo_t));
    if (NULL == p_iotx_coap->p_devinfo) {
        MOLMC_LOGE(TAG, " Allocate memory for iotx_deviceinfo_t failed");
        goto err;
    }
    memset(p_iotx_coap->p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));

    /*It should be implement by the user*/
    if (NULL != p_config->p_devinfo) {
        memset(p_iotx_coap->p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
        strncpy(p_iotx_coap->p_devinfo->device_id, p_config->p_devinfo->device_id, IOTX_DEVICE_ID_LEN);
        strncpy(p_iotx_coap->p_devinfo->device_secret, p_config->p_devinfo->device_secret, IOTX_DEVICE_SECRET_LEN);
    }

    /*Init coap token*/
    p_iotx_coap->coap_token = IOTX_COAP_INIT_TOKEN;

    /*Create coap context*/
    snprintf(url, sizeof(url), "%s:%d", p_config->p_host, p_config->p_port);
    memset(&param, 0x00, sizeof(CoAPInitParam));
    param.url = url;
    param.maxcount = IOTX_LIST_MAX_ITEM;
    param.notifier = (CoAPEventNotifier)iotx_event_notifyer;
    param.waittime = p_config->wait_time_ms;

    p_iotx_coap->p_coap_ctx = CoAPContext_create(&param);

    if (NULL == p_iotx_coap->p_coap_ctx) {
        MOLMC_LOGE(TAG, " Create coap context failed");
        goto err;
    }

    /*Register the event handle to notify the application */
    p_iotx_coap->event_handle = p_config->event_handle;
    p_iotx_coap->is_connected = IOT_TRUE;
    iotx_time_init(&(p_iotx_coap->heartbeat_timer));
    iotx_time_init(&(p_iotx_coap->auth_timer));

    return (iotx_coap_context_t *)p_iotx_coap;
err:
    /* Error, release the memory */
    if (NULL != p_iotx_coap) {
        if (NULL != p_iotx_coap->p_devinfo) {
            coap_free(p_iotx_coap->p_devinfo);
        }
        if (NULL != p_iotx_coap->p_auth_token) {
            coap_free(p_iotx_coap->p_auth_token);
        }
        if (NULL != p_iotx_coap->p_coap_ctx) {
            CoAPContext_free(p_iotx_coap->p_coap_ctx);
        }

        p_iotx_coap->auth_token_len = 0;
        p_iotx_coap->is_authed = IOT_FALSE;
        coap_free(p_iotx_coap);
    }
    p_iotx_coap->is_connected = IOT_FALSE;
    return NULL;
}


int IOT_CoAP_Auth(iotx_coap_context_t *p_context, iotx_message_t *message)
{
    //int len = 0;
    int ret = COAP_SUCCESS;
    //CoAPContext      *p_coap_ctx = NULL;
    iotx_coap_t      *p_iotx_coap = NULL;

    p_iotx_coap = (iotx_coap_t *)p_context;
    if (NULL == p_iotx_coap || (NULL != p_iotx_coap && (NULL == p_iotx_coap->p_auth_token
                    || NULL == p_iotx_coap->p_coap_ctx || 0 == p_iotx_coap->auth_token_len))) {
        MOLMC_LOGD(TAG, "Invalid paramter");
        return IOTX_ERR_INVALID_PARAM;
    }

    //p_coap_ctx = (CoAPContext *)p_iotx_coap->p_coap_ctx;

    ret = IOT_CoAP_Client_Send(p_context, message);

    return ret;
}

int IOT_CoAP_SendMessage(iotx_coap_context_t *p_context, iotx_message_t *p_message)
{
    return 0;
}

int IOT_CoAP_GetMessagePayload(void *p_message, unsigned char **pp_payload, int *p_len)
{
    CoAPMessage *message = NULL;

    if (NULL == p_message || NULL == pp_payload || NULL == p_len) {
        MOLMC_LOGE(TAG, "Invalid paramter p_message %p, pp_payload %p, p_len %p", p_message, pp_payload, p_len);
        return IOTX_ERR_INVALID_PARAM;
    }
    message = (CoAPMessage *)p_message;
    *pp_payload    =  message->payload;
    *p_len         =  message->payloadlen;

    return IOTX_SUCCESS;
}

int IOT_CoAP_GetMessageCode(void *p_message, iotx_coap_resp_code_t *p_resp_code)
{
    CoAPMessage *message = NULL;

    if (NULL == p_message || NULL == p_resp_code) {
        MOLMC_LOGE(TAG, "Invalid paramter p_message %p, p_resp_code %p",
                p_message, p_resp_code);
        return IOTX_ERR_INVALID_PARAM;
    }
    message = (CoAPMessage *)p_message;
    *p_resp_code   = (iotx_coap_resp_code_t) message->header.code;

    return IOTX_SUCCESS;
}

void IOT_CoAP_Deinit(iotx_coap_context_t **pp_context)
{
    iotx_coap_t *p_iotx_coap = NULL;

    if (NULL != pp_context && NULL != *pp_context) {
        p_iotx_coap = (iotx_coap_t *)*pp_context;
        p_iotx_coap->is_authed = IOT_FALSE;
        p_iotx_coap->auth_token_len = 0;
        p_iotx_coap->coap_token = IOTX_COAP_INIT_TOKEN;

        if (NULL != p_iotx_coap->p_auth_token) {
            coap_free(p_iotx_coap->p_auth_token);
            p_iotx_coap->p_auth_token = NULL;
        }

        if (NULL != p_iotx_coap->p_devinfo) {
            coap_free(p_iotx_coap->p_devinfo);
            p_iotx_coap->p_devinfo = NULL;
        }

        if (NULL != p_iotx_coap->p_coap_ctx) {
            CoAPContext_free(p_iotx_coap->p_coap_ctx);
            p_iotx_coap->p_coap_ctx = NULL;
        }

        coap_free(p_iotx_coap);
        *pp_context = NULL;
    }
}

int IOT_CoAP_Yield(iotx_coap_context_t *p_context)
{
    iotx_coap_t *p_iotx_coap = NULL;
    p_iotx_coap = (iotx_coap_t *)p_context;
    if (NULL == p_iotx_coap || (NULL != p_iotx_coap && NULL == p_iotx_coap->p_coap_ctx)) {
        MOLMC_LOGE(TAG, "Invalid paramter");
        return IOTX_ERR_INVALID_PARAM;
    }
    return CoAPMessage_cycle(p_iotx_coap->p_coap_ctx);
}

