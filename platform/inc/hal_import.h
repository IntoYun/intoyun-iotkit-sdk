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

#ifndef __HAL_IMPORT_H__
#define __HAL_IMPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>


#define NETWORK_ADDR_LEN 16

typedef struct
{
    char addr[NETWORK_ADDR_LEN];
    unsigned short port;
} NetworkAddr;


#include "hal_os_interface.h"
#include "hal_tcp_interface.h"
#include "hal_udp_interface.h"
#include "hal_dtls_interface.h"
#include "hal_tls_interface.h"

#ifdef __cplusplus
}
#endif

#endif  /* __HAL_IMPORT_H__ */

