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

/* Prepares a `bt_req` object to be sent over the wire. */
void bt_resp_to_network(bt_resp_t *resp);

int bt_ipv4_udp_sock(uint16_t port, struct addrinfo **addrinfo) {
  struct addrinfo hints = {
    .ai_family = AF_INET,      // IPv4
    .ai_socktype = SOCK_DGRAM,
    .ai_flags = AI_PASSIVE
  };

  char portstr[10];
  sprintf(portstr, "%d", port);

  if (getaddrinfo(NULL, portstr, &hints, addrinfo) != 0) {
    syslog(LOG_ERR, "Error in getaddrinfo(). Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  int sock;
  if ((sock = socket((*addrinfo)->ai_family, (*addrinfo)->ai_socktype, (*addrinfo)->ai_protocol)) == -1) {
    syslog(LOG_ERR, "Cannot create socket. Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  return sock;
}

void bt_req_to_host(bt_req_t *req) {
  req->connection_id = ntohll(req->connection_id);
  req->action = ntohl(req->action);
  req->transaction_id = ntohl(req->transaction_id);
}

void bt_resp_to_network(bt_resp_t *resp) {
  resp->action = htonl(resp->action);
  resp->transaction_id = htonl(resp->transaction_id);
}

void bt_connection_resp_to_network(bt_connection_resp_t *resp) {
  bt_resp_to_network((bt_resp_t *) resp);

  resp->connection_id = htonll(resp->connection_id);
}

void bt_announce_req_to_host(bt_announce_req_t *req) {

  // Not needed to convert the basic request fields; they are converted before
  // the request is handled.
  // bt_req_to_host((bt_req_t *) req);

  // Not needed to convert these fields, as they are not multibyte numbers
  // req->info_hash
  // req->peer_id

  req->downloaded = ntohll(req->downloaded);
  req->left = ntohll(req->left);
  req->uploaded = ntohll(req->uploaded);
  req->event = ntohl(req->event);
  req->ipv4_addr = ntohl(req->ipv4_addr);
  req->key = ntohl(req->key);
  req->num_want = ntohl(req->num_want);
  req->port = ntohs(req->port);
}

void bt_announce_resp_to_network(bt_announce_resp_t *resp) {
  bt_resp_to_network((bt_resp_t *) resp);

  resp->interval = htonl(resp->interval);
  resp->leechers = htonl(resp->leechers);
  resp->seeders = htonl(resp->seeders);
}

void bt_announce_peer_addr_to_network(bt_peer_addr_t *peer_addr) {
  peer_addr->ipv4_addr = htonl(peer_addr->ipv4_addr);
  peer_addr->port = htons(peer_addr->port);
}
