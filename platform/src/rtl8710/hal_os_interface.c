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

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "device.h"

#include "hal_import.h"
#include "iotx_log_api.h"

typedef xSemaphoreHandle osi_mutex_t;

void *HAL_MutexCreate(void)
{
    osi_mutex_t *p_mutex = NULL;
    p_mutex = (osi_mutex_t *)pvPortMalloc(sizeof(osi_mutex_t));
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
	return pvPortMalloc(size);
}

void HAL_Free(void *ptr)
{
	vPortFree(ptr);
}

void HAL_SystemInit(void)
{

}

void HAL_SystemReboot(void)
{
	CPU_ClkSet(CLK_31_25M);
	osDelay(100);

	/* CPU reset: Cortex-M3 SCB->AIRCR*/
	NVIC_SystemReset();
}

uint32_t HAL_UptimeMs(void)
{
    return (__get_IPSR() == 0) ? xTaskGetTickCount() : xTaskGetTickCountFromISR();
}

void HAL_Srandom(uint32_t seed)
{
    srand(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return rand();
}

void HAL_Print(const char * output)
{
    printf("%s", output);
}

