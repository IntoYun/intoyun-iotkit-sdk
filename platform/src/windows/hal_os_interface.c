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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#include "hal_import.h"

#include <process.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>

#define PLATFORM_WINOS_PERROR printf

void *HAL_MutexCreate(void)
{
    HANDLE mutex;

    if (NULL == (mutex = CreateMutex(NULL, FALSE, NULL))) {
        PLATFORM_WINOS_PERROR("create mutex error");
    }

    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    if (0 == CloseHandle(mutex)) {
        PLATFORM_WINOS_PERROR("destroy mutex error");
    }
}

void HAL_MutexLock(void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        PLATFORM_WINOS_PERROR("lock mutex error");
    }
}

void HAL_MutexUnlock(void *mutex)
{
    if (WAIT_FAILED == WaitForSingleObject(mutex, INFINITE)) {
        PLATFORM_WINOS_PERROR("lock mutex error");
    }
}

void *HAL_Malloc(uint32_t size)
{
    return malloc(size);
}

void HAL_Free(void *ptr)
{
    free(ptr);
}

void HAL_SystemReboot(void)
{

}

uint32_t HAL_UptimeMs(void)
{
    return (uint32_t)(GetTickCount());
}

void HAL_SleepMs(uint32_t ms)
{
    Sleep(ms);
}

uint32_t orig_seed = 2;
void HAL_Srandom(uint32_t seed)
{
    orig_seed = seed;
}

uint32_t HAL_Random(uint32_t region)
{
    orig_seed = 1664525 * orig_seed + 1013904223;
    return (region > 0) ? (orig_seed % region) : 0;
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

