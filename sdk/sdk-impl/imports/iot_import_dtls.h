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

#ifndef __IOT_IMPORT_DTLS_H__
#define __IOT_IMPORT_DTLS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#include "iot_import.h"
#include "sdk_config.h"

#define dtls_log_print(level, ...) \
    {\
        fprintf(stderr, "%s [%s #%d]   ",level, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);\
    }

#define DTLS_TRC(fmt,  ...)  dtls_log_print("<TRACE>", fmt, ##__VA_ARGS__)
#define DTLS_DUMP(fmt, ...)  dtls_log_print("<DUMP> ", fmt, ##__VA_ARGS__)
#define DTLS_DEBUG(fmt,...)  dtls_log_print("<DEBUG>", fmt, ##__VA_ARGS__)
#define DTLS_INFO(fmt, ...)  dtls_log_print("<INFO> ", fmt, ##__VA_ARGS__)
#define DTLS_ERR(fmt,  ...)  dtls_log_print("<ERROR>", fmt, ##__VA_ARGS__)

#endif

