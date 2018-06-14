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

intptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    return 0;
}

int HAL_TCP_Destroy(intptr_t fd)
{
    return 0;
}

int32_t HAL_TCP_Write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    return 0;
}

int32_t HAL_TCP_Read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    return 0;
}

