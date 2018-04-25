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

#ifndef __IOTX_KEY_API_H__
#define __IOTX_KEY_API_H__

#include "sdk_config.h"

#if CONFIG_SYSTEM_KEY_ENABLE == 1

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

typedef void (*cbInitFunc)(void);          //按键初始化回调函数
typedef uint16_t (*cbGetValueFunc)(void);  //获取按键值回调函数
typedef void (*cbClickFunc)(void);         //单击双击回调函数
typedef void (*cbPressFunc)(uint32_t ms);  //长按回调函数 ms为长按键时间

typedef struct key_cb_s {
    uint8_t    keyNum; //按键编号
    cbInitFunc  cbKeyInitFunc; //初始化回调
    cbGetValueFunc cbKeyGetValueFunc; //获取按键电平回调
    cbClickFunc cbKeyClickFunc;
    cbClickFunc cbKeyDoubleClickFunc;
    cbPressFunc cbKeyPressStartFunc; //按键按下回调
    cbPressFunc cbKeyPressStopFunc; //按键松开回调
    cbPressFunc cbKeyPressDuringFunc; //按键长按间隔回调
    struct key_cb_s *next;
} iotx_key_t;

typedef struct key_param_s{
    uint8_t    _state;
    uint32_t   _debounceTime; //抖动时间
    uint32_t   _clickTime;  //连击时间
    uint32_t   _pressTime; //长按间隔回调时间 比如1000表示每隔1000ms回调长按键函数
    uint8_t    _buttonReleased;
    uint8_t    _buttonPressed;
    bool       _isLongPressed;
    uint32_t   _startTime;
    uint32_t   _stopTime;
} iotx_key_param_t;

enum keyCbFuncType{
    KEY_CLICK_CB        = 1,
    KEY_DOUBLE_CLICK_CB = 2,
    KEY_PRESS_SATRT_CB  = 3,
    KEY_PRESS_STOP_CB   = 4,
    KEY_PRESS_DURING_CB = 5,
};

void IOT_KEY_Init(void);
void IOT_KEY_SetParams(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime);
void IOT_KEY_Register(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc);
void IOT_KEY_ClickCb(uint8_t num, cbClickFunc cbFunc);
void IOT_KEY_DoubleClickCb(uint8_t num, cbClickFunc cbFunc);
void IOT_KEY_PressStartCb(uint8_t num, cbPressFunc cbFunc);
void IOT_KEY_PressStopCb(uint8_t num, cbPressFunc cbFunc);
void IOT_KEY_PressDuringCb(uint8_t num, cbPressFunc cbFunc);
void IOT_KEY_Loop(void);

#ifdef __cplusplus
}
#endif

#endif
#endif

