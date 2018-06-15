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
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "hal_import.h"

const static char *TAG = "hal:udp";

char *s_host;
char s_port[6];
struct addrinfo s_hints = {
  .ai_family = AF_INET,
  .ai_socktype = SOCK_DGRAM,
  .ai_protocol = IPPROTO_UDP,
};


int getUdpAddrInfo(char *host, unsigned short port, struct addrinfo **ainfo)
{
    char p_port[6];
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; //only IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    sprintf(p_port, "%u", port);

    ESP_LOGI(TAG, "host: %s; port: %s", host, p_port);

    if (getaddrinfo(host, p_port, &hints, ainfo) < 0) {
        ESP_LOGI(TAG, "getaddrinfo error");
        HAL_Free(&hints);
        return -1;
    }
    HAL_Free(&hints);
    return 0;
}

intptr_t HAL_UDP_create(const char *host, unsigned short port)
{
    struct sockaddr_in saddr;
    int socket_id = -1;
    int rc = -1;

    s_host = HAL_Malloc(strlen(host) + 1);
    memset(s_host, 0, strlen(host) + 1);
    memcpy(s_host, host, strlen(host));
    sprintf(s_port, "%u", port);

    ESP_LOGI(TAG, "establish udp connection with server(host=%s port=%u)", host, port);

    socket_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_id < 0) {
        ESP_LOGI(TAG, "create socket failed");
        return -1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9200);
    saddr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(socket_id, (struct sockaddr *) &saddr, sizeof(saddr));
    if(rc < 0) {
        ESP_LOGI(TAG, "bind socket failed");
        return -1;
    }

    ESP_LOGI(TAG, "create socket success socket id: %d", socket_id);

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
    char            addr[NETWORK_ADDR_LEN] = {0};
    struct addrinfo *ainfo;

    socket_id = (long)p_socket;
    ESP_LOGI(TAG, "send data %d", p_socket);

    if (getaddrinfo(s_host, s_port, &s_hints, &ainfo) < 0) {
        ESP_LOGI(TAG, "getaddrinfo error");
        return -1;
    }

    struct sockaddr_in *sa = (struct sockaddr_in *) ainfo->ai_addr;
    inet_ntop(AF_INET, &sa->sin_addr, addr, NETWORK_ADDR_LEN);
    ESP_LOGI(TAG, "send The host IP %s, port is %d", addr, ntohs(sa->sin_port));

    rc = sendto(socket_id, (char *)p_data, (int)datalen, 0, ainfo->ai_addr, ainfo->ai_addrlen);

    ESP_LOGI(TAG, "send data result rc: %d", rc);
    HAL_Free(ainfo);

    if (rc < 0) {
        return -1;
    }

    return rc;
}

int HAL_UDP_read(intptr_t p_socket, unsigned char *p_data, unsigned int datalen)
{
    long            socket_id = -1;
    int             count = -1;
    char                    addr[NETWORK_ADDR_LEN] = {0};
    struct addrinfo *ainfo;

    if (NULL == p_data) {
        return -1;
    }

    socket_id = (long)p_socket;

    if (socket_id < 0) {
        ESP_LOGI(TAG, "socket error");
        return -1;
    }

    if (getaddrinfo(s_host, s_port, &s_hints, &ainfo) < 0) {
        ESP_LOGI(TAG, "getaddrinfo error");
        return -1;
    }

    struct sockaddr_in *sa = (struct sockaddr_in *) ainfo->ai_addr;
    inet_ntop(AF_INET, &sa->sin_addr, addr, NETWORK_ADDR_LEN);
    ESP_LOGI(TAG, "The host IP %s, port is %d", addr, ntohs(sa->sin_port));

    count = recvfrom(socket_id, p_data, (size_t)datalen, 0, ainfo->ai_addr, &ainfo->ai_addrlen);

    ESP_LOGI(TAG, "read data count: %d", count);
    HAL_Free(ainfo);

    return count;
}

int HAL_UDP_readTimeout(intptr_t p_socket, unsigned char *p_data, unsigned int datalen, unsigned int timeout)
{
    int                 ret;
    struct timeval      tv;
    fd_set              read_fds;
    long                socket_id = -1;

    if (NULL == p_data) {
        ESP_LOGI(TAG, "p_data is NULL");
        return -1;
    }
    socket_id = (long)p_socket;


    if (socket_id < 0) {
        ESP_LOGI(TAG, "socket_id error");
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
  int socket_id = -1;
  struct sockaddr from;
  int count = -1, ret = -1;
  socklen_t addrlen = 0;
  struct timeval      tv;
  fd_set              read_fds;

  if(NULL == p_remote  || NULL == p_data){
    return -1;
  }

  socket_id = (int)sockfd;

  FD_ZERO(&read_fds);
  FD_SET(socket_id, &read_fds);

  tv.tv_sec  = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  ret = select(socket_id + 1, &read_fds, NULL, NULL, timeout_ms == 0 ? NULL : &tv);

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

  addrlen = sizeof(struct sockaddr);
  count = recvfrom(socket_id, p_data, (size_t)datalen, 0, &from, &addrlen);
  if(-1 == count) {
    return -1;
  }
  if (from.sa_family == AF_INET) {
    struct sockaddr_in *sin = (struct sockaddr_in *)&from;
    inet_ntop(AF_INET, &sin->sin_addr, p_remote->addr, NETWORK_ADDR_LEN);
    p_remote->port = ntohs(sin->sin_port);
  }
  return count;
}

int HAL_UDP_sendto(intptr_t sockfd, const NetworkAddr *p_remote, const unsigned char *p_data, unsigned int datalen, unsigned int timeout_ms)
{
  int rc = -1;
  int socket_id = -1;
  struct sockaddr_in remote_addr;

  if(NULL == p_remote || NULL == p_data) {
    return -1;
  }

  socket_id = (int)sockfd;
  remote_addr.sin_family = AF_INET;
  if(1 != (rc = inet_pton(remote_addr.sin_family, p_remote->addr, &remote_addr.sin_addr.s_addr))) {
    return -1;
  }
  remote_addr.sin_port = htons(p_remote->port);
  rc = sendto(socket_id, p_data, (size_t)datalen, 0,
  (const struct sockaddr *)&remote_addr, sizeof(remote_addr));
  if(-1 == rc) {
    return -1;
  }
  return rc;
}

int HAL_UDP_joinmulticast(intptr_t sockfd, const char *p_group)
{
  int err = -1;
  int socket_id = -1;

  if(NULL == p_group) {
    return -1;
  }

  /*set loopback*/
  int loop = 1;
  socket_id = (int)sockfd;
  err = setsockopt(socket_id, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
  if(err < 0) {
    fprintf(stderr,"setsockopt():IP_MULTICAST_LOOP failed\r\n");
    return err;
  }

  struct ip_mreq mreq;
  mreq.imr_multiaddr.s_addr = inet_addr(p_group);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY); /*default networt interface*/

  /*join to the mutilcast group*/
  err = setsockopt(socket_id, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
  if (err < 0) {
    fprintf(stderr,"setsockopt():IP_ADD_MEMBERSHIP failed\r\n");
    return err;
  }

  return 0;
}

