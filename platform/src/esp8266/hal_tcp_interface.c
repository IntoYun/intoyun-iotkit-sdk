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

static uint32_t timer_get_id(void)
{
    return HAL_UptimeMs();
}

static uint32_t timer_get_left(uint32_t timerID, uint32_t time)
{
    uint32_t current_millis = HAL_UptimeMs();
    uint32_t elapsed_millis = 0;

    //Check for wrapping
    if (current_millis < timerID){
        elapsed_millis =  UINT_MAX - timerID + current_millis;
    } else {
        elapsed_millis = current_millis - timerID;
    }

    if (elapsed_millis >= time) {
        return 0;
    }
    return time - elapsed_millis;
}

intptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = -1;
    char service[6];
    int sockopt = 1;
    memset(&hints, 0, sizeof(hints));

    MOLMC_LOGD(TAG, "establish tcp connection with server(host=%s port=%u)", host, port);

    hints.ai_family = AF_INET; /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        MOLMC_LOGD(TAG, "getaddrinfo error");
        return 0;
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            MOLMC_LOGD(TAG, "socket type error");
            rc = 0;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            MOLMC_LOGD(TAG, "create socket error");
            rc = -1;
            continue;
        }

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,&sockopt, sizeof(sockopt));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,&sockopt, sizeof(sockopt));

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        close(fd);
        MOLMC_LOGD(TAG, "connect error");
        rc = -1;
    }

    if (-1 == rc) {
        MOLMC_LOGD(TAG, "fail to establish tcp");
    } else {
        MOLMC_LOGD(TAG, "success to establish tcp, fd=%d", rc);
    }
    freeaddrinfo(addrInfoList);

    return (intptr_t)rc;
}

int HAL_TCP_Destroy(intptr_t fd)
{
    int rc;

    /* Shutdown both send and receive operations. */
    rc = shutdown((int) fd, 2);
    if (0 != rc) {
        MOLMC_LOGD(TAG, "shutdown error");
        return -1;
    }

    rc = close((int) fd);
    if (0 != rc) {
        MOLMC_LOGD(TAG, "closesocket error");
        return -1;
    }

    return 0;
}

int32_t HAL_TCP_Write(intptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret = 1; /* send one time if timeout_ms is value 0 */
    uint32_t len_sent = 0;
    uint32_t timer_id = timer_get_id();
    uint32_t t_left = 0;
    struct timeval timeout;
    fd_set sets;

    do {
        t_left = timer_get_left(timer_id, timeout_ms);

        if (0 != t_left) {
            FD_ZERO(&sets);
            FD_SET(fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(fd + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0) {
                if (0 == FD_ISSET(fd, &sets)) {
                    MOLMC_LOGD(TAG, "Should NOT arrive");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                MOLMC_LOGD(TAG, "select-write timeout %d", (int)fd);
                break;
            } else {
                if (EINTR == errno) {
                    MOLMC_LOGD(TAG, "EINTR be caught");
                    continue;
                }
                MOLMC_LOGD(TAG, "select-write fail");
                break;
            }
        }

        if (ret > 0) {
            ret = send(fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                MOLMC_LOGD(TAG, "No data be sent");
            } else {
                if (EINTR == errno) {
                    MOLMC_LOGD(TAG, "EINTR be caught");
                    continue;
                }
                MOLMC_LOGD(TAG, "send fail");
                break;
            }
        }
    } while ((len_sent < len) && (timer_get_left(timer_id, timeout_ms) > 0));

    return len_sent;
}

int32_t HAL_TCP_Read(intptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret = 1, err_code = 0;
    uint32_t len_recv = 0;
    uint32_t timer_id = timer_get_id();
    uint32_t t_left = 0;
    struct timeval timeout;
    fd_set sets;

    do {
        t_left = timer_get_left(timer_id, timeout_ms);
        if (0 == t_left) {
            break;
        }

        FD_ZERO(&sets);
        FD_SET(fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(fd + 1, &sets, NULL, NULL, &timeout);
        if (ret > 0) {
            ret = recv(fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                MOLMC_LOGD(TAG, "connection is closed");
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
                    MOLMC_LOGD(TAG, "EINTR be caught");
                    continue;
                }
                MOLMC_LOGD(TAG, "recv fail");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            MOLMC_LOGD(TAG, "select-recv fail");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}

