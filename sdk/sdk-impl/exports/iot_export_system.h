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

#ifndef __IOT_EXPORT_SYSTEM_H__
#define __IOT_EXPORT_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_system_api.h"
#include "iotx_timer_api.h"

typedef struct
{
    // 协议操作接口
    void (*init)(void);
    void (*loop)(void);
    //应用操作接口
    void (*setDeviceInfo)(char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion, char *deviceId, char *deviceSecret);
    void (*setEventCallback)(event_handler_t handler);
    void (*setDatapointControl)(dp_transmit_mode_t mode, uint32_t lapse);
} iot_system_if_t;

typedef struct {
    void (*timerRegister)(uint8_t num, uint32_t period, bool oneShot, cbTimerFunc cbFunc);
    void (*changePeriod)(uint8_t num, uint32_t period);
    void (*start)(uint8_t num);
    void (*stop)(uint8_t num);
    void (*reset)(uint8_t num);
    void (*loop)(void);
} iot_timers_if_t;

extern const iot_system_if_t System;
extern const iot_timers_if_t Timer;

uint32_t timerGetId(void);
bool timerIsEnd(uint32_t timerID, uint32_t time);

#ifdef __cplusplus
}
#endif

#endif

