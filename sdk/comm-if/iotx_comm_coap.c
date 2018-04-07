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

#include "iotx_coap_api.h"

static int iotx_comm_connect(void)
{
    return 0;
}

static bool iotx_comm_isconnected(void)
{
    return iotx_conn_info_get()->is_connected;
}

static int iotx_comm_disconnect(void)
{
    iotx_coap_context_t *pclient = (iotx_coap_context_t *)iotx_conn_info_get()->pclient;

    if(NULL != pclient) {
        IOT_CoAP_Deinit(pclient);
    }
    return 0;
}

static int iotx_comm_senddata(const uint8_t *data, uint16_t datalen)
{
    return 0;
}

static int iotx_comm_reportprogress(uint8_t type, iotx_ota_reply_t reply, uint8_t progress)
{
    return 0;
}

static int iotx_comm_yield(void)
{
    iotx_coap_context_t *pclient = (iotx_coap_context_t *)iotx_conn_info_get()->pclient;

    if(NULL != pclient) {
        IOT_CoAP_Yield(pclient);
    }

    return 0;
}

