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
#include <windows.h>
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib") //加载 ws2_32.dll

#include "hal_import.h"

#define MOLMC_LOGD(tag, format, ...) do { \
        printf("D [%010u]:[%-12.12s]: "format"\n", HAL_UptimeMs(), tag, ##__VA_ARGS__);\
    } while(0)

const static char *TAG = "hal:udp";

intptr_t HAL_UDP_create(const char *host, unsigned short port)
{
    intptr_t sockfd;
    WSADATA wsaData;
    struct hostent *hp;
    struct sockaddr_in addrServer;
    char flag = 1;

    WSAStartup(0x202, &wsaData);

    MOLMC_LOGD(TAG, "establish udp connection host : %s, port : %u\n", host, port);

    hp = gethostbyname(host);


    memset(&addrServer, 0, sizeof(addrServer));
    memcpy(&(addrServer.sin_addr), hp->h_addr, hp->h_length);

    MOLMC_LOGD(TAG, "ip = %u.%u.%u.%u",
    addrServer.sin_addr.S_un.S_un_b.s_b1,
    addrServer.sin_addr.S_un.S_un_b.s_b2,
    addrServer.sin_addr.S_un.S_un_b.s_b3,
    addrServer.sin_addr.S_un.S_un_b.s_b4);

    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons((unsigned short)port);

    sockfd = socket(addrServer.sin_family, SOCK_DGRAM, 0);   /* socket */

    int retr = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    MOLMC_LOGD(TAG, "connecting to %s", host);
    if (0 < connect(sockfd, (struct sockaddr *)&addrServer, sizeof(struct sockaddr))) {
        MOLMC_LOGD(TAG, "connect failed!\n");
        closesocket(sockfd);
        return (intptr_t)-1;
    }

    MOLMC_LOGD(TAG, "connect successfully!\n");

    return sockfd;
}

void HAL_UDP_close(intptr_t p_socket)
{
    long            socket_id = -1;

    socket_id = (long)p_socket;
    closesocket(socket_id);
    WSACleanup();
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

