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

#include "iot_import.h"
#include "iotx_ota_api.h"
#include "iotx_comm_if_api.h"
#include "ca.h"

const static char *TAG = "sdk:otaupdate";

/* ofc, OTA fetch channel */
typedef struct {
    const char *url;
    httpclient_t http;              /* http client */
    httpclient_data_t http_data;    /* http client data */
} otahttp_Struct_t, *otahttp_Struct_pt;

typedef struct  {
    IOT_OTA_State_t state;         /* OTA state */
    uint8_t type_file;                  /* firmware type */
    char *purl;                    /* point to URL */
    char *pmd5;                    /* point to MD5 */
    uint32_t size_last_fetched;    /* size of last downloaded */
    uint32_t size_fetched;         /* size of already downloaded */
    uint32_t size_file;            /* size of file */
    int err;                       /* last error code */
    void *md5;                     /* MD5 handle */
    void *ch_fetch;                /* channel handle of download */
    THandlerFunction_Progress cb;  /* progress callback */
} OTA_Struct_t, *OTA_Struct_pt;

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

static void *ofc_Init(const char *url)
{
    otahttp_Struct_pt h_odc;

    if (NULL == (h_odc = HAL_Malloc(sizeof(otahttp_Struct_t)))) {
        MOLMC_LOGE(TAG, "allocate for h_odc failed");
        return NULL;
    }

    memset(h_odc, 0, sizeof(otahttp_Struct_t));
    httpclient_init(&h_odc->http);

    /* set http request-header parameter */
    h_odc->http.header = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                         "Accept-Encoding: gzip, deflate\r\n";
    h_odc->url = url;
    return h_odc;
}

static int32_t ofc_Fetch(void *handle, char *buf, uint32_t buf_len, uint32_t timeout_s)
{
    int diff;
    otahttp_Struct_pt h_odc = (otahttp_Struct_pt)handle;

    h_odc->http_data.response_buf = buf;
    h_odc->http_data.response_buf_len = buf_len;
    diff = h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len;

    if (0 != httpclient_common(&h_odc->http, h_odc->url, 80, NULL, HTTPCLIENT_GET, timeout_s * 1000,
                               &h_odc->http_data)) {
        MOLMC_LOGE(TAG, "fetch firmware failed");
        return -1;
    }

    return h_odc->http_data.response_content_len - h_odc->http_data.retrieve_len - diff;
}

static int ofc_Deinit(void *handle)
{
    if (NULL != handle) {
        HAL_Free(handle);
    }

    return 0;
}

/* Initialize OTA module */
void *IOT_OTA_Init(uint8_t fileType, const char *url, const char *md5, uint32_t size)
{
    OTA_Struct_pt h_ota = NULL;

    if ((NULL == url) || (NULL == md5)) {
        MOLMC_LOGE(TAG, "one or more parameters is invalid");
        return NULL;
    }

    MOLMC_LOGI(TAG, "type : %d", fileType);
    MOLMC_LOGI(TAG, "url  : %s", url);
    MOLMC_LOGI(TAG, "md5  : %s", md5);
    MOLMC_LOGI(TAG, "size : %d", size);

    if (NULL == (h_ota = HAL_Malloc(sizeof(OTA_Struct_t)))) {
        MOLMC_LOGE(TAG, "allocate failed");
        return NULL;
    }
    memset(h_ota, 0, sizeof(OTA_Struct_t));
    h_ota->state = IOT_OTAS_UNINITED;

    if (NULL == (h_ota->purl = HAL_Malloc(strlen(url) + 1))) {
        MOLMC_LOGE(TAG, "allocate url failed");
        goto do_exit;
    }

    if (NULL == (h_ota->pmd5 = HAL_Malloc(strlen(md5) + 1))) {
        MOLMC_LOGE(TAG, "allocate md5 failed");
        goto do_exit;
    }
    strcpy(h_ota->purl, url);
    strcpy(h_ota->pmd5, md5);
    h_ota->size_file = size;
    h_ota->md5 = otalib_MD5Init();
    if (NULL == h_ota->md5) {
        MOLMC_LOGE(TAG, "initialize md5 failed");
        goto do_exit;
    }

    if (NULL == (h_ota->ch_fetch = ofc_Init(h_ota->purl))) {
        MOLMC_LOGE(TAG, "Initialize fetch module failed");
        goto do_exit;
    }

    h_ota->state = IOT_OTAS_INITED;
    return h_ota;

do_exit:

    if (NULL != h_ota->purl) {
        HAL_Free(h_ota->purl);
    }

    if (NULL != h_ota->pmd5) {
        HAL_Free(h_ota->pmd5);
    }

    if (NULL != h_ota->md5) {
        otalib_MD5Deinit(h_ota->md5);
    }

    if (NULL != h_ota->ch_fetch) {
        ofc_Deinit(h_ota->ch_fetch);
    }

    if (NULL != h_ota) {
        HAL_Free(h_ota);
    }

    return NULL;
}

/* deinitialize OTA module */
int IOT_OTA_Deinit(void *handle)
{
    OTA_Struct_pt h_ota = (OTA_Struct_pt) handle;

    if (NULL == h_ota) {
        MOLMC_LOGE(TAG, "handle is NULL");
        return IOT_OTAE_INVALID_PARAM;
    }

    if (IOT_OTAS_UNINITED == h_ota->state) {
        MOLMC_LOGE(TAG, "handle is uninitialized");
        h_ota->err = IOT_OTAE_INVALID_STATE;
        return -1;
    }

    HAL_Free(h_ota->purl);
    HAL_Free(h_ota->pmd5);
    otalib_MD5Deinit(h_ota->md5);
    ofc_Deinit(h_ota->ch_fetch);
    HAL_Free(h_ota);
    return 0;
}

int IOT_OTA_SetProgressCallback(void *handle, THandlerFunction_Progress fn)
{
    OTA_Struct_pt h_ota = (OTA_Struct_pt) handle;

    if (NULL == h_ota) {
        MOLMC_LOGE(TAG, "handle is NULL");
        return IOT_OTAE_INVALID_PARAM;
    }

    if (IOT_OTAS_UNINITED == h_ota->state) {
        MOLMC_LOGE(TAG, "handle is uninitialized");
        h_ota->err = IOT_OTAE_INVALID_STATE;
        return -1;
    }

    h_ota->cb = fn;
    return 0;
}

bool IOT_OTA_Update(void *handle)
{
#define OTA_BUF_LEN        (1500)
    int ret;
    int ok = false;
    char buf[OTA_BUF_LEN];
    OTA_Struct_pt h_ota = (OTA_Struct_pt) handle;

    if (NULL == handle) {
        MOLMC_LOGE(TAG, "invalid parameter");
        goto do_exit;
    }

    do {
        ret = ofc_Fetch(h_ota->ch_fetch, buf, OTA_BUF_LEN, 1);
        if (ret < 0) {
            MOLMC_LOGE(TAG, "Fetch firmware failed");
            h_ota->state = IOT_OTAS_FETCHED;
            h_ota->err = IOT_OTAE_FETCH_FAILED;
            break;
        } else if (0 == h_ota->size_fetched) {
            /* force report status in the first */
            h_ota->cb(h_ota, NULL, 0, 0, h_ota->size_file);
        }

        h_ota->size_last_fetched = ret;
        h_ota->size_fetched += ret;

        otalib_MD5Update(h_ota->md5, buf, ret);

        h_ota->cb(h_ota, (uint8_t *)buf, ret, h_ota->size_fetched, h_ota->size_file);
        if (h_ota->size_fetched >= h_ota->size_file) {
            char md5_str[33];
            otalib_MD5Finalize(h_ota->md5, md5_str);
            MOLMC_LOGD(TAG, "origin=%s, now=%s", h_ota->pmd5, md5_str);
            if (0 == strcmp(h_ota->pmd5, md5_str)) {
                ok = true;
                h_ota->err = IOT_OTAE_NONE;
            } else {
                h_ota->err = IOT_OTAE_CHECK_FAILED;
            }
            h_ota->state = IOT_OTAS_FETCHED;
            break;
        }
        IOT_Comm_Yield();
    } while(1);

do_exit:
    if(ok) {
        IOT_OTA_ReportProgress(h_ota, IOTX_OTA_REPLY_FETCH_SUCCESS, 0);
    } else {
        IOT_OTA_ReportProgress(h_ota, IOTX_OTA_REPLY_FETCH_FAILED, 0);
    }
    return ok;
#undef OTA_BUF_LEN
}

int IOT_OTA_ReportProgress(void *handle, iotx_ota_reply_t reply, uint8_t progress)
{
    OTA_Struct_pt h_ota = (OTA_Struct_pt) handle;

    if (NULL == h_ota) {
        MOLMC_LOGE(TAG, "handle is NULL");
        return IOT_OTAE_INVALID_PARAM;
    }

    if (IOT_OTAS_UNINITED == h_ota->state) {
        MOLMC_LOGE(TAG, "handle is uninitialized");
        h_ota->err = IOT_OTAE_INVALID_STATE;
        return -1;
    }

    return IOT_Comm_SendActionReply(h_ota->type_file, reply, progress);
}

/* Get last error code */
int IOT_OTA_GetLastError(void *handle)
{
    OTA_Struct_pt h_ota = (OTA_Struct_pt) handle;

    if (NULL == handle) {
        MOLMC_LOGE(TAG, "handle is NULL");
        return  IOT_OTAE_INVALID_PARAM;
    }

    return h_ota->err;
}

