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

#ifndef __SDK_CONFIG_H__
#define __SDK_CONFIG_H__

#define SDK_VERSION                        "1.3.0"

#define CONFIG_CLOUD_SELECT                1   //选择平台类型  1: 生产平台  2: 测试平台

#define CONFIG_CLOUD_CHANNEL               1   //选择云端通讯通道  1: MQTT  2: COAP
#define CONFIG_CLOUD_DATAPOINT_ENABLED     1   //是否使能数据点通讯接口

// 数据点相关配置
#define CONFIG_PROPERTIES_MAX              50  //支持的数据点的最大个数
#define CONFIG_AUTOMATIC_INTERVAL          600 //数据点自动发送默认时间间隔

// 云端通讯安全模式
// 通道 + 认证 + 数据组织加密方式
// 1. TCP + Guider + formCrypto
// 2. TCP + Guider + formPlain
// 3. TLS + Guider + Plain
#define CONFIG_CLOUD_SECURE_MODE           1   //云端通讯安全模式

#define CONFIG_MQTT_ENABLED                1   //是否使能MQTT功能
#define CONFIG_COAP_ENABLED                1   //是否使能COAP功能
#define CONFIG_SYSTEM_KEY_ENABLE           0   //是否按键接口功能
#define CONFIG_SYSTEM_TIMER_ENABLE         0   //是否定时器接口功能
#define CONFIG_LOG_ENABLE                  1   //是否使能日志功能

//MOLMC_LOG_NONE,       /*!< No log output */
//MOLMC_LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
//MOLMC_LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
//MOLMC_LOG_INFO,       /*!< Information messages which describe normal flow of events */
//MOLMC_LOG_DEBUG,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
//MOLMC_LOG_VERBOSE     /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
#define CONFIG_MOLMC_LOG_DEFAULT_LEVEL           MOLMC_LOG_VERBOSE

#endif

