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
#include "iotx_key_api.h"

#if CONFIG_SYSTEM_KEY_ENABLE == 1

static iotx_key_param_t keyParams = {0,20,600,1000,1,0,false,0,0};
static uint8_t keyNumRecord = 0;  //记录按下了的按键编号
static iotx_key_t *keyListHead = NULL;
static bool keyListInitFlag = false;

static void KeyListInit(void)
{
    keyListHead = (iotx_key_t*)malloc(sizeof(iotx_key_t));
    if(keyListHead == NULL) {
        return;
    }
    keyListHead->next = NULL;
}

//该按键是否存在
static bool KeyExists( iotx_key_t *obj )
{
    iotx_key_t* cur = keyListHead->next;

    while( cur != NULL ) {
        if(cur->keyNum == obj->keyNum) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

//按键插入
static void KeyListInsert(iotx_key_t *obj)
{
    iotx_key_t *head = keyListHead;
    iotx_key_t *cur = keyListHead->next;

    if(( obj == NULL ) || ( KeyExists( obj ) == true )){
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

static bool KeyListQueryDoubleClickCb(uint8_t num)
{
    iotx_key_t *cur = keyListHead->next;
    while(cur != NULL) {
        if( cur->keyNum == num) {
            if(cur->cbKeyDoubleClickFunc != NULL) {
                return true;
            }
        }
        cur = cur->next;
    }
    return false;
}

//查找按键 注册按键回调
static void KeyListRegisterCbFunc(uint8_t cbType, uint8_t num, cbPressFunc cbFunc, cbClickFunc clickFunc)
{
    iotx_key_t *cur = keyListHead->next;
    while(cur != NULL) {
        if( cur->keyNum == num) {
            switch(cbType){
                case KEY_CLICK_CB:
                    cur->cbKeyClickFunc = clickFunc;
                    return;
                case KEY_DOUBLE_CLICK_CB:
                    cur->cbKeyDoubleClickFunc = clickFunc;
                    return;
                case KEY_PRESS_SATRT_CB:
                    cur->cbKeyPressStartFunc = cbFunc;
                    return;
                case KEY_PRESS_STOP_CB:
                    cur->cbKeyPressStopFunc = cbFunc;
                    return;
                case KEY_PRESS_DURING_CB:
                    cur->cbKeyPressDuringFunc = cbFunc;
                    return;
                default:
                    return;
            }
        }
        cur = cur->next;
    }
}

//执行按键回调函数
static void KeyListExeCbFunc(uint8_t cbType, uint8_t num, uint32_t ms)
{
    iotx_key_t *cur = keyListHead->next;
    while(cur != NULL) {
        if( cur->keyNum == num) {
            switch(cbType){
                case KEY_CLICK_CB:
                    if(cur->cbKeyClickFunc != NULL) {
                        cur->cbKeyClickFunc();
                    }
                    break;
                case KEY_DOUBLE_CLICK_CB:
                    if(cur->cbKeyDoubleClickFunc != NULL) {
                        cur->cbKeyDoubleClickFunc();
                    }
                case KEY_PRESS_SATRT_CB:
                    if(cur->cbKeyPressStartFunc != NULL) {
                        cur->cbKeyPressStartFunc(ms);
                    }
                    break;
                case KEY_PRESS_STOP_CB:
                    if(cur->cbKeyPressStopFunc != NULL) {
                        cur->cbKeyPressStopFunc(ms);
                    }
                    break;
                case KEY_PRESS_DURING_CB:
                    if(cur->cbKeyPressDuringFunc != NULL) {
                        cur->cbKeyPressDuringFunc(ms);
                    }
                    break;
                default:
                    break;
            }
        }
        cur = cur->next;
    }
}

//获取键值
static int KeyListGetValue(void)
{
    iotx_key_t *cur = keyListHead->next;
    while(cur != NULL) {
        if(cur->cbKeyGetValueFunc() == keyParams._buttonPressed) {
            keyNumRecord = cur->keyNum; //记录哪个按键按下了
            return keyParams._buttonPressed;
        }
        cur = cur->next;
    }
    return keyParams._buttonReleased;
}

void IOT_KEY_Init(void)
{
    iotx_key_t* cur = keyListHead->next;

    while( cur != NULL ) {
        if(cur->cbKeyInitFunc != NULL) {
            cur->cbKeyInitFunc();
        }
        cur = cur->next;
    }
}

void IOT_KEY_SetParams(bool invert, uint32_t debounceTime, uint32_t clickTime, uint32_t pressTime)
{
    if(!invert) {
        keyParams._buttonPressed = 0;
        keyParams._buttonReleased = 1;
    } else {
        keyParams._buttonPressed = 1;
        keyParams._buttonReleased = 0;
    }
    keyParams._debounceTime = debounceTime;
    keyParams._clickTime = clickTime;
    keyParams._pressTime = pressTime;
    keyParams._state = 0;
    keyParams._startTime = 0;
    keyParams._stopTime = 0;
}

void IOT_KEY_Register(uint8_t num, cbInitFunc initFunc, cbGetValueFunc getValFunc)
{
    if(!keyListInitFlag) {
        keyListInitFlag = true;
        KeyListInit();
    }

    iotx_key_t *p = (iotx_key_t*)malloc(sizeof(iotx_key_t));
    if(p == NULL) {
        return;
    }
    p->keyNum= num;
    p->cbKeyInitFunc = initFunc;
    p->cbKeyGetValueFunc = getValFunc;
    p->cbKeyClickFunc = NULL;
    p->cbKeyDoubleClickFunc = NULL;
    p->cbKeyPressStartFunc = NULL;
    p->cbKeyPressStopFunc = NULL;
    p->cbKeyPressDuringFunc = NULL;
    p->next = NULL;

    KeyListInsert(p);
}

//单击
void IOT_KEY_ClickCb(uint8_t num, cbClickFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_CLICK_CB, num, NULL, cbFunc);
}

//双击
void IOT_KEY_DoubleClickCb(uint8_t num, cbClickFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_DOUBLE_CLICK_CB, num, NULL, cbFunc);
}

//长按键开始按下
void IOT_KEY_PressStartCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_SATRT_CB, num, cbFunc, NULL);
}

//长按键松开
void IOT_KEY_PressStopCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_STOP_CB, num, cbFunc, NULL);
}

//按键一直长按
void IOT_KEY_PressDuringCb(uint8_t num, cbPressFunc cbFunc)
{
    KeyListRegisterCbFunc(KEY_PRESS_DURING_CB, num, cbFunc, NULL);
}

void IOT_KEY_Loop(void)
{
    // Detect the input information
    int buttonLevel = KeyListGetValue(); // current button signal.
    uint32_t now = HAL_UptimeMs(); // current (relative) time in msecs.

    // Implementation of the state machine
    if (keyParams._state == 0) {  // waiting for menu pin being pressed.
        if (buttonLevel == keyParams._buttonPressed) {
            keyParams._state = 1; // step to state 1
            keyParams._startTime = now; // remember starting time
        }
    } else if (keyParams._state == 1) { // waiting for menu pin being released.
        if ((buttonLevel == keyParams._buttonReleased) && ((uint32_t)(now - keyParams._startTime) < keyParams._debounceTime)) {
            // button was released to quickly so I assume some debouncing.
            // go back to state 0 without calling a function.
            keyParams._state = 0;
        } else if (buttonLevel == keyParams._buttonReleased) {
            keyParams._state = 2; // step to state 2
            keyParams._stopTime = now; // remember stopping time
        } else if ((buttonLevel == keyParams._buttonPressed) && ((uint32_t)(now - keyParams._startTime) > keyParams._pressTime)) {
            keyParams._isLongPressed = true;  // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_SATRT_CB, keyNumRecord, 0);
            KeyListExeCbFunc(KEY_PRESS_DURING_CB, keyNumRecord, now - keyParams._startTime);
            keyParams._state = 6; // step to state 6
        } else {
            // wait. Stay in this state.
        }
    } else if (keyParams._state == 2) { // waiting for menu pin being pressed the second time or timeout.
        if (KeyListQueryDoubleClickCb(keyNumRecord) == false || (uint32_t)(now - keyParams._startTime) > keyParams._clickTime) {
            // this was only a single short click
            KeyListExeCbFunc(KEY_CLICK_CB, keyNumRecord, 0);
            keyParams._state = 0; // restart.
        } else if ((buttonLevel == keyParams._buttonPressed) && ((uint32_t)(now - keyParams._stopTime) > keyParams._debounceTime)) {
            keyParams._state = 3; // step to state 3
            keyParams._startTime = now; // remember starting time
        }
    } else if (keyParams._state == 3) { // waiting for menu pin being released finally.
        // Stay here for at least keyParams._debounceTime because else we might end up in state 1 if the
        // button bounces for too long.
        if (buttonLevel == keyParams._buttonReleased && ((uint32_t)(now - keyParams._startTime) > keyParams._debounceTime)) {
            // this was a 2 click sequence.
            KeyListExeCbFunc(KEY_DOUBLE_CLICK_CB, keyNumRecord, 0);
            keyParams._state = 0; // restart.
        }
    } else if (keyParams._state == 6) { // waiting for menu pin being release after long press.
        if (buttonLevel == keyParams._buttonReleased) {
            keyParams._isLongPressed = false;  // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_STOP_CB, keyNumRecord, now - keyParams._startTime);
            keyParams._state = 0; // restart.
        } else {
            // button is being long pressed
            keyParams._isLongPressed = true; // Keep track of long press state
            KeyListExeCbFunc(KEY_PRESS_DURING_CB, keyNumRecord, now - keyParams._startTime);
        }
    }
}

#endif

