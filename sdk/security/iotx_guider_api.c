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
#include "iotx_guider_api.h"
#include "iotx_system_api.h"
#include "iotx_comm_if_api.h"

const static char *TAG = "sdk:guider";

const char *secmode_str[] = {
    "TCP + Guider + FormCrypto",
    "TCP + Guider + FormPlain",
    "TLS + Direct + Plain"
};

void _calc_cmac( const uint8_t *buffer, uint16_t size, const uint8_t *key, uint8_t *cMac )
{
    AES_CMAC_CTX AesCmacCtx[1];

    AES_CMAC_Init(AesCmacCtx);
    AES_CMAC_SetKey(AesCmacCtx, key);
    AES_CMAC_Update(AesCmacCtx, buffer, size);
    AES_CMAC_Final(cMac, AesCmacCtx);
}

void _gen_session_keys( const uint8_t *key, char *random_str, uint8_t *nwkSKey, uint8_t *appSKey )
{
    uint8_t nonce[16];

    memset( nonce, 0, sizeof( nonce ) );
    nonce[0] = 0x01;
    memcpy((char *)&nonce[1], random_str, strlen(random_str));
    _calc_cmac( nonce, 16, key, appSKey );

    memset( nonce, 0, sizeof( nonce ) );
    nonce[0] = 0x02;
    memcpy((char *)&nonce[1], random_str, strlen(random_str));
    _calc_cmac( nonce, 16, key, nwkSKey );
}

static void _guider_get_timestamp_str(char *buf, int len)
{
    uint64_t ret = 0;
    int retry = 0;

    do {
#if CONFIG_CLOUD_CHANNEL == 1     //MQTT
        ret = utils_http_get_epoch_time(buf, len);
#else
        ret = utils_coap_get_epoch_time(buf, len);
#endif
    } while (ret == 0 && ++retry < 10);

    if (retry > 1) {
        MOLMC_LOGE(TAG, "utils_get_epoch_time() retry = %d.", retry);
    }

    if (ret == 0) {
        MOLMC_LOGE(TAG, "utils_get_epoch_time() failed!");
    }

    return;
}

SECURE_MODE iotx_guider_get_secure_mode(void)
{
    SECURE_MODE rc = MODE_TCP_GUIDER_FORMENCRYPT;
#if CONFIG_CLOUD_SECURE_MODE == 1
    rc = MODE_TCP_GUIDER_FORMENCRYPT;
#elif CONFIG_CLOUD_SECURE_MODE == 2
    rc = MODE_TCP_GUIDER_FORMPLAIN;
#elif CONFIG_CLOUD_SECURE_MODE == 3
    rc = MODE_TLS_GUIDER_PLAIN;
#else
    #error "NOT support yet!"
#endif
    return rc;
}

void print_comm_info(iotx_device_info_pt pdev_info, iotx_conn_info_pt pconn_info, int secure_mode)
{
    MOLMC_LOGD(TAG, "%s", "....................................................");
    MOLMC_LOGD(TAG, "%20s : %-s", "product_id", pdev_info->product_id);
    MOLMC_LOGD(TAG, "%20s : %-s", "product_secret", pdev_info->product_secret);
    MOLMC_LOGD(TAG, "%20s : %-s", "device_id", pdev_info->device_id);
    MOLMC_LOGD(TAG, "%20s : %-s", "device_secret", pdev_info->device_secret);
    MOLMC_LOGD(TAG, "%20s : %-s", "hardware_version", pdev_info->hardware_version);
    MOLMC_LOGD(TAG, "%20s : %-s", "software_version", pdev_info->software_version);
    MOLMC_LOGD(TAG, "%s", "....................................................");
    MOLMC_LOGD(TAG, "%20s : %d", "server_port", pconn_info->port);
    MOLMC_LOGD(TAG, "%20s : %-s", "server_host", pconn_info->host_name);
    MOLMC_LOGD(TAG, "%20s : %-s", "client_id", pconn_info->client_id);
    MOLMC_LOGD(TAG, "%20s : %-s", "username", pconn_info->username);
    MOLMC_LOGD(TAG, "%20s : %-s", "password", pconn_info->password);
    MOLMC_LOGD(TAG, "%20s : %-s", "random", pconn_info->random_str);
    MOLMC_LOGD(TAG, "%20s : %d (%s)", "SecMode", secure_mode, secmode_str[secure_mode]);
    MOLMC_LOGD(TAG, "%s", "....................................................");
    return;
}

int iotx_guider_authenticate(void)
{
    uint8_t ramdom_hex[4] = {0}, cMac_hex[16] = {0}, device_secret_hex[16] = {0};
    char cMac_string[33] = {0};
    char time_stamp_str[16] = {0};
    SECURE_MODE secure_mode = MODE_TCP_GUIDER_FORMENCRYPT;

    iotx_device_info_pt pdev_info = iotx_device_info_get();
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();

    _guider_get_timestamp_str(time_stamp_str, sizeof(time_stamp_str));
    MOLMC_LOGD(TAG, "timestamp = %s", time_stamp_str);
    HAL_Srandom(atoi(time_stamp_str));

    pconn_info->up_packetid = 0;
    pconn_info->down_packetid = 0;

    Uint32ToHex(ramdom_hex, sizeof(ramdom_hex), HAL_Random(INT_MAX));
    HexToString(ramdom_hex, sizeof(ramdom_hex), pconn_info->random_str, sizeof(pconn_info->random_str), false);

    memset(pconn_info->password, 0, sizeof(pconn_info->password));
    strcat(pconn_info->password, pconn_info->random_str);

    secure_mode = iotx_guider_get_secure_mode();
    switch(secure_mode) {
        case MODE_TCP_GUIDER_FORMPLAIN:
            strcat(pconn_info->password, ":0");
            break;
        case MODE_TLS_GUIDER_PLAIN:
            strcat(pconn_info->password, ":2");
            break;
        default:
            strcat(pconn_info->password, ":1");
            break;
    }

    StringToHex(pdev_info->device_secret, 32, device_secret_hex, 16, false);
    _calc_cmac((uint8_t *)pconn_info->random_str, strlen(pconn_info->random_str), device_secret_hex, cMac_hex);
    HexToString(cMac_hex, 16, cMac_string, 33, false);
    strcat(pconn_info->password, cMac_string);

    print_comm_info(pdev_info, pconn_info, secure_mode);
    return 0;
}

int iotx_guider_auth_gen_keys(void)
{
    uint8_t device_secret_hex[16] = {0};
    iotx_conn_info_pt pconn_info = iotx_conn_info_get();
    iotx_device_info_pt pdev_info = iotx_device_info_get();

    StringToHex(pdev_info->device_secret, 32, device_secret_hex, 16, false);
    _gen_session_keys(device_secret_hex, pconn_info->random_str, pconn_info->nwkskey, pconn_info->appskey);

    MOLMC_LOG_BUFFER_HEX(TAG, pconn_info->appskey, 16);
    MOLMC_LOG_BUFFER_HEX(TAG, pconn_info->nwkskey, 16);
    return 0;
}

