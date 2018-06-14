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

#ifndef __IOTX_COAP_CLIENT_H__
#define __IOTX_COAP_CLIENT_H__

#include "CoAPPacket/CoAPExport.h"
#include "iotx_coap_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define IOTX_DEVICE_ID_LEN       (64)   /* IoTx device ID length */
#define IOTX_DEVICE_SECRET_LEN   (64)   /* IoTx device secret length */
#define IOTX_URI_MAX_LEN         (135)  /* IoTx CoAP uri maximal length */
#define CONFIG_COAP_AUTH_TIMEOUT (10 * 1000)
#define IOTX_COAP_PING_INTERVAL_S  (60 * 1000)   /* ping包发送间隔时间 */
#define IOTX_COAP_AUTH_INTERVAL_S  (42 * 60 * 60 * 1000) /** 设备认证token过期时间为48小时*/

typedef struct {
  char                *p_auth_token;
  int                  auth_token_len;
  char                 is_authed;
  char                 is_connected;
  iotx_deviceinfo_t    *p_devinfo;
  CoAPContext          *p_coap_ctx;
  unsigned int         coap_token;
  iotx_time_t          heartbeat_timer;         /* the next time point of ping */
  iotx_time_t          auth_timer;         /* the next time point of device auth */
  iotx_event_handle_t  event_handle;
} iotx_coap_t, *iotx_coap_pt;

#ifdef __cplusplus
}
#endif

#endif

