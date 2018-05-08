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

#include "iot_export.h"

const static char *TAG = "user:ota";

static void otaProgressCallback(void *handle, uint8_t *data, size_t len, uint32_t currentSize, uint32_t totalSize)
{
    static uint8_t last_percent = 0;
    uint8_t percent = 0;

    MOLMC_LOGI(TAG, "otaProgressCallback len = %d, currentSize = %d, totalSize = %d", len, currentSize, totalSize);
    //烧写FLASH

    //上送OTA进度
    last_percent = percent;
    percent = (currentSize * 100) / totalSize;
    if (percent - last_percent > 0) {
        OTAUpdate.reportProgress(handle, IOTX_OTA_REPLY_PROGRESS, percent);
    }
}

bool otaUpdate(uint8_t *data, uint32_t len)
{
    void *h_ota = NULL;
    bool ret = false;

    MOLMC_LOGI(TAG, "action data : %s\r\n", data);
    cJSON *actionJson=cJSON_Parse((char *)data);
    if(NULL == actionJson) {
        goto do_exit;
    }

    cJSON *cmdObject = cJSON_GetObjectItem(actionJson, "cmd");
    if(NULL == cmdObject) {
        goto do_exit;
    }

    if(!strcmp(cmdObject->valuestring, "upgradeApp")) {
        cJSON *typeObject = cJSON_GetObjectItem(actionJson, "type");
        if(NULL == typeObject) {
            goto do_exit;
        }

        cJSON *urlObject = cJSON_GetObjectItem(actionJson, "url");
        if(NULL == typeObject) {
            goto do_exit;
        }

        cJSON *md5Object = cJSON_GetObjectItem(actionJson, "md5");
        if(NULL == typeObject) {
            goto do_exit;
        }

        cJSON *sizeObject = cJSON_GetObjectItem(actionJson, "size");
        if(NULL == typeObject) {
            goto do_exit;
        }

        void *h_ota = OTAUpdate.init(atoi(typeObject->valuestring), urlObject->valuestring, md5Object->valuestring, sizeObject->valueint);
        OTAUpdate.setProgressCallback(h_ota, &otaProgressCallback);
        if(OTAUpdate.update(h_ota)) {
            ret = true;
        }
    }

do_exit:
    if(NULL != actionJson) {
        cJSON_Delete(actionJson);
    }

    if(NULL != h_ota) {
        OTAUpdate.deInit(h_ota);
    }
    return ret;
}

