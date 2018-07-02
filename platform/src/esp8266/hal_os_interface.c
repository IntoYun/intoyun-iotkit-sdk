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

static os_timer_t hal_micros_overflow_timer;
static uint32_t hal_micros_at_last_overflow_tick = 0;
static uint32_t hal_micros_overflow_count = 0;

static void hal_micros_overflow_tick(void *arg)
{
    uint32_t m = system_get_time();

    if (m < hal_micros_at_last_overflow_tick) {
        hal_micros_overflow_count ++;
    }

    hal_micros_at_last_overflow_tick = m;
}

static void hal_micros_set_default_time(void)
{
    os_timer_disarm(&hal_micros_overflow_timer);
    os_timer_setfn(&hal_micros_overflow_timer, (os_timer_func_t *)hal_micros_overflow_tick, 0);
    os_timer_arm(&hal_micros_overflow_timer, 60 * 1000, 1);
}

static uint32_t hal_millis(void)
{
    uint32_t m = system_get_time();
    uint32_t c = hal_micros_overflow_count + ((m < hal_micros_at_last_overflow_tick) ? 1 : 0);
    return c * 4294967LL + m / 1000;
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

void HAL_SystemInit(void)
{
    hal_micros_set_default_time();
}

void HAL_SystemReboot(void)
{

}

uint32_t HAL_UptimeMs(void)
{
    return hal_millis();
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

