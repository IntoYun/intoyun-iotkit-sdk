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

#include <string.h>
#include "utils_common.h"

static inline char ascii_nibble(uint8_t nibble)
{
    char hex_digit = nibble + 48;
    if (57 < hex_digit)
        hex_digit += 39;
    return hex_digit;
}

static inline uint8_t hex_nibble(uint8_t nibble)
{
    if(('0' <= nibble) && (nibble <= '9')) {
        nibble -= '0';
    } else if(('a' <= nibble) && (nibble <= 'f')) {
        nibble -= 'a';
        nibble += 10;
    } else if(('A' <= nibble) && (nibble <= 'F')) {
        nibble -= 'A';
        nibble += 10;
    } else {
        nibble = 0;
    }
    return nibble;
}

int StringToHex(char *psrc, uint16_t srclen, uint8_t *pdest, uint16_t destlen, bool reverse)
{
    int index = 0, n = 0;

    if(strlen(psrc) != srclen) {
        return -1;
    }

    memset(pdest, 0, sizeof(destlen));

    if(false == reverse) {
        for(index = 0, n = 0; (index < srclen)&&(n <= destlen); index += 2) {
            pdest[n++] = hex_nibble(psrc[index]) << 4 | hex_nibble(psrc[index+1]);
        }
    } else {
        for(index = srclen-2, n = 0; (index >= 0)&&(n <= destlen); index -= 2) {
            pdest[n++] = hex_nibble(psrc[index]) << 4 | hex_nibble(psrc[index+1]);
        }
    }
    return n;
}

int HexToString(uint8_t *psrc, uint16_t srclen, char *pdest, uint16_t destlen, bool reverse)
{
    int index = 0, n = 0;

    if(false == reverse) {
        for(index = 0, n = 0; index < srclen; index++) {
            pdest[n++] = ascii_nibble(psrc[index] >> 4);
            pdest[n++] = ascii_nibble(psrc[index] & 0xF);
        }
    } else {
        for(index = srclen-1, n = 0; index >= 0; index--) {
            pdest[n++] = ascii_nibble(psrc[index] >> 4);
            pdest[n++] = ascii_nibble(psrc[index] & 0xF);
        }
    }
    return n;
}

int Uint16ToHex(uint8_t *pdest, uint16_t destlen, uint16_t val)
{
    if(destlen < 2) {
        return -1;
    }

    pdest[0] = (val >> 8) & 0xFF;
    pdest[1] = (val) & 0xFF;

    return 0;
}

int Uint32ToHex(uint8_t *pdest, uint16_t destlen, uint32_t val)
{
    if(destlen < 4) {
        return -1;
    }

    pdest[0] = (val >> 24) & 0xFF;
    pdest[1] = (val >> 16) & 0xFF;
    pdest[2] = (val >> 8) & 0xFF;
    pdest[3] = (val) & 0xFF;

    return 0;
}

uint16_t HexToUint16(uint8_t *psrc)
{
    return (psrc[0] << 8) + psrc[1];
}

uint32_t HexToUint32(uint8_t *psrc)
{
    return (psrc[0] << 24) + (psrc[1] << 16) + (psrc[2] << 8) + psrc[3];
}

