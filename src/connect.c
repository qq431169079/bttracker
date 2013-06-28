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

/* Function used to free keys and values from the connection hash table. */
void bt_free_hash_table_key_val(gpointer data);

/* Returns whether a specific hash table entry should be expired. */
gboolean bt_hash_table_old_connection(gpointer key, gpointer val, gpointer data);


bt_hash_table_t *bt_new_connection_table() {

  /* Keys and values of a connection hash table are 64-bit integers. */
  return g_hash_table_new_full(g_int64_hash, g_int64_equal,
                               bt_free_hash_table_key_val,
                               bt_free_hash_table_key_val);
}

void bt_add_connection(bt_hash_table_t *table, int64_t connection_id) {
  int64_t *key, *val;

  /* Store key and value into the heap. */
  key = (int64_t *) malloc(sizeof(int64_t)),
    val = (int64_t *) malloc(sizeof(int64_t));

  /* Check memory allocation result. */
  if (key == NULL || val == NULL) {
    syslog(LOG_ERR, "Cannot allocate memory for new connection");
    exit(BT_EXIT_MALLOC_ERROR);
  }

  /* Store the connection into the hash table. */
  *key = connection_id, *val = bt_current_timestamp();
  g_hash_table_insert(table, key, val);

  syslog(LOG_DEBUG, "Registered new connection, ID = %" PRId64, connection_id);
}

bool bt_valid_request(bt_hash_table_t *table, const bt_req_t *req) {
  bt_action action = req->action;

  if (action == BT_ACTION_CONNECT) {

    /* A connection is valid when its connection ID matches the protocol ID. */
    return req->connection_id == BT_PROTOCOL_ID;
  }

  /* Other types of requests are unsupported for now. */
  return false;
}

void bt_free_connection_response(void *resp) {
  free(resp);
}

int bt_handle_connection(bt_req_t *request, bt_connection_resp_t *out,
                         bt_concurrent_hashtable_t *table) {
  syslog(LOG_DEBUG, "Handling incoming connection");

  if (!bt_valid_request(table->self, request)) {
    syslog(LOG_ERR, "Invalid connection data. Ignoring request");
    return -1;
  }

  /* According to the spec, the connection ID must be a random 64-bit int. */
  int64_t connection_id = bt_random_int64();

  /* Add connection ID to the table of active connections. */
  pthread_mutex_lock(table->mutex);
  if (table->self != NULL) {
    bt_add_connection(table->self, connection_id);
  }
  pthread_mutex_unlock(table->mutex);

  /* Response data to this connection request. */
  out->action = request->action;
  out->transaction_id = request->transaction_id;
  out->connection_id = connection_id;

  /* Convert the response data to network byte order. */
  bt_conn_resp_to_network(out);

  return sizeof(*out);
}

void *bt_clear_old_connections(void *data) {
  const bt_connection_purge_data_t *in = (bt_connection_purge_data_t *) data;

  while (true) {
    int steps = 0;

    /* Block this thread for a few seconds. */
    while (steps++ < in->purge_interval) {
      sleep(1);

      /* Return if this thread is interrupted in the meantime. */
      if (in->interrupted) {
        syslog(LOG_DEBUG, "Interrupting connection purging thread");
        return NULL;
      }
    }

    /* Remove old connection IDs. */
    pthread_mutex_lock(in->table->mutex);
    if (in->table->self != NULL) {
      g_hash_table_foreach_remove(in->table->self, bt_hash_table_old_connection, (gpointer) &in->connection_ttl);
    }
    pthread_mutex_unlock(in->table->mutex);
  }

  return NULL;
}

void bt_free_hash_table_key_val(gpointer data) {
  free((int64_t *) data);
}

gboolean bt_hash_table_old_connection(gpointer key, gpointer val, gpointer data) {
  int *connection_ttl = (int *) data;

  /* Keys and values are 64-bit integers. */
  int64_t timestamp = bt_current_timestamp();
  const int64_t *added_at = (int64_t *) val;

  /* Check if this connection ID exceeded TTL. */
  bool should_expire = bt_expired(*added_at, timestamp, *connection_ttl);

  if (should_expire) {
    syslog(LOG_DEBUG, "Expiring Connection ID %" PRId64, *((int64_t *) key));
  }

  return should_expire;
}
