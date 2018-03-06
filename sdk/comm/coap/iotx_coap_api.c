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

#include "iot_import_coap.h"
#include "lite-utils.h"
#include "CoAPPacket/CoAPMessage.h"
#include "CoAPPacket/CoAPExport.h"
#include "iotx_coap_api.h"

iotx_coap_context_t *IOT_CoAP_Init(iotx_coap_config_t *p_config)
{
    return NULL;
}

void IOT_CoAP_Deinit(iotx_coap_context_t **pp_context)
{
}

int IOT_CoAP_DeviceNameAuth(iotx_coap_context_t *p_context)
{
    return IOTX_SUCCESS;
}

int IOT_CoAP_SendMessage(iotx_coap_context_t *p_context, char *p_path, iotx_message_t *p_message)
{
    return IOTX_SUCCESS;
}

int IOT_CoAP_GetMessagePayload(void *p_message, unsigned char **pp_payload, int *p_len)
{
    return IOTX_SUCCESS;
}

int IOT_CoAP_GetMessageCode(void *p_message, iotx_coap_resp_code_t *p_resp_code)
{
    return IOTX_SUCCESS;
}

int IOT_CoAP_Yield(iotx_coap_context_t *p_context)
{
    return IOTX_SUCCESS;
}

