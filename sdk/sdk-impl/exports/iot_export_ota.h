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

#ifndef __IOT_EXPORT_OTA_H__
#define __IOT_EXPORT_OTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_ota_api.h"

typedef struct
{
    void *(*init)(const char *product_key, const char *device_name, void *ch_signal);
    int (*deInit)(void *handle);
    int (*reportVersion)(void *handle, const char *version);
    int (*reportProgress)(void *handle, IOT_OTA_Progress_t progress, const char *msg);
    bool (*isFetching)(void *handle);
    bool (*isFetchFinish)(void *handle);
    int (*fetchYield)(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s);
    int (*ioctl)(void *handle, IOT_OTA_CmdType_t type, void *buf, size_t buf_len);
    int (*getLastError)(void *handle);
} iot_ota_if_t;

extern const iot_ota_if_t OTAUpdate;

#ifdef __cplusplus
}
#endif

#endif /* __OTA_EXPORT_H__ */

