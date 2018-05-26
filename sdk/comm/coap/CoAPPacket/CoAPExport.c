/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
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
#include "ctype.h"
#include "iot_import.h"

#include "CoAPNetwork.h"
#include "CoAPExport.h"

#define COAP_DEFAULT_PORT           5683 /* CoAP default UDP port */
#define COAPS_DEFAULT_PORT          5684 /* CoAP default UDP port for secure transmission */

#define COAP_DEFAULT_SCHEME         "coap" /* the default scheme for CoAP URIs */
#define COAP_DEFAULT_HOST_LEN       128
#define COAP_DEFAULT_WAIT_TIME_MS   2000

unsigned int CoAPUri_parse(char *p_uri, coap_endpoint_type *p_endpoint_type,
                           char host[COAP_DEFAULT_HOST_LEN], unsigned short *port)
{
    int len = 0;
    char *p = NULL, *q = NULL;
    if (NULL == p_uri  || NULL == p_endpoint_type) {
        return COAP_ERROR_INVALID_PARAM;
    }

    len = strlen(p_uri);
    p = p_uri;
    q = (char *)COAP_DEFAULT_SCHEME;
    while (len && *q && tolower(*p) == *q) {
        ++p;
        ++q;
        --len;
    }

    if (*q) {
        return COAP_ERROR_INVALID_URI;
    }
    if (tolower(*p) == 's') {
        ++p;
        --len;
        *p_endpoint_type = COAP_ENDPOINT_DTLS;
        *port     = COAPS_DEFAULT_PORT;
    } else {
        *p_endpoint_type = COAP_ENDPOINT_NOSEC;
        *port     = COAP_DEFAULT_PORT;
    }
    COAP_DEBUG("The endpoint type is: %d", *p_endpoint_type);

    q = (char *)"://";
    while (len && *q && tolower(*p) == *q) {
        ++p;
        ++q;
        --len;
    }

    if (*q) {
        return COAP_ERROR_INVALID_URI;
    }

    q = p;
    while (len && *q != ':') {
        ++q;
        --len;
    }
    if (p == q) {
        return COAP_ERROR_INVALID_URI;
    }

    if (COAP_DEFAULT_HOST_LEN - 1 < (q - p)) {
        return COAP_ERROR_INVALID_URI;
    } else {
        memset(host, 0x00, COAP_DEFAULT_HOST_LEN);
        strncpy(host, p, q - p);
    }
    COAP_DEBUG("The host name is: %s", host);
    if (len && *q == ':') {
        p = ++q;
        --len;

        while (len && isdigit(*q)) {
            ++q;
            --len;
        }

        if (p < q) {
            int uri_port = 0;

            while (p < q) {
                uri_port = uri_port * 10 + (*p++ - '0');
            }

            if (uri_port > 65535) {
                return COAP_ERROR_INVALID_URI;
            }
            *port = uri_port;
        }
    }
    COAP_DEBUG("The port is: %d", *port);

    return COAP_SUCCESS;
}


CoAPContext *CoAPContext_create(CoAPInitParam *param)
{
    unsigned int    ret   = COAP_SUCCESS;
    CoAPContext    *p_ctx = NULL;
    coap_network_init_t network_param;
    char host[COAP_DEFAULT_HOST_LEN] = {0};

    memset(&network_param, 0x00, sizeof(coap_network_init_t));
    p_ctx = coap_malloc(sizeof(CoAPContext));
    if (NULL == p_ctx) {
        COAP_ERR("malloc for coap context failed");
        goto err;
    }

    memset(p_ctx, 0, sizeof(CoAPContext));
    p_ctx->message_id = 1;
    p_ctx->notifier = param->notifier;
    p_ctx->sendbuf = coap_malloc(COAP_MSG_MAX_PDU_LEN);
    if (NULL == p_ctx->sendbuf) {
        COAP_ERR("not enough memory");
        goto err;
    }

    p_ctx->recvbuf = coap_malloc(COAP_MSG_MAX_PDU_LEN);
    if (NULL == p_ctx->recvbuf) {
        COAP_ERR("not enough memory");
        goto err;
    }

    if (0 == param->waittime) {
        p_ctx->waittime = COAP_DEFAULT_WAIT_TIME_MS;
    } else {
        p_ctx->waittime = param->waittime;
    }

    /*CoAP message send list*/
    INIT_LIST_HEAD(&p_ctx->list.sendlist);
    p_ctx->list.count = 0;
    p_ctx->list.maxcount = param->maxcount;

    /*set the endpoint type by uri schema*/
    if (NULL != param->url) {
        ret = CoAPUri_parse(param->url, &network_param.ep_type, host, &network_param.port);
    }

    if (COAP_SUCCESS != ret) {
        goto err;
    }

#ifdef COAP_DTLS_SUPPORT
    if (COAP_ENDPOINT_DTLS == network_param.ep_type) {
        extern const char *iotx_coap_get_ca(void);
        network_param.p_ca_cert_pem  = (unsigned char *)iotx_coap_get_ca();
    }
#endif
    if (COAP_ENDPOINT_NOSEC == network_param.ep_type) {
        network_param.p_ca_cert_pem = NULL;
    }
    network_param.p_host = host;

    /*CoAP network init*/
    ret = CoAPNetwork_init(&network_param, &p_ctx->network);

    if (COAP_SUCCESS != ret) {
        goto err;
    }

    return p_ctx;
err:
    if (NULL == p_ctx) {
        return p_ctx;
    }

    if (NULL != p_ctx->recvbuf) {
        coap_free(p_ctx->recvbuf);
        p_ctx->recvbuf = NULL;
    }

    if (NULL != p_ctx->sendbuf) {
        coap_free(p_ctx->sendbuf);
        p_ctx->sendbuf = NULL;
    }

    coap_free(p_ctx);
    p_ctx = NULL;

    return p_ctx;
}

void CoAPContext_free(CoAPContext *p_ctx)
{
    if (NULL == p_ctx) {
        return;
    }

    CoAPSendNode *cur, *next;

    CoAPNetwork_deinit(&p_ctx->network);

    list_for_each_entry_safe(cur, next, &p_ctx->list.sendlist, sendlist, CoAPSendNode) {
        if (NULL != cur) {
            if (NULL != cur->message) {
                coap_free(cur->message);
                cur->message = NULL;
            }
            coap_free(cur);
            cur = NULL;
        }
    }

    if (NULL != p_ctx->recvbuf) {
        coap_free(p_ctx->recvbuf);
        p_ctx->recvbuf = NULL;
    }

    if (NULL != p_ctx->sendbuf) {
        coap_free(p_ctx->sendbuf);
        p_ctx->sendbuf = NULL;
    }


    if (NULL != p_ctx) {
        coap_free(p_ctx);
        p_ctx    =  NULL;
    }
}
