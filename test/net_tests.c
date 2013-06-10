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

#include "minunit.h"

char *test_bt_local_addr() {
  unsigned short port = 321;
  struct sockaddr_in addr = bt_local_addr(port);

  mu_assert("ip socket", addr.sin_family == AF_INET);
  mu_assert("socket port", addr.sin_port == htons(port));
  mu_assert("incomming address", addr.sin_addr.s_addr == htonl(INADDR_ANY));

  return NULL;
}

char *test_bt_req_from_network() {
  bt_req_t req = {.connection_id = htonll(3),
                  .action = htonl(3),
                  .transaction_id = htonl(3)};

  bt_req_from_network(&req);

  mu_assert("req.connection_id should be in host byte order", req.connection_id == 3LL);
  mu_assert("req.action should be in host byte order", req.action == 3);
  mu_assert("req.transaction_id should be in host byte order", req.transaction_id == 3);

  return NULL;
}

char *test_bt_conn_resp_to_network() {
  bt_connection_resp_t resp = {.action = 3,
                               .transaction_id = 3,
                               .connection_id = 3};

  bt_conn_resp_to_network(&resp);

  mu_assert("a", resp.action == htonl(3));
  mu_assert("b", resp.transaction_id == htonl(3));
  mu_assert("c", resp.connection_id == htonll(3));

  return NULL;
}

char *all_tests() {
  mu_run_test(test_bt_local_addr);
  mu_run_test(test_bt_req_from_network);
  mu_run_test(test_bt_conn_resp_to_network);

  return NULL;
}
