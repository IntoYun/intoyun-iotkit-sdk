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

#include "iot_import.h"
#include "iotx_timer_api.h"

#if CONFIG_SYSTEM_TIMER_ENABLE == 1

static iotx_timer_t *timerListHead = NULL;
static bool timerListInitFlag = false;

static void TimerListInit(void)
{
    timerListHead = (iotx_timer_t*)malloc(sizeof(iotx_timer_t));
    if(timerListHead == NULL) {
        return;
    }
    timerListHead->next = NULL;
}

//该按键是否存在
static bool TimerExists( iotx_timer_t *obj )
{
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL ) {
        if( cur->timerNum == obj->timerNum ) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

//按键插入
static void TimerListInsert(iotx_timer_t *obj)
{
    iotx_timer_t *head = timerListHead;
    iotx_timer_t *cur = timerListHead->next;

    if( ( obj == NULL ) || ( TimerExists( obj ) == true ) ) {
        //log_v("timer is exists\r\n");
        return;
    }

    if(cur == NULL) {
        head->next = obj;
        obj->next = NULL;
    } else {
        while((cur->next != NULL)) {
            cur = cur->next;
        }
        cur->next = obj;
        obj->next = NULL;
    }
}

static void TimerListChangPeriod(uint8_t num, uint32_t period)
{
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL ) {
        if( cur->timerNum == num ) {
            cur->period = period;
        }
        cur = cur->next;
    }
}

static void TimerListTimerStart(uint8_t num)
{
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL ) {
        if( cur->timerNum == num ) {
            cur->start = true;
        }
        cur = cur->next;
    }
}

static void TimerListTimerStop(uint8_t num)
{
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL ) {
        if( cur->timerNum == num ) {
            cur->start = false;
        }
        cur = cur->next;
    }
}

static void TimerListTimerReset(uint8_t num)
{
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL ) {
        if( cur->timerNum == num ) {
            cur->timerTick = 0;
        }
        cur = cur->next;
    }
}

static void TimerListLoop(void)
{
    iotx_timer_t* head = timerListHead;
    iotx_timer_t* cur = timerListHead->next;

    while( cur != NULL && head != NULL) {
        if(cur->start) {
            if(++cur->timerTick >= cur->period) {
                cur->timerTick = 0;
                if(cur->timerCbFunc != NULL) {
                    cur->timerCbFunc();
                }

                if(cur->oneShot) {
                    //只执行一次
                    cur->start = false;
                }
            }
        }
        cur = cur->next;
    }
}

void IOT_TIMER_Register(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc)
{
    if(!timerListInitFlag) {
        timerListInitFlag = true;
        TimerListInit();
    }

    iotx_timer_t *p = (iotx_timer_t*)malloc(sizeof(iotx_timer_t));
    if(p == NULL) {
        //log_v("error malloc\r\n");
        return;
    }
    p->timerNum= num;
    p->period= period;
    p->oneShot = oneShot;
    p->start = false;
    p->timerTick = 0;
    p->timerCbFunc = cbFunc;
    p->next = NULL;

    //log_v("timerNum=%d\r\n",p->timerNum);
    TimerListInsert(p);
}

void IOT_TIMER_ChangePeriod(uint8_t num, uint32_t period)
{
    TimerListChangPeriod(num, period);
}

void IOT_TIMER_Start(uint8_t num)
{
    TimerListTimerStart(num);
}

void IOT_TIMER_Stop(uint8_t num)
{
    TimerListTimerStop(num);
}

void IOT_TIMER_Reset(uint8_t num)
{
    TimerListTimerReset(num);
}

void IOT_TIMER_Loop(void)
{
    TimerListLoop();
}

#endif

