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

#ifndef __IOT_EXPORT_COAP_H__
#define __IOT_EXPORT_COAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_coap_api.h"

typedef struct
{
    iotx_coap_context_t *(*init)(iotx_coap_config_t *p_config);
    void (*deInit)(iotx_coap_context_t **p_context);
    int (*deviceNameAuth)(iotx_coap_context_t *p_context, iotx_message_t *message);
    int (*yield)(iotx_coap_context_t *p_context);
    int (*sendMessage)(iotx_coap_context_t *p_context, iotx_message_t *p_message);
    int (*getMessagePayload)(void *p_message, unsigned char **pp_payload, int *p_len);
    int (*getMessageCode)(void *p_message, iotx_coap_resp_code_t *p_resp_code);
} iot_coap_client_if_t;

extern const iot_coap_client_if_t CoAPClient;

#ifdef __cplusplus
}
#endif

#endif

