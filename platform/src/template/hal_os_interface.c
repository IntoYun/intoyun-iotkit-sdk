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

void *HAL_MutexCreate(void)
{
    return NULL;
}

void HAL_MutexDestroy(void *mutex)
{
}

void HAL_MutexLock(void *mutex)
{
}

void HAL_MutexUnlock(void *mutex)
{
}

void *HAL_Malloc(uint32_t size)
{
    return NULL;
}

void HAL_Free(void *ptr)
{
}

void HAL_SystemReboot(void)
{
}

uint32_t HAL_UptimeMs(void)
{
    return 0;
}

void HAL_Srandom(uint32_t seed)
{
}

uint32_t HAL_Random(uint32_t region)
{
    return 0;
}

void HAL_Print(const char * output)
{
}

