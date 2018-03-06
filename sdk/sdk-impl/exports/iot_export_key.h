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


#ifndef __IOT_EXPORT_KEY_H__
#define __IOT_EXPORT_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "iotx_key_api.h"

typedef struct {
    void (*init)(void);
    void (*setParams)(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime);
    void (*keyRegister)(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc);
    void (*attachClick)(uint8_t num, cbClickFunc cbFunc);           //注册单击处理函数
    void (*attachDoubleClick)(uint8_t num, cbClickFunc cbFunc);     //注册双击处理函数
    void (*attachLongPressStart)(uint8_t num, cbPressFunc cbFunc);  //注册按下按键处理函数
    void (*attachLongPressStop)(uint8_t num, cbPressFunc cbFunc);   //注册释放按键处理函数
    void (*attachDuringLongPress)(uint8_t num, cbPressFunc cbFunc); //注册按键按下回调函数
    void (*loop)(void);
} iot_keys_if_t;

extern const iot_keys_if_t Key;

#ifdef __cplusplus
}
#endif

#endif

