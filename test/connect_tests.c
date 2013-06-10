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

/* Initializes a local mutex. */
#define INIT_MUTEX() \
  pthread_mutex_t mutex; \
  pthread_mutex_init(&mutex, NULL);

/* Creates a new empty connection table. */
bt_concurrent_connection_table_t new_table(pthread_mutex_t *mutex);

char *test_bt_new_connection_table() {
  INIT_MUTEX();
  bt_concurrent_connection_table_t table = new_table(&mutex);

  mu_assert("glib hash table allocation", table.self != NULL);
  mu_assert("hash table should be empty", g_hash_table_size(table.self) == 0);

  bt_free_concurrent_connection_table(&table);

  return NULL;
}

char *test_bt_add_connection() {
  INIT_MUTEX();
  bt_concurrent_connection_table_t table = new_table(&mutex);
  int64_t connection_id = 123456LL;
  int64_t now_ts = bt_current_timestamp(), *add_ts;

  bt_add_connection(table.self, connection_id);
  add_ts = g_hash_table_lookup(table.self, &connection_id);

  mu_assert("hash table should contain 1 item", g_hash_table_size(table.self) == 1);
  mu_assert("key should be the given connection_id", &add_ts != NULL);
  mu_assert("value should be the current timestamp", abs(now_ts - *add_ts) <= 1);

  bt_free_concurrent_connection_table(&table);

  return NULL;
}

char *all_tests() {
  mu_run_test(test_bt_new_connection_table);
  mu_run_test(test_bt_add_connection);

  return NULL;
}

bt_concurrent_connection_table_t new_table(pthread_mutex_t *mutex) {
  bt_connection_table_t *table = bt_new_connection_table();
  return (bt_concurrent_connection_table_t) {.self = table, .mutex = mutex};
}
