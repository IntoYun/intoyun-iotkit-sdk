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

#include "hal_import.h"


intptr_t HAL_SSL_Establish(const char *host, uint16_t port, const char *ca_crt, size_t ca_crt_len)
{
    return 0;
}

int32_t HAL_SSL_Destroy(intptr_t handle)
{
    return 0;
}

int HAL_SSL_Read(intptr_t handle, char *buf, int len, int timeout_ms)
{
    return 0;
}

int HAL_SSL_Write(intptr_t handle, const char *buf, int len, int timeout_ms)
{
    return 0;
}

