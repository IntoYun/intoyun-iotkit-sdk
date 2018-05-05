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
#include <pthread.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "lwip/sockets.h"
#include "hal_import.h"

static os_timer_t hal_micros_overflow_timer;
static uint32 hal_micros_at_last_overflow_tick = 0;
static uint32 hal_micros_overflow_count = 0;

static void hal_micros_overflow_tick(void *arg)
{
    uint32 m = system_get_time();

    if (m < hal_micros_at_last_overflow_tick) {
        hal_micros_overflow_count ++;
    }

    hal_micros_at_last_overflow_tick = m;
}

void hal_micros_set_default_time(void)
{
    os_timer_disarm(&hal_micros_overflow_timer);
    os_timer_setfn(&hal_micros_overflow_timer, (os_timer_func_t *)hal_micros_overflow_tick, 0);
    os_timer_arm(&hal_micros_overflow_timer, 60 * 1000, 1);
}

unsigned long hal_millis(void)
{
    uint32 m = system_get_time();
    uint32 c = hal_micros_overflow_count + ((m < hal_micros_at_last_overflow_tick) ? 1 : 0);
    return c * 4294967 + m / 1000;
}

void mygettimeofday(struct timeval *tv, void *tz);

/**
 * @brief  gain millisecond time
 * gettimeofday in libcirom.a cannot get a accuracy time, redefine it and for time calculation
 *
 * */
void mygettimeofday(struct timeval *tv, void *tz)
{
    uint32 current_time_us = system_get_time();

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
    return hal_millis();
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (rand() % region) : 0;
}

void HAL_SleepMs(uint32_t ms)
{
    if ((ms > 0) && (ms < portTICK_RATE_MS)) {
        ms = portTICK_RATE_MS;
    }
    vTaskDelay(ms/portTICK_RATE_MS);
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

