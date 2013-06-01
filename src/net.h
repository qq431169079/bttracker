/*
 * Copyright (c) 2013, BtTracker Authors
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BTTRACKER_NET_H_
#define BTTRACKER_NET_H_

/* Macro that returns a UDP-based datagram socket. */
#define BT_SERVER_SOCK() (socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))

/* Buffer length used to receive data from clients. */
#define BT_RECV_BUFLEN 1024

/* 64-bit integer that identifies the UDP-based tracker protocol. */
#define BT_PROTOCOL_ID 0x41727101980LL

/* UDP port number to listen to. */
#define BT_UDP_PORT 1234

/* Fields common to all types of requests. */
#define _BT_REQUEST_HEADER_  \
  int64_t connection_id;     \
  bt_action action;          \
  int32_t transaction_id;

/* Fields common to all types of responses. */
#define _BT_RESPONSE_HEADER_ \
  bt_action action;          \
  int32_t transaction_id;

/* Types of requests. */
typedef enum {
  BT_ACTION_CONNECT  = 0,
  BT_ACTION_ANNOUNCE = 1,
  BT_ACTION_SCRAPE   = 2,
  BT_ACTION_ERROR    = 3
} bt_action;

/* Object that contains fields common to all types of requests. */
typedef struct {
  _BT_REQUEST_HEADER_
} bt_req_t;

/* Object that contains fields common to all types of responses. */
typedef struct {
  _BT_RESPONSE_HEADER_
} bt_resp_t;

/* Data sent to the client in response to a connection request. */
typedef struct {
  _BT_RESPONSE_HEADER_
  int64_t connection_id;
} bt_connection_resp_t;

/* Returns the local address where the UDP socket will be bounded to. */
struct sockaddr_in bt_local_addr(unsigned short port);

/* Converts a bt_req object to host byte order. */
void bt_req_from_network(bt_req_t *req);

/* Converts a bt_req object to network byte order. */
void bt_req_to_network(bt_req_t *req);

/* Converts a bt_connection_resp to network byte order. */
void bt_conn_resp_to_network(bt_connection_resp_t *resp);

#endif // BTTRACKER_NET_H_
