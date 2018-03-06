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

#ifndef __IOTX_DEVICE_API_H__
#define __IOTX_DEVICE_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "iot_import.h"
#include "iot_import_errno.h"

#define DEVICE_ID_LEN                     (32)
#define DEVICE_SECRET_LEN                 (32)
#define PRODUCT_ID_LEN                    (32)
#define PRODUCT_SECRET_LEN                (32)
#define HARDWARE_VERSION_LEN              (32)
#define SOFTWARE_VERSION_LEN              (32)

#define PRODUCT_KEY_LEN                   (20)
#define DEVICE_NAME_LEN                   (32)

typedef enum {
    COMM_TYPE_WIFI,
    COMM_TYPE_GPRS
} iotx_comm_type_t;

typedef struct {
    iotx_comm_type_t comm_type;
    char device_id[DEVICE_ID_LEN + 1];
    char device_secret[DEVICE_SECRET_LEN + 1];
    char product_id[PRODUCT_ID_LEN + 1];
    char product_secret[PRODUCT_SECRET_LEN + 1];
    char hardware_version[HARDWARE_VERSION_LEN + 1];
    char software_version[SOFTWARE_VERSION_LEN + 1];

    char product_key[PRODUCT_KEY_LEN + 1];
    char device_name[DEVICE_NAME_LEN + 1];
} iotx_device_info_t, *iotx_device_info_pt;

iotx_device_info_pt iotx_device_info_get(void);

int IOT_SYSTEM_DeviceInit(void);
int IOT_SYSTEM_SetDeviceInfo(char *productID, char *productSecret, char *hardwareVersion, char *softwareVersion, char *deviceId, char *deviceSecret, iotx_comm_type_t commType);
void IOT_SYSTEM_Init(void);
void IOT_SYSTEM_Loop(void);


#ifdef __cplusplus
}
#endif

#endif  /* #ifndef _IOTX_DEVICE_H_ */

