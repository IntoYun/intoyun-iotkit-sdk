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

#ifndef INTOYUN_CONFIG_H_
#define INTOYUN_CONFIG_H_

#define SDK_VERSION                               "1.3.1"

// IntoYun云服务接入参数
// 包括接入域名和端口号
#define INTOYUN_SERVER_DOMAIN                     "iot.intoyun.com"
#define INTOYUN_SERVER_PORT                       8080

// 协议包默认缓冲区的大小，当大小不够时，内部会自动申请
#define PROTOCOL_BUFFER_SIZE                      128

// 支持的数据点的最大个数
#define PROPERTIES_MAX                            50

// 数据点自动发送默认时间间隔
#define DATAPOINT_TRANSMIT_AUTOMATIC_INTERVAL     600

// 选择用户日志打印级别后，将打印选择级别及高级别的日志信息
// 0 - INTOYUN_LOG_LEVEL_NONE     :  无调试信息       最高级别
// 1 - INTOYUN_LOG_LEVEL_ERROR    :  打印错误信息
// 2 - INTOYUN_LOG_LEVEL_WARN     :  打印警告信息
// 3 - INTOYUN_LOG_LEVEL_INFO     :  打印描述信息
// 4 - INTOYUN_LOG_LEVEL_DEBUG    :  打印调试信息
// 5 - INTOYUN_LOG_LEVEL_VERBOSE  :  打印详细信息     最低级别

#define INTOYUN_DEBUG_LEVEL                       5

// SDK包内部调试信息开关   注释后关闭
#define INTOYUN_DEBUG_SDK

// ASSERT开关  注释后打开
#define CONFIG_NOASSERT

#endif

