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

#ifndef __IOTX_CRYPTO_API_H__
#define __IOTX_CRYPTO_API_H__

#ifdef __cplusplus
extern "C" {
#endif

int iotx_comm_payload_encrypt(uint8_t *dest, uint16_t destlen, uint8_t *src, uint16_t srclen);
int iotx_comm_payload_decrypt(uint8_t *dest, uint16_t destlen, uint8_t *src, uint16_t srclen);

#ifdef __cplusplus
}
#endif

#endif // __IOTX_CRYPTO_API_H__

