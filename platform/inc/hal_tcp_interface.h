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

#ifndef __HAL_TCP_INTERFACE_H__
#define __HAL_TCP_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_import.h"

/** @} */ /* end of group_platform_other */

/**
 * @brief Establish a TCP connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the TCP server
 * @param [in] port: @n Specify the TCP port of TCP server
 *
 * @return 0, fail; > 0, success, the value is handle of this TCP connection.
 */
intptr_t HAL_TCP_Establish(const char *host, uint16_t port);


/**
 * @brief Destroy the specific TCP connection.
 *
 * @param [in] fd: @n Specify the TCP connection by handle.
 *
 * @return < 0, fail; 0, success.
 */
int32_t HAL_TCP_Destroy(intptr_t fd);


/**
 * @brief Write data into the specific TCP connection.
 *        The API will return immediately if @len be written into the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
        < 0 : TCP connection error occur..
          0 : No any data be write into the TCP connection in @timeout_ms timeout period.
   (0, len] : The total number of bytes be written in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t HAL_TCP_Write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);


/**
 * @brief Read data from the specific TCP connection with timeout parameter.
 *        The API will return immediately if @len be received from the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a TCP connection.
 * @param [in] buf @n A pointer to a buffer to receive incoming data.
 * @param [in] len @n The length, in bytes, of the data pointed to by the @buf parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block @timeout_ms millisecond maximumly.
 * @return
   @verbatim
         -2 : TCP connection error occur.
         -1 : TCP connection be closed by remote server.
          0 : No any data be received in @timeout_ms timeout period.
   (0, len] : The total number of bytes be received in @timeout_ms timeout period.
   @endverbatim
 * @see None.
 */
int32_t HAL_TCP_Read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif

