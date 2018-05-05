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

#ifndef __UTILS_COMMON_H__
#define __UTILS_COMMON_H__

#include <stdbool.h>
#include <stdint.h>


int StringToHex(char *psrc, uint16_t srclen, uint8_t *pdest, uint16_t destlen, bool reverse);
int HexToString(uint8_t *psrc, uint16_t srclen, char *pdest, uint16_t destlen, bool reverse);
int Uint16ToHex(uint8_t *pdest, uint16_t destlen, uint16_t val);
int Uint32ToHex(uint8_t *pdest, uint16_t destlen, uint32_t val);
uint16_t HexToUint16(uint8_t *psrc);
uint32_t HexToUint32(uint8_t *psrc);

#endif

