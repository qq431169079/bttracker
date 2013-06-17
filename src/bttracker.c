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

/* Mutexes used to access shared data in a thread safe way. */
pthread_mutex_t conn_mutex;

/* Object where all active connections are kept. */
bt_concurrent_connection_table_t conn_table;

/* Socket used to receive data from clients. */
int sock;

/* Thread that periodically purges old connections. */
bt_connection_purge_data_t purge_thread_data;
pthread_t connection_purge_thread;

/* Function that is executed when the signal SIGINT is received. */
void on_sigint(int signum);

int main(int argc, char *argv[]) {

  /* Syslog configuration. */
  // setlogmask(LOG_UPTO(LOG_ERR));
  openlog(PACKAGE, LOG_PID | LOG_PERROR, LOG_LOCAL0);

  syslog(LOG_INFO, "Welcome, version %s", PACKAGE_VERSION);

  syslog(LOG_DEBUG, "Creating hash table for active connections");
  conn_mutex = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  conn_table = (bt_concurrent_connection_table_t) {
    .self = bt_new_connection_table(),
    .mutex = &conn_mutex
  };

  /* Required by network communication code. */
  struct sockaddr_in si_me, si_other;
  socklen_t me_len, other_len;

  syslog(LOG_DEBUG, "Creating UDP socket");
  if ((sock = BT_SERVER_SOCK()) == -1) {
    syslog(LOG_ERR, "Cannot create socket. Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  /* Handle interruption signal (C-c on term). */
  signal(SIGINT, on_sigint);

  /* Local address where the UDP socket will bind against. */
  si_me = bt_local_addr(BT_UDP_PORT);
  me_len = sizeof(si_me), other_len = sizeof(si_other);

  syslog(LOG_DEBUG, "Binding UDP socket to local port %" PRId32, BT_UDP_PORT);
  if (bind(sock, (struct sockaddr *) &si_me, me_len) == -1) {
    syslog(LOG_ERR, "Error in bind(). Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  syslog(LOG_DEBUG, "Starting connection purging thread");
  purge_thread_data = (bt_connection_purge_data_t) {
    .interrupted = false,
    .table = &conn_table,
    .connection_ttl = BT_ACTIVE_CONNECTION_TTL,
    .purge_interval = BT_CONNECTION_PURGE_INTERVAL
  };
  pthread_create(&connection_purge_thread, NULL, bt_clear_old_connections,
                 &purge_thread_data);

  while (true) {
    char buff[BT_RECV_BUFLEN];

    if (recvfrom(sock, buff, BT_RECV_BUFLEN, 0, (struct sockaddr *) &si_other,
                 &other_len) == -1) {
      syslog(LOG_ERR, "Error in recvfrom(). Exiting");
      exit(BT_EXIT_NETWORK_ERROR);
    }

    bt_req_t *request = (bt_req_t *) buff;

    /* Convert numbers from network byte order to host byte order. */
    bt_req_from_network(request);

    syslog(LOG_DEBUG, "Datagram received. Action = %" PRId32
           ", Connection ID = %" PRId64 ", Transaction ID = %" PRId32,
           request->action, request->connection_id, request->transaction_id);

    /* Dispatch the request to the appropriate handler function. */
    if (request->action == BT_ACTION_CONNECT) {
      bt_connection_data_t data = {
        .request = request,
        .sock = sock,
        .client_addr = &si_other,
        .client_addr_len = other_len,
        .table = &conn_table
      };

      bt_handle_connection(&data);
    }
  }
}

void on_sigint(int signum) {
  syslog(LOG_DEBUG, "Freeing resources");

  /* Interrupt connection purging thread. */
  purge_thread_data.interrupted = true;
  pthread_join(connection_purge_thread, NULL);

  /* Destroy connection hash table. */
  bt_free_concurrent_connection_table(&conn_table);

  /* Close UDP socket. */
  close(sock);

  syslog(LOG_INFO, "Exiting");
  closelog();

  /* Resume signal's default behavior. */
  signal(signum, SIG_DFL);
  raise(signum);
}
