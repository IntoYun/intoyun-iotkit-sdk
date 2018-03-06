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
#ifndef __LITE_LOG_INTERNAL_H__
#define __LITE_LOG_INTERNAL_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "lite-log.h"

#define LITE_LOG_ENABLED

#define LOG_MSG_MAXLEN                  (255)
#define LOG_MOD_NAME_LEN                (7)
#define LOG_PREFIX_FMT                  "[%s] %s(%d): "
#define HEXDUMP_SEP_LINE                "+" \
    "-----------------------" \
    "-----------------------" \
    "-----------------------"

#if defined(_PLATFORM_IS_LINUX_)
#undef  LOG_MSG_MAXLEN
#define LOG_MSG_MAXLEN                  (1023)
#endif

typedef struct {
    char            name[LOG_MOD_NAME_LEN + 1];
    int             priority;
    char            text_buf[LOG_MSG_MAXLEN + 1];
} log_client;

#endif  /* __LITE_LOG_INTERNAL_H__ */

