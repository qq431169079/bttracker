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

bool
bt_load_config(const char *filename, bt_config_t *config)
{
  GKeyFile *keyfile;
  GKeyFileFlags flags;
  GError *error = NULL;

  /* Creates a new GKeyFile object and a bitwise list of flags. */
  keyfile = g_key_file_new ();
  flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

  /* Loads the GKeyFile from filename or return. */
  if (!g_key_file_load_from_file (keyfile, filename, flags, &error)) {
    syslog(LOG_ERR, "Cannot load config file: %s", error->message);
    return false;
  }

  /* Fills the config data struct, one section at a time. */
  config->bttracker_addr          =
    g_key_file_get_string (keyfile, "BtTracker", "Address", NULL);
  config->bttracker_port          =
    g_key_file_get_integer(keyfile, "BtTracker", "Port", NULL);
  config->thread_max              =
    g_key_file_get_integer(keyfile, "Threading", "MaxThreads", NULL);
  config->thread_max_idle_time    =
    g_key_file_get_integer(keyfile, "Threading", "MaxIdleTime", NULL);
  config->announce_wait_time      =
    g_key_file_get_integer(keyfile, "Announce",  "WaitTime", NULL);
  config->announce_peer_ttl       =
    g_key_file_get_integer(keyfile, "Announce",  "PeerTTL", NULL);
  config->announce_max_numwant    =
    g_key_file_get_integer(keyfile, "Announce",  "MaxNumWant", NULL);

  char *info_hash_restriction_str =
    g_key_file_get_string(keyfile,  "Announce",  "InfoHashRestriction", NULL);

  if (strcmp(info_hash_restriction_str, "whitelist") == 0) {
    config->info_hash_restriction = BT_RESTRICTION_WHITELIST;
  } else if (strcmp(info_hash_restriction_str, "blacklist") == 0) {
    config->info_hash_restriction = BT_RESTRICTION_BLACKLIST;
  } else {
    config->info_hash_restriction = BT_RESTRICTION_NONE;
  }

  char *log_level_str =
    g_key_file_get_string (keyfile, "BtTracker", "LogLevel", NULL);

  if (strcmp(log_level_str, "DEBUG") == 0) {
    config->bttracker_log_level_mask = LOG_DEBUG;
  } else if (strcmp(log_level_str, "INFO") == 0) {
    config->bttracker_log_level_mask = LOG_INFO;
  } else if (strcmp(log_level_str, "NOTICE") == 0) {
    config->bttracker_log_level_mask = LOG_NOTICE;
  } else if (strcmp(log_level_str, "WARNING") == 0) {
    config->bttracker_log_level_mask = LOG_WARNING;
  } else if (strcmp(log_level_str, "ERR") == 0) {
    config->bttracker_log_level_mask = LOG_ERR;
  } else if (strcmp(log_level_str, "CRIT") == 0) {
    config->bttracker_log_level_mask = LOG_CRIT;
  } else if (strcmp(log_level_str, "ALERT") == 0) {
    config->bttracker_log_level_mask = LOG_ALERT;
  } else if (strcmp(log_level_str, "EMERG") == 0) {
    config->bttracker_log_level_mask = LOG_EMERG;
  } else {
    config->bttracker_log_level_mask = LOG_INFO;
  }

  free(info_hash_restriction_str);
  free(log_level_str);

  config->redis_socket_path =
    g_key_file_get_string(keyfile,  "Redis", "SocketPath", NULL);
  config->redis_host        =
    g_key_file_get_string(keyfile,  "Redis", "Host", NULL);
  config->redis_port        =
    g_key_file_get_integer(keyfile, "Redis", "Port", NULL);
  config->redis_timeout     =
    g_key_file_get_integer(keyfile, "Redis", "Timeout", NULL);
  config->redis_db          =
    g_key_file_get_integer(keyfile, "Redis", "DB", NULL);
  config->redis_key_prefix  =
    g_key_file_get_string(keyfile,  "Redis", "KeyPrefix", NULL);

  g_key_file_free(keyfile);

  return true;
}
