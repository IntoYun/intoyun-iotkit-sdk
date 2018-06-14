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

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "hal_import.h"

#define MOLMC_LOGD(tag, format, ...) do { \
        printf("D [%010u]:[%-12.12s]: "format"\n", HAL_UptimeMs(), tag, ##__VA_ARGS__);\
    } while(0)

const static char *TAG = "hal:tcp";

intptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    int fd = -1;
    struct sockaddr_in sAddr;
    struct hostent* ipAddress;

    MOLMC_LOGD(TAG, "establish tcp connection with server(host=%s port=%u)", host, port);

    if ((ipAddress = gethostbyname(host)) == 0) {
        goto exit;
    }

    sAddr.sin_family = AF_INET;
    sAddr.sin_addr.s_addr = ((struct in_addr*)(ipAddress->h_addr))->s_addr;
    sAddr.sin_port = htons(port);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        goto exit;
    }

    if ((connect(fd, (struct sockaddr*)&sAddr, sizeof(sAddr))) < 0) {
        close(fd);
        fd = -1;
        goto exit;
    }

exit:
    if (fd < 0) {
        MOLMC_LOGD(TAG, "fail to establish tcp");
    } else {
        MOLMC_LOGD(TAG, "success to establish tcp, fd=%d", fd);
    }
    return (intptr_t)fd;
}

int HAL_TCP_Destroy(intptr_t fd)
{
    MOLMC_LOGD(TAG, "success to destroy tcp, fd=%d", fd);
    close((int)fd);
    return 0;
}

int32_t HAL_TCP_Write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    portTickType xTicksToWait = timeout_ms / portTICK_RATE_MS; /* convert milliseconds to ticks */
    xTimeOutType xTimeOut;
    int sentLen = 0;
    int rc = 0;
    int readysock;

    struct timeval timeout;
    fd_set fdset;

    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms * 1000;

    vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */

    do {
        readysock = select(fd + 1, NULL, &fdset, NULL, &timeout);
    } while (readysock <= 0);

    if (FD_ISSET(fd, &fdset)) {
        do {
            rc = send(fd, buf + sentLen, len - sentLen, MSG_DONTWAIT);

            if (rc > 0) {
                sentLen += rc;
            } else if (rc < 0) {
                sentLen = rc;
                break;
            }
        } while (sentLen < len && xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);
    }

    return sentLen;
}

int32_t HAL_TCP_Read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    portTickType xTicksToWait = timeout_ms / portTICK_RATE_MS; /* convert milliseconds to ticks */
    xTimeOutType xTimeOut;
    int recvLen = 0;
    int rc = 0;

    struct timeval timeout;
    fd_set fdset;

    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);

    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_ms * 1000;

    vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */

    if (select(fd + 1, &fdset, NULL, NULL, &timeout) > 0) {
        if (FD_ISSET(fd, &fdset)) {
            do {
                rc = recv(fd, buf + recvLen, len - recvLen, MSG_DONTWAIT);

                if (rc > 0) {
                    recvLen += rc;
                } else if (rc < 0) {
                    recvLen = rc;
                    break;
                }
            } while (recvLen < len && xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);
        }
    }

    return recvLen;
}

