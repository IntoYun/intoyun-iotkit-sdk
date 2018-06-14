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

#ifndef __IOTX_COMM_IF_H__
#define __IOTX_COMM_IF_H__

/* send type */
typedef enum {
    IOTX_CONN_SEND_DATA = 0,               /* 发送数据 */
    IOTX_CONN_SEND_ACTION_REPLY = 1,       /* 发送指令回复 */
} iotx_conn_send_t;

#ifdef __cplusplus
extern "C"
{
#endif

int iotx_comm_connect(void);
bool iotx_comm_isconnected(void);
int iotx_comm_disconnect(void);
int iotx_comm_send(iotx_conn_send_t type, const uint8_t *data, uint16_t datalen);
int iotx_comm_yield(void);

#ifdef __cplusplus
}
#endif

#endif

