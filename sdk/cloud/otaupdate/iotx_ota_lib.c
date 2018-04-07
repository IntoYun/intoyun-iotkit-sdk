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

#ifndef _IOTX_OTA_LIB_C_
#define _IOTX_OTA_LIB_C_

#include "utils_md5.h"

static void *otalib_MD5Init(void)
{
    iot_md5_context *ctx = HAL_Malloc(sizeof(iot_md5_context));
    if (NULL == ctx) {
        return NULL;
    }

    utils_md5_init(ctx);
    utils_md5_starts(ctx);

    return ctx;
}

static void otalib_MD5Update(void *md5, const char *buf, size_t buf_len)
{
    utils_md5_update(md5, (unsigned char *)buf, buf_len);
}

static void otalib_MD5Finalize(void *md5, char *output_str)
{
    int i;
    unsigned char buf_out[16];
    utils_md5_finish(md5, buf_out);

    for (i = 0; i < 16; ++i) {
        output_str[i * 2] = utils_hb2hex(buf_out[i] >> 4);
        output_str[i * 2 + 1] = utils_hb2hex(buf_out[i]);
    }
    output_str[32] = '\0';
}

static void otalib_MD5Deinit(void *md5)
{
    if (NULL != md5) {
        HAL_Free(md5);
    }
}

#endif /* IOTX_OTA_LIB_C_ */

