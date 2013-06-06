#include "minunit.h"

char *test_demo() {
  mu_assert("timestamp", bt_current_timestamp() > 0);
  return 0;
}

char *all_tests() {
  mu_run_test(test_demo);
  return 0;
}
