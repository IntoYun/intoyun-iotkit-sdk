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

#ifndef __HAL_UDP_INTERFACE_H__
#define __HAL_UDP_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_import.h"

intptr_t HAL_UDP_create(const char *host, unsigned short port);
void HAL_UDP_close(intptr_t p_socket);
int HAL_UDP_write(intptr_t p_socket, const unsigned char *p_data, unsigned int datalen);
int HAL_UDP_read(intptr_t p_socket, unsigned char *p_data, unsigned int datalen);
int HAL_UDP_readTimeout(intptr_t p_socket, unsigned char *p_data, unsigned int datalen, unsigned int timeout);
int HAL_UDP_recvfrom(intptr_t sockfd, NetworkAddr *p_remote, unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms);
int HAL_UDP_sendto(intptr_t sockfd, const NetworkAddr *p_remote, const unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms);
int HAL_UDP_joinmulticast(intptr_t sockfd, const char *p_group);

#ifdef __cplusplus
}
#endif

#endif

