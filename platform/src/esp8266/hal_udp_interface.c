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

const static char *TAG = "hal:udp";

intptr_t HAL_UDP_create(const char *host, unsigned short port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = 0;
    char service[6];
    int flag = 1;

    memset(&hints, 0, sizeof(hints));

    MOLMC_LOGD(TAG, "establish udp connection with server(host=%s port=%u)", host, port);

    hints.ai_family = AF_INET; //only IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
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
            rc = 0;
            continue;
        }

        rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
        if (rc < 0) {
            MOLMC_LOGD(TAG, "socket set opt error");
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        close(fd);
        MOLMC_LOGD(TAG, "connect error");
        rc = 0;
    }

    if (0 == rc) {
        MOLMC_LOGD(TAG, "fail to establish tcp");
    } else {
        MOLMC_LOGD(TAG, "success to establish tcp, fd=%d", rc);
    }
    freeaddrinfo(addrInfoList);

    return (intptr_t)rc;
}

void HAL_UDP_close(intptr_t p_socket)
{
    long            socket_id = -1;

    socket_id = (long)p_socket;
    close(socket_id);
}

int HAL_UDP_write(intptr_t p_socket, const unsigned char *p_data, unsigned int datalen)
{
    int             rc = -1;
    long            socket_id = -1;

    socket_id = (long)p_socket;
    rc = send(socket_id, (char *)p_data, (int)datalen, 0);
    if (-1 == rc) {
        return -1;
    }

    return rc;
}

int HAL_UDP_read(intptr_t p_socket, unsigned char *p_data, unsigned int datalen)
{
    long            socket_id = -1;
    int             count = -1;

    if (NULL == p_data) {
        return -1;
    }

    socket_id = (long)p_socket;
    count = (int)recv(socket_id, p_data, datalen, 0);

    return count;
}

int HAL_UDP_readTimeout(intptr_t p_socket, unsigned char *p_data, unsigned int datalen, unsigned int timeout)
{
    int                 ret;
    struct timeval      tv;
    fd_set              read_fds;
    long                socket_id = -1;

    if (NULL == p_data) {
        return -1;
    }
    socket_id = (long)p_socket;

    if (socket_id < 0) {
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(socket_id, &read_fds);

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ret = select(socket_id + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);

    /* Zero fds ready means we timed out */
    if (ret == 0) {
        return -2;    /* receive timeout */
    }

    if (ret < 0) {
        if (errno == EINTR) {
            return -3;    /* want read */
        }

        return -4; /* receive failed */
    }

    /* This call will not block */
    return HAL_UDP_read(p_socket, p_data, datalen);
}

int HAL_UDP_recvfrom(intptr_t sockfd, NetworkAddr *p_remote, unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms)
{
    return 0;
}

int HAL_UDP_sendto(intptr_t sockfd, const NetworkAddr *p_remote, const unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms)
{
    return 0;
}

int HAL_UDP_joinmulticast(intptr_t sockfd, const char *p_group)
{
    return 0;
}

