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

#ifndef __HAL_TLS_INTERFACE_H__
#define __HAL_TLS_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "hal_import.h"

/**
 * @brief Establish a SSL connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the SSL server
 * @param [in] port: @n Specify the SSL port of SSL server
 * @param [in] ca_crt @n Specify the root certificate which is PEM format.
 * @param [in] ca_crt_len @n Length of root certificate, in bytes.
 * @return SSL handle.
 * @see None.
 * @note None.
 */
intptr_t HAL_SSL_Establish(const char *host, uint16_t port, const char *ca_crt, size_t ca_crt_len);


/**
 * @brief Destroy the specific SSL connection.
 *
 * @param[in] handle: @n Handle of the specific connection.
 *
 * @return < 0, fail; 0, success.
 */
int32_t HAL_SSL_Destroy(intptr_t handle);


/**
 * @brief Write data into the specific SSL connection.
 *        The API will return immediately if @len be written into the specific SSL connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
        < 0 : SSL connection error occur..
          0 : No any data be write into the SSL connection in @timeout_ms timeout period.
   (0, len] : The total number of bytes be written in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t HAL_SSL_Write(intptr_t handle, const char *buf, int len, int timeout_ms);


/**
 * @brief Read data from the specific SSL connection with timeout parameter.
 *        The API will return immediately if @len be received from the specific SSL connection.
 *
 * @param [in] fd @n A descriptor identifying a SSL connection.
 * @param [in] buf @n A pointer to a buffer to receive incoming data.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
         -2 : SSL connection error occur.
         -1 : SSL connection be closed by remote server.
          0 : No any data be received in @timeout_ms timeout period.
   (0, len] : The total number of bytes be received in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t HAL_SSL_Read(intptr_t handle, char *buf, int len, int timeout_ms);

#ifdef __cplusplus
}
#endif

#endif

