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

void
write_tmp_config(char *filename)
{
  const char *template = "/tmp/bttracker.conf.XXXXXX";

  strcpy(filename, template);
  int fd = mkstemp(filename);

  const char *text = "[BtTracker]\n\
LogLevel=INFO\n\
Address=0.0.0.0\n\
Port=1234\n\
\n\
[Threading]\n\
MaxThreads=4\n\
MaxIdleTime=300\n\
\n\
[Announce]\n\
InfoHashRestriction=none\n\
WaitTime=1800\n\
PeerTTL=1920\n\
MaxNumWant=80\n\
\n\
[Redis]\n\
SocketPath=/tmp/redis.sock\n\
Host=127.0.0.1\n\
Port=6379\n\
Timeout=500\n\
DB=1\n\
KeyPrefix=bttracker";

  write(fd, text, strlen(text));
  close(fd);
}

char *
test_load_config_valid_file()
{
  bt_config_t config;

  char filename[PATH_MAX];
  write_tmp_config(filename);

  bool succeeded = bt_load_config(filename, &config);
  unlink(filename);

  mu_assert("error, expected return value to be true", succeeded == true);

  mu_assert("error, unexpected bttracker_addr", strcmp(config.bttracker_addr, "0.0.0.0") == 0);
  mu_assert("error, unexpected bttracker_port", config.bttracker_port == 1234);
  mu_assert("error, unexpected bttracker_log_level_mask", config.bttracker_log_level_mask == LOG_INFO);

  mu_assert("error, unexpected thread_max", config.thread_max == 4);
  mu_assert("error, unexpected thread_max_idle_time", config.thread_max_idle_time == 300);

  mu_assert("error, unexpected announce_wait_time", config.announce_wait_time == 1800);
  mu_assert("error, unexpected announce_peer_ttl", config.announce_peer_ttl == 1920);
  mu_assert("error, unexpected announce_max_numwant", config.announce_max_numwant == 80);

  mu_assert("error, unexpected redis_socket_path", strcmp(config.redis_socket_path, "/tmp/redis.sock") == 0);
  mu_assert("error, unexpected redis_host", strcmp(config.redis_host, "127.0.0.1") == 0);
  mu_assert("error, unexpected redis_port", config.redis_port == 6379);
  mu_assert("error, unexpected redis_timeout", config.redis_timeout == 500);
  mu_assert("error, unexpected redis_db", config.redis_db == 1);
  mu_assert("error, unexpected redis_key_prefix", strcmp(config.redis_key_prefix, "bttracker") == 0);
  mu_assert("error, unexpected info_hash_restriction", config.info_hash_restriction== BT_RESTRICTION_NONE);

  return NULL;
}

char *
test_load_config_invalid_file()
{
  bt_config_t config;
  const char *filename = "bttracker.conf.invalid";

  bool succeeded = bt_load_config(filename, &config);
  mu_assert("error, expected return value to be false", succeeded == false);

  return NULL;
}

char *
all_tests()
{
  mu_run_test(test_load_config_valid_file);
  mu_run_test(test_load_config_invalid_file);

  return NULL;
}
