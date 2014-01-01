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

/* Redis connection handler. */
redisContext *redis;

/* Configuration data. */
bt_config_t config;

/* Input socket descriptors. */
int in_sock;
struct addrinfo *in_addrinfo;

/* Function that is executed when the signal SIGINT is received. */
void on_sigint(int signum);

int main(int argc, char *argv[]) {

  /* Seed the pseudo-random number generator. */
  srand(time(NULL));

  /* Default logging level. */
  setlogmask(LOG_UPTO(LOG_INFO));

  openlog(PACKAGE, LOG_PID | LOG_PERROR | LOG_CONS, LOG_LOCAL0);
  syslog(LOG_INFO, "Welcome to %s, version %s", PACKAGE_NAME, PACKAGE_VERSION);

  if (argc != 2) {
    syslog(LOG_ERR, "Please specify the configuration file. Usage: %s <config_file>", PACKAGE_NAME);
    exit(BT_EXIT_CONFIG_ERROR);
  }

  /* Reads the configuration file. */
  if (!bt_load_config(argv[1], &config)) {
    exit(BT_EXIT_CONFIG_ERROR);
  }

  if (config.bttracker_debug_mode) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  /* Connects to a Redis instance where the data should be stored. */
  redis = bt_redis_connect(config.redis_host, config.redis_port, config.redis_timeout * 1000, config.redis_db);

  /* Handle interruption signal (C-c on term). */
  signal(SIGINT, on_sigint);

  /* Required by network communication code. */
  struct sockaddr_in si_other;
  socklen_t other_len = sizeof(si_other);

  /* Local address where the UDP server socket will bind against. */
  syslog(LOG_DEBUG, "Creating UDP server socket");
  in_sock = bt_ipv4_udp_sock(config.bttracker_port, &in_addrinfo);

  syslog(LOG_DEBUG, "Binding UDP socket to local port %d", config.bttracker_port);
  if (bind(in_sock, in_addrinfo->ai_addr, in_addrinfo->ai_addrlen) == -1) {
    syslog(LOG_ERR, "Error in bind(). Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  while (true) {
    char buff[BT_RECV_BUFLEN];

    if (recvfrom(in_sock, buff, BT_RECV_BUFLEN, 0,
                 (struct sockaddr *) &si_other, &other_len) == -1) {
      syslog(LOG_ERR, "Error in recvfrom(). Exiting");
      exit(BT_EXIT_NETWORK_ERROR);
    }

    /* Get basic request data. */
    bt_req_t *request = (bt_req_t *) buff;

    /* Data to be sent to the client. */
    bt_response_buffer_t *resp_buffer = NULL;

    /* Convert numbers from network byte order to host byte order. */
    bt_req_to_host(request);

    /* Get the printable IPv4 address from the socket. */
    char ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &si_other.sin_addr, ipv4_str, INET_ADDRSTRLEN);

    syslog(LOG_DEBUG, "Datagram received from %s:%d. Action = %" PRId32
           ", Connection ID = %" PRId64 ", Transaction ID = %" PRId32,
           ipv4_str, ntohs(si_other.sin_port), request->action,
           request->connection_id, request->transaction_id);

    /* Dispatch the request to the appropriate handler function. */
    if (request->action == BT_ACTION_CONNECT) {
      resp_buffer = bt_handle_connection(request, &config, redis);
    } else if (request->action == BT_ACTION_ANNOUNCE) {
      resp_buffer = bt_handle_announce(request, &config, &si_other, redis);
    }

    if (resp_buffer != NULL) {
      syslog(LOG_DEBUG, "Sending response to matching Transaction ID %" PRId32,
             request->transaction_id);

      if (sendto(in_sock, resp_buffer->data, resp_buffer->length, 0,
                 (struct sockaddr *) &si_other, other_len) == -1) {
        syslog(LOG_ERR, "Error in sendto()");
      }

      /* Destroys response data. */
      free(resp_buffer->data);
      free(resp_buffer);
    }
  }
}

void on_sigint(int signum) {
  syslog(LOG_DEBUG, "Freeing resources");

  /* Closes the connection with Redis. */
  redisFree(redis);

  /* Close UDP socket. */
  freeaddrinfo(in_addrinfo);
  close(in_sock);

  syslog(LOG_INFO, "Exiting");
  closelog();

  /* Resume signal's default behavior. */
  signal(signum, SIG_DFL);
  raise(signum);
}
