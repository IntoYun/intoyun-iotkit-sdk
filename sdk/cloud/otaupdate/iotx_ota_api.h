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

#ifndef __IOTX_OTA_API_H__
#define __IOTX_OTA_API_H__

#include "iotx_comm_if_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    IOT_OTAE_CHECK_FAILED = -5,
    IOT_OTAE_FETCH_FAILED = -4,
    IOT_OTAE_INVALID_STATE = -3,
    IOT_OTAE_INVALID_PARAM = -2,
    IOT_OTAE_GENERAL = -1,
    IOT_OTAE_NONE = 0,
} IOT_OTA_Err_t;

/* State of OTA */
typedef enum {
    IOT_OTAS_UNINITED = 0,  /* Uninitialized State */
    IOT_OTAS_INITED,        /* Initialized State */
    IOT_OTAS_FETCHING,      /* Fetching firmware */
    IOT_OTAS_FETCHED        /* Fetching firmware finish */
} IOT_OTA_State_t;


typedef void (*THandlerFunction_Progress)(void *handle, uint8_t *data, size_t len, uint32_t currentSize, uint32_t totalSize);

/** @defgroup group_api api
 *  @{
 */

/** @defgroup group_api_ota ota
 *  @{
 */

/**
 * @brief Initialize OTA module, and return handle.
 *        The MQTT client must be construct before calling this interface.
 *
 * @param [in] fileType: the type of firmmware.
 * @param [in] url: the url fetch the firmware.
 * @param [in] md5: the checkout md5 of the firmware.
 * @param [in] size: the size of the firmware.
 *
 * @return handle: specify the OTA module.
 * @see None.
 */
void *IOT_OTA_Init(uint8_t fileType, const char *url, const char *md5, uint32_t size);

/**
 * @brief Deinitialize OTA module specified by the 'handle', and release the related resource.
 *        You must call this interface to release resource if reboot is not invoked after downloading.
 *
 * @param [in] handle: specify the OTA module.
 *
 * @retval   0 : Successful.
 * @retval < 0 : Failed, the value is error code.
 * @see None.
 */
int IOT_OTA_Deinit(void *handle);

/**
 * @brief Set the ProgressCallBack.
 *        NOTE: please
 *
 * @param [in] handle: specify the OTA module.
 * @param [in] fn: specify callback function.
 *
 * @retval   0 : Successful.
 * @retval < 0 : Failed, the value is error code.
 * @see None.
 */
int IOT_OTA_SetProgressCallback(void *handle, THandlerFunction_Progress fn);

/**
 * @brief Update the firmware
 *
 * @param [in] handle: specify the OTA module.
 *
 * @retval 1 : Yes.
 * @retval 0 : No.
 * @see None.
 */
bool IOT_OTA_Update(void *handle);

/**
 * @brief Report detail progress to OTA server (optional).
 *        NOTE: please
 *
 * @param [in] handle: specify the OTA module.
 * @param [in] reply: specify the reply defined by 'iotx_ota_reply_t'.
 * @param [in] progress: specify the progress.
 *
 * @retval   0 : Successful.
 * @retval < 0 : Failed, the value is error code.
 * @see None.
 */
int IOT_OTA_ReportProgress(void *handle, iotx_ota_reply_t reply, uint8_t progress);

/**
 * @brief Get last error code.
 *
 * @param [in] handle: specify the OTA module.
 *
 * @return The error code.
 * @see None.
 */
int IOT_OTA_GetLastError(void *handle);

/** @} */ /* end of api_ota */
/** @} */ /* end of api */

#ifdef __cplusplus
}
#endif

#endif /* __IOTX_OTA_API_H__ */

