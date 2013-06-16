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

#ifndef BTTRACKER_CONNECT_H_
#define BTTRACKER_CONNECT_H_

/* Interval, in seconds, in which a connection ID is considered active. */
#define BT_ACTIVE_CONNECTION_TTL 120

/* Interval, in seconds, in which inactive connections will be purged. */
#define BT_CONNECTION_PURGE_INTERVAL 10

/* Type used to store the active connections. */
typedef GHashTable bt_connection_table_t;

/* Object that holds the hash table and its mutex together. */
typedef struct {
  bt_connection_table_t *self; // Hash table of active connections.
  pthread_mutex_t *mutex;      // Mutex used to control concurrent access.
} bt_concurrent_connection_table_t;

/* Input value for `bt_handle_connection` function. */
typedef struct {
  bt_req_t *request;                       // Incoming request data.
  int sock;                                // Socket handling the connection.
  struct sockaddr_in *client_addr;         // Source address of the request.
  socklen_t client_addr_len;               // sizeof(*client_addr).
  bt_concurrent_connection_table_t *table; // Active connections so far.
} bt_connection_data_t;

/* Object used as input to the connection purging thread. */
typedef struct {
  bool interrupted;                        // Whether it should be interrupted.
  bt_concurrent_connection_table_t *table; // Active connections so far.
  int connection_ttl;                      // Connection is valid for that many seconds.
  int purge_interval;                      // Purging interval, in seconds.
} bt_connection_purge_data_t;

/* Creates a new hash table to store active connection IDs. */
bt_connection_table_t *bt_new_connection_table();

/* Destroys the connection hash table object. */
void bt_free_concurrent_connection_table(bt_concurrent_connection_table_t *table);

/* Adds a new connection ID to the hash table of active connections. */
void bt_add_connection(bt_connection_table_t *table, int64_t connection_id);

/* Returns whether the incoming connection request should be accepted. */
bool bt_valid_request(bt_connection_table_t *table, const bt_req_t *req);

/* Thread that handles a connection request. */
void *bt_handle_connection(void *data);

/*
 * Thread that purges all connections older than 2 minutes. The argument `data`
 * is a pointer to a `bt_connection_purge_data_t` object.
 */
void *bt_clear_old_connections(void *data);

#endif // BTTRACKER_CONNECT_H_
