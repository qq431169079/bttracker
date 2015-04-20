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

/* Buffer length used to receive data from clients. */
#define BT_RECV_BUFLEN 1024

/* 64-bit integer that identifies the UDP-based tracker protocol. */
#define BT_PROTOCOL_ID 0x41727101980LL

/* Fills request struct with buffer data. */
void bt_read_request_data(const char *buffer, bt_req_t *req);

/* Writes the error response data to an output buffer. */
void bt_write_error_data(char *resp_buffer, bt_req_t *req, const char* msg);

/* Writes the connection response data to an output buffer. */
void bt_write_connection_data(char *buffer, bt_connection_resp_t *resp);

/* Fills the announce request with buffer data. */
void bt_read_announce_request_data(const char *buffer, bt_announce_req_t *req);

/* Writes the announce response data to an output buffer. */
void bt_write_announce_response_data(char *resp_buffer, bt_announce_resp_t *resp);

/* Writes the peer data to be sent along the announce response. */
void bt_write_announce_peer_data(char *resp_buffer, bt_list_t *peers);

/* Fills the scrape request with buffer data. */
void bt_read_scrape_request_data(char *buffer, size_t buflen, bt_scrape_req_t *req);

/* Writes the scrape response data to an output buffer. */
void bt_write_scrape_response_data(char *resp_buffer, bt_scrape_resp_t *resp);

/* Fills a `struct addrinfo` and returns a corresponding UDP socket. */
int bt_ipv4_udp_sock(const char *addr, uint16_t port, struct addrinfo **addrinfo);

#endif // BTTRACKER_NET_H_
