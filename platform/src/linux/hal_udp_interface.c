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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "hal_import.h"

#define MOLMC_LOGD(tag, format, ...) do { \
        printf("D [%010u]:[%-12.12s]: "format"\n", HAL_UptimeMs(), tag, ##__VA_ARGS__);\
    } while(0)

const static char *TAG = "hal:udp";

intptr_t HAL_UDP_create(const char *host, unsigned short port)
{
    int                     rc = -1;
    long                    socket_id = -1;
    char                    port_ptr[6] = {0};
    struct addrinfo         hints;
    char                    addr[NETWORK_ADDR_LEN] = {0};
    struct addrinfo        *res, *ainfo;
    struct sockaddr_in     *sa = NULL;
    int flag = 1;

    MOLMC_LOGD(TAG, "establish udp connection with server(host=%s port=%u)", host, port);

    sprintf(port_ptr, "%u", port);
    memset((char *)&hints, 0x00, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;

    rc = getaddrinfo(host, port_ptr, &hints, &res);
    if (0 != rc) {
        MOLMC_LOGD(TAG, "getaddrinfo error");
        return (intptr_t)(-1);
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        if (AF_INET == ainfo->ai_family) {
            sa = (struct sockaddr_in *)ainfo->ai_addr;
            inet_ntop(AF_INET, &sa->sin_addr, addr, NETWORK_ADDR_LEN);
            fprintf(stderr, "The host IP %s, port is %d\r\n", addr, ntohs(sa->sin_port));

            socket_id = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
            if (socket_id < 0) {
                MOLMC_LOGD(TAG, "create socket error");
                continue;
            }
            int retr = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
            if (retr < 0) {
                MOLMC_LOGD(TAG, "socket set opt error");
                continue;
            }
            if (0 == connect(socket_id, ainfo->ai_addr, ainfo->ai_addrlen)) {
                break;
            }

            close(socket_id);
        }
    }
    freeaddrinfo(res);

    return (intptr_t)socket_id;
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
    count = (int)read(socket_id, p_data, datalen);

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

