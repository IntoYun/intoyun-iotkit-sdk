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

#ifndef __IOT_IMPORT_UTILS_H__
#define __IOT_IMPORT_UTILS_H__

#include "iot_import.h"
#include "sdk_config.h"
#include "lite-utils.h"
#include "utils_common.h"
#include "utils_epoch_time.h"
#include "utils_list.h"
#include "utils_net.h"
#include "utils_timer.h"
#include "utils_aes.h"
#include "utils_base64.h"
#include "utils_cJSON.h"
#include "utils_cmac.h"
#include "utils_hmac.h"
#include "utils_md5.h"
#include "utils_sha1.h"

#ifndef MIN
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#endif /* ifndef MIN */

#ifndef MAX
#define MAX(x,y)  ((x) > (y) ? (x) : (y))
#endif /* ifndef MAX */

uint32_t timerGetId(void);
bool timerIsEnd(uint32_t timerID, uint32_t time);

#endif /* _IOT_IMPORT_UTILS_H_ */

