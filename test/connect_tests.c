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
