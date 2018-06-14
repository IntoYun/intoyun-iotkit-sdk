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
#include "iotx_crypto_api.h"
#include "iotx_guider_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if_api.h"

const static char *TAG = "sdk:crypto";

static void _cal_mic(const uint8_t *src, uint16_t srclen, const uint8_t *key, uint8_t *mic)
{
    uint8_t Mic[16];
    AES_CMAC_CTX AesCmacCtx[1];

    AES_CMAC_Init( AesCmacCtx );
    AES_CMAC_SetKey( AesCmacCtx, key );
    AES_CMAC_Update( AesCmacCtx, src, srclen );
    AES_CMAC_Final( Mic, AesCmacCtx );

    memcpy(mic, Mic, 4);
}

static void _payload_encrypt(const uint8_t *src, uint16_t srclen, const uint8_t *key, uint8_t dir, uint16_t seqId, char *deviceId, uint8_t *dest)
{
    uint16_t i;
    uint8_t bufferIndex = 0;
    uint16_t ctr = 1;
    uint8_t aBlock[16] = {0};
    uint8_t sBlock[16] = {0};
    aes_context AesContext;

    i = strlen(deviceId);
    if(i < 10) {
        return;
    }
    memset( AesContext.ksch, '\0', 240 );
    aes_set_key1( key, 16, &AesContext );

    aBlock[0] = dir;
    aBlock[1] = ( seqId >> 8 ) & 0xFF;
    aBlock[2] = ( seqId ) & 0xFF;
    memcpy(&aBlock[3], &deviceId[i-10], 10);

    while( srclen >= 16 ) {
        aBlock[14] = ( ctr >> 8 ) & 0xFF;
        aBlock[15] = ( ( ctr ) & 0xFF );
        ctr++;
        aes_encrypt1( aBlock, sBlock, &AesContext );
        for( i = 0; i < 16; i++ ) {
            dest[bufferIndex + i] = src[bufferIndex + i] ^ sBlock[i];
        }
        srclen -= 16;
        bufferIndex += 16;
    }

    if( srclen > 0 ) {
        aBlock[14] = ( ctr >> 8 ) & 0xFF;
        aBlock[15] = ( ( ctr ) & 0xFF );
        aes_encrypt1( aBlock, sBlock, &AesContext );
        for( i = 0; i < srclen; i++ ) {
            dest[bufferIndex + i] = src[bufferIndex + i] ^ sBlock[i];
        }
    }
}

static void _payload_decrypt(const uint8_t *src, uint16_t srclen, const uint8_t *key, uint8_t dir, uint16_t seqId, char *deviceId, uint8_t *dest)
{
    _payload_encrypt(src, srclen, key, dir, seqId, deviceId, dest);
}

int iotx_comm_payload_encrypt(uint8_t *dest, uint16_t destlen, uint8_t *src, uint16_t srclen)
{
    uint8_t *ptr = dest;
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_device_info_pt pdev_info = iotx_device_info_get();
    //SECURE_MODE secure_mode = iotx_guider_get_secure_mode();

#if CONFIG_CLOUD_SECURE_MODE == 3 //MODE_TLS_GUIDER_PLAIN
    //防止内存溢出
    if ( srclen > destlen) {
        return -1;
    }
    memcpy(dest, src, srclen);
    return srclen;
#else
    //防止内存溢出
    if ( (2 + srclen + 4) > destlen) {
        return -1;
    }

    pconn_info->up_packetid++;
    Uint16ToHex(ptr, 2, pconn_info->up_packetid);
    ptr += 2;

#if CONFIG_CLOUD_SECURE_MODE == 1 //MODE_TCP_GUIDER_FORMENCRYPT
    _payload_encrypt(src, srclen, pconn_info->appskey, 0, pconn_info->up_packetid, pdev_info->device_id, ptr);
#else
    memcpy(ptr, src, srclen);
#endif
    ptr += srclen;
    _cal_mic(dest, ptr - dest, pconn_info->nwkskey, ptr);
    ptr += 4;

    return ptr - dest;
#endif
}

int iotx_comm_payload_decrypt(uint8_t *dest, uint16_t destlen, uint8_t *src, uint16_t srclen)
{
    uint8_t *ptr = src;
    uint8_t mic[4];
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_device_info_pt pdev_info = iotx_device_info_get();
    //SECURE_MODE secure_mode = iotx_guider_get_secure_mode();

#if CONFIG_CLOUD_SECURE_MODE == 3 //MODE_TLS_GUIDER_PLAIN
    //防止内存溢出
    if ( srclen > destlen) {
        return -1;
    }
    memcpy(dest, src, srclen);
    return srclen;
#else
    //防止内存溢出
    if (( srclen - 2 - 4 ) > destlen) {
        return -1;
    }

    destlen = srclen - 6;
    _cal_mic(ptr, srclen - 4, pconn_info->nwkskey, mic);
    if(!memcmp(&ptr[srclen-4], mic, 4)) {
        pconn_info->down_packetid = HexToUint16(ptr);
        ptr += 2;
#if CONFIG_CLOUD_SECURE_MODE == 1 //MODE_TCP_GUIDER_FORMENCRYPT
        _payload_decrypt( ptr, destlen, pconn_info->appskey, 1, pconn_info->down_packetid, pdev_info->device_id, dest );
#else
        memcpy(dest, ptr, destlen);
#endif
    } else {
        MOLMC_LOGD(TAG, "decrypt error!");
        destlen = 0;
    }
    return destlen;
#endif
}

