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

#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

//产品ID
//产品ID是一个16个字节的字符串，为一个产品的标识符。设备通过上送产品ID可以把设备归属到该类产品中。
//产品ID属于绝密信息，请防止泄露。
#define PRODUCT_ID_DEF                           "3f132b840000036f"

//产品密钥
//产品密钥是一个32个字节的字符串，通过产品密钥可以完成设备的自动注册。产品密钥在任何时候不传输。
//产品密钥属于绝密信息，请防止泄露。
#define PRODUCT_SECRET_DEF                       "969c42a62213705ffc8769d1678e7f6c"

//硬件版本号
//硬件版本号，为设备的硬件版本号。该版本号将上送至服务器。
#define HARDWARE_VERSION_DEF                     "1.0.0"

//软件版本号
//软件版本号，为设备当前软件的版本号。该版本号将上送至服务器。
#define SOFTWARE_VERSION_DEF                     "1.0.0"

#endif

