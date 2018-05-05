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

#ifndef __IOT_EXPORT_LOG_H__
#define __IOT_EXPORT_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_log_api.h"

typedef struct
{
    void (*setLogLevel)(const char* tag, molmc_log_level_t level);
    log_output_fn_t (*setLogOutput)(log_output_fn_t func);
} iot_log_if_t;

extern const iot_log_if_t Log;

#ifdef __cplusplus
}
#endif


#endif

