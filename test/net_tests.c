#include "minunit.h"

char *test_bt_req_from_network() {
  int32_t net_int = htonl(3);
  int64_t net_lng = htonll(3);

  bt_req_t req = {.connection_id = net_lng,
                  .action = net_int,
                  .transaction_id = net_int};

  bt_req_from_network(&req);

  mu_assert("req.connection_id should be in host byte order", req.connection_id == 3LL);
  mu_assert("req.action should be in host byte order", req.action == 3);
  mu_assert("req.transaction_id should be in host byte order", req.transaction_id == 3);

  return NULL;
}

char *all_tests() {
  mu_run_test(test_bt_req_from_network);

  return NULL;
}
