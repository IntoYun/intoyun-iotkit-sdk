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



#include "stdio.h"
#include "CoAPExport.h"
#include "CoAPSerialize.h"
#include "CoAPDeserialize.h"
#include "iot_import.h"
#include "iot_import_coap.h"

const static char *TAG = "sdk:coappacket";

#define COAPAckMsg(header) \
    ((header.code == COAP_MSG_CODE_EMPTY_MESSAGE) \
     &&(header.type == COAP_MESSAGE_TYPE_ACK))

#define CoAPRespMsg(header)\
    ((header.code >= 0x40) && (header.code < 0xc0))

#define CoAPPingMsg(header)\
    ((header.code == COAP_MSG_CODE_EMPTY_MESSAGE)\
     && (header.type == COAP_MESSAGE_TYPE_CON))

#define CoAPRstMsg(header)\
    (header.type == COAP_MESSAGE_TYPE_RST)

#define CoAPCONRespMsg(header)\
    ((header.code == COAP_MSG_CODE_205_CONTENT) \
     && (header.type == COAP_MESSAGE_TYPE_CON))

#define CoAPReqMsg(header)\
    ((1 <= header.code) && (32 > header.code))


#define COAP_CUR_VERSION        1
#define COAP_WAIT_TIME_MS       2000
#define COAP_MAX_MESSAGE_ID     65535
#define COAP_MAX_RETRY_COUNT    4
#define COAP_ACK_TIMEOUT        2
#define COAP_ACK_RANDOM_FACTOR  1
#define COAP_MAX_TRANSMISSION_SPAN   10

int CoAPStrOption_add(CoAPMessage *message, unsigned short optnum, unsigned char *data, unsigned short datalen)
{
    unsigned char *ptr = NULL;
    if (COAP_MSG_MAX_OPTION_NUM <= message->optnum) {
        MOLMC_LOGE(TAG, "Invalidate option");
        return COAP_ERROR_INVALID_PARAM;
    }

    message->options[message->optnum].num = optnum - message->optdelta;

    message->options[message->optnum].len = datalen;
    ptr = (unsigned char *)coap_malloc(datalen);
    if (NULL != ptr) {
        memcpy(ptr, data, datalen);
    }
    message->options[message->optnum].val = ptr;
    message->optdelta = optnum;
    message->optnum ++;

    return COAP_SUCCESS;

}

int CoAPUintOption_add(CoAPMessage *message, unsigned short  optnum, unsigned int data)
{
    unsigned char *ptr = NULL;
    if (COAP_MSG_MAX_OPTION_NUM <= message->optnum) {
        MOLMC_LOGE(TAG, "Invalidate option");
        return COAP_ERROR_INVALID_PARAM;
    }
    if(optnum == COAP_OPTION_OBSERVE) {
      if(data == 0) {
        message->observer = COAP_REGISTERED_OBSERVE;
      } else {
        message->observer = COAP_DEREGISTER_OBSERVE;
      }
    }
    message->options[message->optnum].num = optnum - message->optdelta;

    if (0 == data) {
        message->options[message->optnum].len = 0;
    } else if (256 >= data) {
        message->options[message->optnum].len = 1;
        ptr = (unsigned char *)coap_malloc(1);
        if (NULL != ptr) {
            *ptr = (unsigned char)data;
        }
    } else if (65535 >= data) {
        message->options[message->optnum].len = 2;
        ptr  = (unsigned char *)coap_malloc(2);
        if (NULL != ptr) {
            *ptr     = (unsigned char)((data & 0xFF00) >> 8);
            *(ptr + 1) = (unsigned char)(data & 0x00FF);
        }
    } else {
        message->options[message->optnum].len = 4;
        ptr   = (unsigned char *)coap_malloc(4);
        if (NULL != ptr) {
            *ptr     = (unsigned char)((data & 0xFF000000) >> 24);
            *(ptr + 1) = (unsigned char)((data & 0x00FF0000) >> 16);
            *(ptr + 2) = (unsigned char)((data & 0x0000FF00) >> 8);
            *(ptr + 3) = (unsigned char)(data & 0x000000FF);
        }
    }
    message->options[message->optnum].val = ptr;
    message->optdelta = optnum;
    message->optnum   += 1;

    return COAP_SUCCESS;
}

unsigned short CoAPMessageId_gen(CoAPContext *context)
{
    unsigned short msg_id = 0;
    msg_id = ((COAP_MAX_MESSAGE_ID == context->message_id)  ? 1 : context->message_id++);
    return msg_id;
}


int CoAPMessageId_set(CoAPMessage *message, unsigned short msgid)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    message->header.msgid = msgid;
    return COAP_SUCCESS;
}

int CoAPMessageType_set(CoAPMessage *message, unsigned char type)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    if (COAP_MESSAGE_TYPE_CON != type && COAP_MESSAGE_TYPE_NON != type
        && COAP_MESSAGE_TYPE_ACK != type && COAP_MESSAGE_TYPE_RST != type) {
        return COAP_ERROR_INVALID_PARAM;
    }

    message->header.type = type;
    return COAP_SUCCESS;
}

int CoAPMessageTopic_set(CoAPMessage *message, unsigned char *topic)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    int topic_len = strlen((char *)topic);
    message->topic = coap_malloc(topic_len);
    memset(message->topic, 0x00, topic_len);
    memcpy(message->topic, topic, strlen((char *)topic));
    return COAP_SUCCESS;
}

int CoAPMessageCode_set(CoAPMessage *message, CoAPMessageCode code)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    message->header.code  = code;
    return COAP_SUCCESS;
}

int CoAPMessageToken_set(CoAPMessage *message, unsigned char *token,
                         unsigned char tokenlen)
{
    if (NULL == message || NULL == token) {
        return COAP_ERROR_NULL;
    }
    if (COAP_MSG_MAX_TOKEN_LEN < tokenlen) {
        return COAP_ERROR_INVALID_LENGTH;
    }
    memcpy(message->token, token, tokenlen);
    message->header.tokenlen = tokenlen;

    return COAP_SUCCESS;
}

int CoAPMessageUserData_set(CoAPMessage *message, void *userdata)
{
    if (NULL == message || NULL == userdata) {
        return COAP_ERROR_NULL;
    }
    message->user = userdata;
    return COAP_SUCCESS;
}

int CoAPMessagePayload_set(CoAPMessage *message, unsigned char *payload,
                           unsigned short payloadlen)
{
    if (NULL == message || (0 < payloadlen && NULL == payload)) {
        return COAP_ERROR_NULL;
    }
    message->payload = payload;
    message->payloadlen = payloadlen;

    return COAP_SUCCESS;
}

int CoAPMessageHandler_set(CoAPMessage *message, CoAPRespMsgHandler handler)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    message->handler = handler;
    return COAP_SUCCESS;
}

int CoAPMessage_init(CoAPMessage *message)
{
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }
    memset(message, 0x00, sizeof(CoAPMessage));
    message->header.version    = COAP_CUR_VERSION;
    message->header.type       = COAP_MESSAGE_TYPE_ACK;
    message->header.tokenlen   = 0;
    message->header.code       = COAP_MSG_CODE_EMPTY_MESSAGE;
    message->header.msgid      = 0;
    message->payload           = NULL;
    message->payloadlen        = 0;
    message->optnum            = 0;
    message->optdelta          = 0;
    message->observer          = 1;
    message->handler           = NULL;
    message->topic             = NULL;

    return COAP_SUCCESS;
}

int CoAPMessage_destory(CoAPMessage *message)
{
    int count = 0;
    if (NULL == message) {
        return COAP_ERROR_NULL;
    }

    for (count = 0; count < COAP_MSG_MAX_TOKEN_LEN; count++) {
        if (NULL != message->options[count].val) {
            coap_free(message->options[count].val);
            message->options[count].val = NULL;
        }
    }

    return COAP_SUCCESS;
}

static int CoAPMessageList_add(CoAPContext *context, CoAPMessage *message, int len)
{
    CoAPSendNode *node = NULL;
    node = coap_malloc(sizeof(CoAPSendNode));

    if (NULL != node) {
        node->acked        = 0;
        node->user         = message->user;
        node->msgid        = message->header.msgid;
        node->handler      = message->handler;
        node->msglen       = len;
        node->timeout_val  = COAP_ACK_TIMEOUT * COAP_ACK_RANDOM_FACTOR;
        node->observer     = message->observer;
        node->topic        = message->topic;
        MOLMC_LOGD(TAG, "send node topic: %s",  message->topic);

        if (COAP_MESSAGE_TYPE_CON == message->header.type) {
            node->timeout       = node->timeout_val;
            node->retrans_count = 0;
        } else {
            node->timeout       = COAP_MAX_TRANSMISSION_SPAN;
            node->retrans_count = COAP_MAX_RETRY_COUNT;
        }
        node->tokenlen     = message->header.tokenlen;
        memcpy(node->token, message->token, message->header.tokenlen);
        node->message      = (unsigned char *)coap_malloc(len);
        if (NULL != node->message) {
            memcpy(node->message, context->sendbuf, len);
        }

        if (&context->list.count >= &context->list.maxcount) {
            coap_free(node);
            return -1;
        } else {
            list_add_tail(&node->sendlist, &context->list.sendlist);
            context->list.count ++;
            return 0;
        }
    } else {
        return -1;
    }
}

int CoAPMessage_send(CoAPContext *context, CoAPMessage *message)
{
    unsigned int   ret            = COAP_SUCCESS;
    unsigned short msglen         = 0;

    if (NULL == message || NULL == context) {
        return (COAP_ERROR_INVALID_PARAM);
    }

    /* TODO: get the message length */
    msglen = CoAPSerialize_MessageLength(message);
    if (COAP_MSG_MAX_PDU_LEN < msglen) {
        MOLMC_LOGD(TAG, "The message length %d is too loog", msglen);
        return COAP_ERROR_DATA_SIZE;
    }

    memset(context->sendbuf, 0x00, COAP_MSG_MAX_PDU_LEN);
    msglen = CoAPSerialize_Message(message, context->sendbuf, COAP_MSG_MAX_PDU_LEN);
    MOLMC_LOGD(TAG, "----The message length %d-----", msglen);


    ret = CoAPNetwork_write(&context->network, context->sendbuf, (unsigned int)msglen);
    if (COAP_SUCCESS == ret) {
        if (CoAPReqMsg(message->header) || CoAPCONRespMsg(message->header)) {
            MOLMC_LOGD(TAG, "Add message id %d len %d to the list",
                       message->header.msgid, msglen);
            CoAPMessageList_add(context, message, msglen);
        } else {
            MOLMC_LOGD(TAG, "The message doesn't need to be retransmitted");
        }
    } else {
        MOLMC_LOGE(TAG, "CoAP transoprt write failed, return %d", ret);
    }

    return ret;
}


static int CoAPAckMessage_handle(CoAPContext *context, CoAPMessage *message)
{
    CoAPSendNode *node = NULL;

    list_for_each_entry(node, &context->list.sendlist, sendlist, CoAPSendNode) {
        if (node->msgid == message->header.msgid) {
            node->acked = 1;
            return COAP_SUCCESS;
        }
    }

    return COAP_SUCCESS;
}

static int CoAPAckMessage_send(CoAPContext *context, unsigned short msgid)
{
    CoAPMessage message;
    CoAPMessage_init(&message);
    CoAPMessageId_set(&message, msgid);
    return CoAPMessage_send(context, &message);
}

static int CoAPRespMessage_handle(CoAPContext *context, CoAPMessage *message)
{
    CoAPSendNode *node = NULL;

    if (COAP_MESSAGE_TYPE_CON == message->header.type) {
        CoAPAckMessage_send(context, message->header.msgid);
    }

    list_for_each_entry(node, &context->list.sendlist, sendlist, CoAPSendNode) {
        if (0 != node->tokenlen && node->tokenlen == message->header.tokenlen
            && 0 == memcmp(node->token, message->token, message->header.tokenlen)) {

            MOLMC_LOGD(TAG, "Find the node by token");
            message->user  = node->user;
            if (COAP_MSG_CODE_400_BAD_REQUEST <= message->header.code) {
                if (NULL != context->notifier) {
                    context->notifier(message->header.code, message);
                }
            }

            if (NULL != node->handler) {
                node->handler(node->user, message);
            }

            MOLMC_LOGD(TAG, "Remove the message id %d from list", node->msgid);
            if(node->observer == COAP_DEREGISTER_OBSERVE)
            {
                list_del_init(&node->sendlist);
                context->list.count--;
                if (NULL != node->message) {
                    coap_free(node->message);
                }
                coap_free(node);
                node = NULL;
            }
            return COAP_SUCCESS;
        }
    }
    return COAP_ERROR_NOT_FOUND;
}

static void CoAPMessage_handle(CoAPContext *context,
                               unsigned char     *buf,
                               unsigned short      datalen)
{
    int    ret  = COAP_SUCCESS;
    CoAPMessage     message;
    memset(&message, 0x00, sizeof(CoAPMessage));

    ret = CoAPDeserialize_Message(&message, buf, datalen);
    if (NULL != message.payload) {
        MOLMC_LOGD(TAG, "-----payload: ---");
        MOLMC_LOG_BUFFER_HEX("token", message.payload, message.payloadlen);
    }
    MOLMC_LOGD(TAG, "-----code   : 0x%x---", message.header.code);
    MOLMC_LOGD(TAG, "-----type   : 0x%x---", message.header.type);
    MOLMC_LOGD(TAG, "-----msgid  : %d---", message.header.msgid);
    MOLMC_LOGD(TAG, "-----opt    : %d---", message.optnum);
    /** MOLMC_LOG_BUFFER_HEX("token", message.token, 4); */

    if (COAP_SUCCESS != ret) {
        if (NULL != context->notifier) {
            /* TODO: */
            /** context->notifier(context, event); */
        }
    }

    if (COAPAckMsg(message.header)) {
        MOLMC_LOGD(TAG, "Receive CoAP ACK Message,ID %d", message.header.msgid);
        CoAPAckMessage_handle(context, &message);
    } else if (CoAPRespMsg(message.header)) {
        MOLMC_LOGD(TAG, "Receive CoAP Response Message,ID %d", message.header.msgid);
        CoAPRespMessage_handle(context, &message);
    }
}

int CoAPMessage_recv(CoAPContext *context, unsigned int timeout, int readcount)
{
    int len = 0;
    int count = readcount;

    while (1) {
        len = CoAPNetwork_read(&context->network, context->recvbuf,
                               COAP_MSG_MAX_PDU_LEN, timeout);
        /** MOLMC_LOGI("recv", "receive data: %d, readcount: %d", len,  readcount); */
        if (len > 0) {
            if(0 == readcount){
                CoAPMessage_handle(context, context->recvbuf, len);
            }
            else{
                count--;
                CoAPMessage_handle(context, context->recvbuf, len);
                if(0 == count){
                    return len;
                }
            }
        } else {
            return 0;
        }
    }
}

int CoAPMessage_cycle(CoAPContext *context)
{
    unsigned int ret = 0;
    CoAPMessage_recv(context, context->waittime, 0);

    CoAPSendNode *node = NULL, *next = NULL;
    list_for_each_entry_safe(node, next, &context->list.sendlist, sendlist, CoAPSendNode) {
        if (NULL != node) {
            if (node->timeout == 0) {
                if (node->retrans_count < COAP_MAX_RETRY_COUNT && (0 == node->acked) && node->observer==COAP_DEREGISTER_OBSERVE) {
                    node->timeout     = node->timeout_val * 2;
                    node->timeout_val = node->timeout;
                    node->retrans_count++;
                    MOLMC_LOGD("coap message", "Retansmit the message id %d len %d", node->msgid, node->msglen);
                    ret = CoAPNetwork_write(&context->network, node->message, node->msglen);
                    if (ret != COAP_SUCCESS) {
                        if (NULL != context->notifier) {
                            /* TODO: */
                            /* context->notifier(context, event); */
                        }
                    }
                }

                if ((node->timeout > COAP_MAX_TRANSMISSION_SPAN) ||
                    (node->retrans_count >= COAP_MAX_RETRY_COUNT)) {
                    if (NULL != context->notifier) {
                        /* TODO: */
                        /* context->notifier(context, event); */
                    }

                    if(node->observer == COAP_DEREGISTER_OBSERVE) {
                        /*Remove the node from the list*/
                        list_del_init(&node->sendlist);
                        context->list.count--;
                        MOLMC_LOGI(TAG, "Retransmit timeout,remove the message id %d count %d",
                                  node->msgid, context->list.count);
                        coap_free(node->message);
                        coap_free(node);
                        break;
                    }
                }
            }
             else {
                node->timeout--;
            }
        }
    }
    return COAP_SUCCESS;
}

