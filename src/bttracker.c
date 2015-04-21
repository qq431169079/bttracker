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

/* Configuration data. */
bt_config_t config;

/* Thread pool used to answer all requests. */
GThreadPool *pool;

/* Input socket descriptors. */
int in_sock;
struct addrinfo *in_addrinfo;

/* Function that is executed when the signal SIGINT is received. */
void
on_sigint(int signum);

int
main(int argc, char *argv[])
{
  /* Seed the pseudo-random number generator. */
  srand((unsigned) time(NULL));

  /* Default logging level. */
  setlogmask(LOG_UPTO(LOG_INFO));

  openlog(PACKAGE, LOG_PID | LOG_PERROR | LOG_CONS, LOG_LOCAL0);
  syslog(LOG_INFO, "Welcome to %s, version %s", PACKAGE_NAME, PACKAGE_VERSION);

  if (argc != 2) {
    syslog(LOG_ERR, "Please specify the configuration file."
           " Usage: %s <config_file>", PACKAGE_NAME);
    exit(BT_EXIT_CONFIG_ERROR);
  }

  /* Reads the configuration file. */
  if (!bt_load_config(argv[1], &config)) {
    exit(BT_EXIT_CONFIG_ERROR);
  }

  if (config.bttracker_debug_mode) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
  }

  /* Handle interruption signal (C-c on term). */
  signal(SIGINT, on_sigint);

  /* Creates the thread pool. */
  pool = bt_new_request_processor_pool(&config);

  /* Required by network communication code. */
  struct sockaddr_in si_other;
  socklen_t other_len = sizeof(si_other);

  /* Local address where the UDP server socket will bind against. */
  syslog(LOG_DEBUG, "Creating UDP server socket");
  in_sock = bt_ipv4_udp_sock(config.bttracker_addr, config.bttracker_port,
                             &in_addrinfo);

  syslog(LOG_DEBUG, "Binding UDP socket to local port %d",
         config.bttracker_port);

  if (bind(in_sock, in_addrinfo->ai_addr, in_addrinfo->ai_addrlen) == -1) {
    syslog(LOG_ERR, "Error in bind(). Exiting");
    exit(BT_EXIT_NETWORK_ERROR);
  }

  while (true) {
    char buff[BT_RECV_BUFLEN];
    size_t buflen = recvfrom(in_sock, buff, BT_RECV_BUFLEN, 0,
                             (struct sockaddr *) &si_other, &other_len);

    if (-1 == buflen) {
      syslog(LOG_ERR, "Cannot retrieve data from socket. Continuing");
      continue;
    }

    char ipv4_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &si_other.sin_addr, ipv4_str, INET_ADDRSTRLEN);
    syslog(LOG_DEBUG, "Datagram received");

    /* Clone the input buffer so the processing thread can use it safely. */
    char *buff_clone = (char *) malloc(BT_RECV_BUFLEN);
    memcpy(buff_clone, buff, BT_RECV_BUFLEN);

    bt_job_params_t *params = (bt_job_params_t *)
      malloc(sizeof(bt_job_params_t));

    params->sock          = in_sock;
    params->buff          = buff_clone;
    params->buflen        = buflen;
    params->from_addr     = &si_other;
    params->from_addr_len = other_len;

    if (g_thread_pool_push(pool, params, NULL)) {
      syslog(LOG_DEBUG, "Successfully pushed job to thread pool");
    }
  }
}

void
on_sigint(int signum)
{
  signal(signum, SIG_DFL);

  syslog(LOG_DEBUG, "Freeing resources");

  /* Terminates the thread pool. */
  g_thread_pool_free(pool, true, true);

  /* Gives some time for the threads to be destroyed. */
  sleep(1);

  /* Closes UDP socket. */
  close(in_sock);
  freeaddrinfo(in_addrinfo);

  syslog(LOG_INFO, "Exiting");
  closelog();

  raise(signum);
}
