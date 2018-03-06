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

#ifndef __IOTX_GUIDER_API_H__
#define __IOTX_GUIDER_API_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    MODE_TCP_GUIDER_FORMENCRYPT  = 0,
    MODE_TCP_GUIDER_FORMPLAIN    = 1,
    MODE_TLS_GUIDER_PLAIN        = 2,
} SECURE_MODE;


SECURE_MODE iotx_guider_get_secure_mode(void);
int iotx_guider_authenticate(void);
int iotx_guider_auth_gen_keys(void);

#if defined(__cplusplus)
}
#endif
#endif
