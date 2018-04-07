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

#ifndef __IOTX_OTA_FETCH_C__
#define __IOTX_OTA_FETCH_C__

#include "utils_httpc.h"
#include "ca.h"

/* ofc, OTA fetch channel */
typedef struct {
    const char *url;
    httpclient_t http;              /* http client */
    httpclient_data_t http_data;    /* http client data */
} otahttp_Struct_t, *otahttp_Struct_pt;


void *ofc_Init(const char *url)
{
    otahttp_Struct_pt h_odc;

    if (NULL == (h_odc = HAL_Malloc(sizeof(otahttp_Struct_t)))) {
        log_err("allocate for h_odc failed");
        return NULL;
    }

    memset(h_odc, 0, sizeof(otahttp_Struct_t));
    /* set http request-header parameter */
    h_odc->http.header = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                         "Accept-Encoding: gzip, deflate\r\n";
    h_odc->url = url;
    return h_odc;
}

int32_t ofc_Fetch(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    int diff;
    otahttp_Struct_pt h_odc = (otahttp_Struct_pt)handle;

    h_odc->http_data.response_buf = buf;
    h_odc->http_data.response_buf_len = buf_len;
    diff = h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len;

    if (0 != httpclient_common(&h_odc->http, h_odc->url, 80, NULL, HTTPCLIENT_GET, timeout_s * 1000,
                               &h_odc->http_data)) {
        log_err("fetch firmware failed");
        return -1;
    }

    return h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len - diff;
}

int ofc_Deinit(void *handle)
{
    if (NULL != handle) {
        HAL_Free(handle);
    }

    return 0;
}

#endif

