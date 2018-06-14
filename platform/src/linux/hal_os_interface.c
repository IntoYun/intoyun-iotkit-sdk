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
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "hal_import.h"

#define MOLMC_LOGD(tag, format, ...) do { \
        printf("D [%010u]:[%-12.12s]: "format"\n", HAL_UptimeMs(), tag, ##__VA_ARGS__);\
    } while(0)

const static char *TAG = "hal:os";

void *HAL_MutexCreate(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        MOLMC_LOGD(TAG, "create mutex failed");
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        MOLMC_LOGD(TAG, "destroy mutex failed");
    }

    HAL_Free(mutex);
}

void HAL_MutexLock(void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        MOLMC_LOGD(TAG, "lock mutex failed");
    }
}

void HAL_MutexUnlock(void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        MOLMC_LOGD(TAG, "unlock mutex failed");
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
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void HAL_Srandom(uint32_t seed)
{
    srandom(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (random() % region) : 0;
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

