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

#include <time.h>
#include <reent.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "hal_import.h"

void mygettimeofday(struct timeval *tv, void *tz)
{
    struct _reent r;
    _gettimeofday_r(&r, tv, tz);
}

void *HAL_MutexCreate(void)
{
    return xSemaphoreCreateMutex();
}

void HAL_MutexDestroy(void *mutex)
{
    vSemaphoreDelete(mutex);
}

void HAL_MutexLock(void *mutex)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
}

void HAL_MutexUnlock(void *mutex)
{
    xSemaphoreGive(mutex);
}

void *HAL_Malloc(uint32_t size)
{
    return malloc(size);
}

void HAL_Free(void *ptr)
{
    return free(ptr);
}

void HAL_SystemReboot(void)
{

}

uint32_t HAL_UptimeMs(void)
{
    struct timeval tv = { 0 };
    uint32_t time_ms;

    mygettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (rand() % region) : 0;
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

