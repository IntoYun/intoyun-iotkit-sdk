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

#include <stdint.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_timer.h"
#include "esp_system.h"

#include "hal_import.h"

#define MOLMC_LOGD(tag, format, ...) do { \
        printf("D [%010u]:[%-12.12s]: "format"\n", HAL_UptimeMs(), tag, ##__VA_ARGS__);\
    } while(0)

typedef xSemaphoreHandle osi_mutex_t;

/**
 * @brief  gain millisecond time
 * gettimeofday in libcirom.a cannot get a accuracy time, redefine it and for time calculation
 *
 * */
void mygettimeofday(struct timeval *tv, void *tz)
{
    uint32_t current_time_us = system_get_time();

    if (tv == NULL) {
        return;
    }

    if (tz != NULL) {
        tv->tv_sec = *(time_t *)tz + current_time_us / 1000000;
    } else {
        tv->tv_sec = current_time_us / 1000000;
    }

    tv->tv_usec = current_time_us % 1000000;
}

void *HAL_MutexCreate(void)
{
    osi_mutex_t *p_mutex = NULL;
    p_mutex = (osi_mutex_t *)malloc(sizeof(osi_mutex_t));
    if(p_mutex == NULL)
        return NULL;

    *p_mutex = xSemaphoreCreateMutex();
    return p_mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    vSemaphoreDelete(*((osi_mutex_t*)mutex));
    free(mutex);
}

void HAL_MutexLock(void *mutex)
{
    xSemaphoreTake(*((osi_mutex_t*)mutex), portMAX_DELAY);
}

void HAL_MutexUnlock(void *mutex)
{
    xSemaphoreGive(*((osi_mutex_t*)mutex));
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
    return os_random();
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

