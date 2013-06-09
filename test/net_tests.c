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
