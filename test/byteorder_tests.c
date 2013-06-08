#include "minunit.h"

#define INT_HOST 4497486125440
#define INT_NET  -9216317402361102336

char *test_ntohll() {
  int64_t result = ntohll(INT_NET);
  mu_assert("invert network byte order to host byte order", result == INT_HOST);

  return NULL;
}

char *test_htonll() {
  int64_t result = htonll(INT_HOST);
  mu_assert("invert host byte order to network byte order", result == INT_NET);

  return NULL;
}

char *all_tests() {
  mu_run_test(test_ntohll);
  mu_run_test(test_htonll);

  return NULL;
}
