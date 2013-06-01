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

int main(int argc, char *argv[]) {

  /* Syslog configuration. */
  // setlogmask(LOG_UPTO(LOG_ERR));
  openlog(PACKAGE, LOG_PID | LOG_PERROR, LOG_LOCAL0);

  syslog(LOG_INFO, "Welcome, version %s", PACKAGE_VERSION);

  syslog(LOG_DEBUG, "Creating hash table for active connections");
  static pthread_mutex_t conn_mutex = PTHREAD_MUTEX_INITIALIZER;
  bt_concurrent_connection_table_t conn_table = {.self = bt_new_connection_table(),
                                                 .mutex = &conn_mutex};

  /* Thread that removes inactive connections every few seconds. */
  pthread_t connection_purge_thread;
  bt_connection_purge_data_t purge_thread_data = {.interrupted = false,
                                                  .table = &conn_table};

  /* Required by network communication code. */
  struct sockaddr_in si_me, si_other;
  socklen_t me_len, other_len;
  int sock, reqlen;
  char buff[BT_RECV_BUFLEN];

  syslog(LOG_DEBUG, "Creating UDP socket");
  if ((sock = BT_SERVER_SOCK()) == -1) {
    syslog(LOG_ERR, "Cannot create socket. Exiting");
    exit(1);
  }

  /* Function that is executed when the signal SIGINT is received. */
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

  /* Handle interruption signal (C-c on term). */
  signal(SIGINT, on_sigint);

  /* Local address where the UDP socket will bind against. */
  si_me = bt_local_addr(BT_UDP_PORT);
  me_len = sizeof(si_me), other_len = sizeof(si_other);

  syslog(LOG_DEBUG, "Binding UDP socket to local port %" PRId32, BT_UDP_PORT);
  if (bind(sock, (struct sockaddr *) &si_me, me_len) == -1) {
    syslog(LOG_ERR, "Error in bind(). Exiting");
    exit(2);
  }

  syslog(LOG_DEBUG, "Starting connection purging thread");
  pthread_create(&connection_purge_thread, NULL, bt_clear_old_connections, &purge_thread_data);

  while (true) {
    if ((reqlen = recvfrom(sock, buff, BT_RECV_BUFLEN, 0,
                           (struct sockaddr *) &si_other, &other_len)) == -1) {
      syslog(LOG_ERR, "Error in recvfrom(). Exiting");
      exit(3);
    }

    /* Get the basic request data. */
    bt_req_t *request = (bt_req_t *) buff;

    /* Convert numbers from network byte order to host byte order. */
    bt_req_from_network(request);

    syslog(LOG_DEBUG, "Datagram received. Action = %" PRId32
           ", Connection ID = %" PRId64 ", Transaction ID = %" PRId32,
           request->action, request->connection_id, request->transaction_id);

    /* Dispatch the request to the appropriate handler function. */
    if (request->action == BT_ACTION_CONNECT) {
      bt_connection_data_t data = {.request = request, .sock = sock,
                                   .client_addr = &si_other,
                                   .client_addr_len = other_len,
                                   .table = &conn_table};

      bt_handle_connection(&data);
    }
  }
}
