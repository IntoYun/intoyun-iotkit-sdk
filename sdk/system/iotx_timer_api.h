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


#ifndef __IOTX_TIMER_API_H__
#define __IOTX_TIMER_API_H__

#include "sdk_config.h"

#if CONFIG_SYSTEM_TIMER_ENABLE == 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef void (*cbTimerFunc)(void);

typedef struct timer_param_s{
    uint8_t     timerNum;    //定时编号
    uint32_t    period;      //定时周期
    bool        oneShot;     //true只执行一次
    bool        start;       //开始启动
    uint32_t    timerTick;   //定时计数
    cbTimerFunc timerCbFunc; //定时回调
    struct timer_param_s *next;
} iotx_timer_t;

void IOT_TIMER_Register(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc);
void IOT_TIMER_ChangePeriod(uint8_t num, uint32_t period);
void IOT_TIMER_Start(uint8_t num);
void IOT_TIMER_Stop(uint8_t num);
void IOT_TIMER_Reset(uint8_t num);
void IOT_TIMER_Loop(void);

#ifdef __cplusplus
}
#endif

#endif
#endif

