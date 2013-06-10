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

char *test_bt_current_timestamp() {
  int64_t now = time(NULL);
  int64_t ts  = bt_current_timestamp();

  mu_assert("current UNIX timestamp", abs(ts - now) <= 1);

  return NULL;
}

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
  mu_run_test(test_bt_current_timestamp);
  mu_run_test(test_bt_expired);
  mu_run_test(test_bt_now_expired);

  return NULL;
}
