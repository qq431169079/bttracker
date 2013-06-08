#include "minunit.h"

char *test_bt_expired() {
  int64_t begin = 10, end = 15;

  mu_assert("not expired (begin < end)", bt_expired(begin, end, 5) == false);
  mu_assert("not expired (begin > end)", bt_expired(end, begin, 5) == false);

  mu_assert("expired (begin < end)", bt_expired(begin, end, 4) == true);
  mu_assert("expired (begin > end)", bt_expired(end, begin, 4) == true);

  return NULL;
}

char *test_bt_now_expired() {
  int64_t ts = bt_current_timestamp() - 10;

  mu_assert("not expired", bt_now_expired(ts, 15) == false);
  mu_assert("expired", bt_now_expired(ts, 5) == true);

  return NULL;
}

char *all_tests() {
  mu_run_test(test_bt_expired);
  mu_run_test(test_bt_now_expired);

  return NULL;
}
