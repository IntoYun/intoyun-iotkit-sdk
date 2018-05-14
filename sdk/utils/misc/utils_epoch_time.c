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

#include <stdlib.h>
#include "utils_cJSON.h"
#include "utils_httpc.h"
#include "utils_epoch_time.h"
#include "iotx_log_api.h"
#include "iotx_comm_if_api.h"

const static char *TAG = "sdk:epoch_time";

uint64_t utils_get_epoch_time(char copy[], int len)
{
#define HTTP_RESP_CONTENT_LEN   (64)
    int ret;
    char http_content[HTTP_RESP_CONTENT_LEN];
    char url[64];
    httpclient_t httpclient;
    httpclient_data_t httpclient_data;
    cJSON *tsJson = NULL;

    memset(&httpclient, 0, sizeof(httpclient_t));
    memset(&httpclient_data, 0, sizeof(httpclient_data));
    memset(http_content, 0, sizeof(HTTP_RESP_CONTENT_LEN));

    httpclient.header = "Accept: text/xml,text/html\r\n";
    httpclient.header = "Connection: close\r\n";

    httpclient_data.response_buf = http_content;
    httpclient_data.response_buf_len = HTTP_RESP_CONTENT_LEN;

    memset(url, 0, sizeof(url));
    snprintf(url, sizeof(url), "http://%s/v1/device?act=getts", INTOYUN_HTTP_SERVER_DOMAIN);

    ret = httpclient_common(&httpclient, url, 80, NULL, HTTPCLIENT_GET, 1000, &httpclient_data);
    if (0 != ret) {
        MOLMC_LOGE(TAG, "request epoch time from remote server failed.");
        goto do_exit;
    } else {
        tsJson = cJSON_Parse(http_content);
        if(NULL == tsJson) {
            goto do_exit;
        }
        cJSON *tsObject = cJSON_GetObjectItem(tsJson, "ts");
        if(NULL == tsObject) {
            goto do_exit;
        }
        uint32_t res = tsObject->valueint;
        cJSON_Delete(tsJson);
        snprintf(copy, len, "%u", res);
        return res;
    }

do_exit:
    if(NULL != tsJson) {
        cJSON_Delete(tsJson);
    }
    return 0;
#undef HTTP_RESP_CONTENT_LEN
}

