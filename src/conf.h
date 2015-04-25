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

#ifndef BTTRACKER_CONF_H_
#define BTTRACKER_CONF_H_

/* Flags that identify whether some event is ellegible to be blacklisted. */
typedef enum {
  BT_RESTRICTION_NONE,
  BT_RESTRICTION_WHITELIST,
  BT_RESTRICTION_BLACKLIST
} bt_restriction;

/* Configuration data. */
typedef struct {

  // Misc options
  char *bttracker_addr;
  uint16_t bttracker_port;
  bool bttracker_debug_mode;

  // Threading options
  uint16_t thread_max;
  uint32_t thread_max_idle_time;

  // Announce options
  uint32_t announce_wait_time;
  uint32_t announce_peer_ttl;
  uint16_t announce_max_numwant;

  // Redis options
  char *redis_host;
  uint16_t redis_port;
  uint32_t redis_timeout;
  uint16_t redis_db;
  char *redis_key_prefix;

  // Blacklist options
  bt_restriction info_hash_restriction;
} bt_config_t;

/* Loads configuration file to a `bt_config_t` object. */
bool
bt_load_config(const char *filename, bt_config_t *config);

#endif // BTTRACKER_CONF_H_
