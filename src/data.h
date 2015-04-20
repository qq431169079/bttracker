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

#ifndef BTTRACKER_DATA_H_
#define BTTRACKER_DATA_H_

/* Uses Glib's implementation of a doubly linked list. */
typedef GList bt_list_t;

/* Flags that identify whether some event is ellegible to be blacklisted. */
typedef enum {
  BT_RESTRICTION_NONE,
  BT_RESTRICTION_WHITELIST,
  BT_RESTRICTION_BLACKLIST
} bt_restriction;

/* Configuration data. */
typedef struct {

  // Misc options
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

/* Fields common to all types of requests. */
#define _BT_REQUEST_HEADER_  \
  int64_t connection_id;     \
  bt_action action;          \
  int32_t transaction_id;

/* Fields common to all types of responses. */
#define _BT_RESPONSE_HEADER_ \
  bt_action action;          \
  int32_t transaction_id;

/* Types of requests. */
typedef enum {
  BT_ACTION_CONNECT  = 0,
  BT_ACTION_ANNOUNCE = 1,
  BT_ACTION_SCRAPE   = 2,
  BT_ACTION_ERROR    = 3
} bt_action;

/* Types of announce events. */
typedef enum {
  BT_EVENT_NONE      = 0,
  BT_EVENT_COMPLETED = 1,
  BT_EVENT_STARTED   = 2,
  BT_EVENT_STOPPED   = 3
} bt_announce_event;

/* Object that contains fields common to all types of requests. */
typedef struct {
  _BT_REQUEST_HEADER_
} bt_req_t;

/* Object that contains fields common to all types of responses. */
typedef struct {
  _BT_RESPONSE_HEADER_
} bt_resp_t;

/* Data sent to the client in response to a connection request. */
typedef struct {
  _BT_RESPONSE_HEADER_
  int64_t connection_id;
} bt_connection_resp_t;

/* Data sent by the client when it's announcing itself. */
typedef struct {
  _BT_REQUEST_HEADER_
  uint32_t ipv4_addr;
  uint16_t port;
  int8_t info_hash[20];
  int8_t peer_id[20];
  int64_t downloaded;
  int64_t left;
  int64_t uploaded;
  int32_t key;
  int32_t num_want;
  bt_announce_event event;
} bt_announce_req_t;

/* Data sent to the client in response to an announce request. */
typedef struct {
  _BT_RESPONSE_HEADER_
  int32_t interval;
  int32_t leechers;
  int32_t seeders;
} bt_announce_resp_t;

/* Data sent by the client when it asks for torrent stats. */
typedef struct {
  _BT_REQUEST_HEADER_
  size_t info_hash_len;
  int8_t *info_hash[20];
} bt_scrape_req_t;

/* Stats about a given torrent. */
typedef struct {
  int32_t seeders;
  int32_t downloads;
  int32_t leechers;
} bt_torrent_stats_t;

/* Data sent to the client in response to a scrape request. */
typedef struct {
  _BT_RESPONSE_HEADER_
  bt_list_t *scrape_entries;
} bt_scrape_resp_t;

/* Peer address. */
typedef struct {
  int32_t ipv4_addr;
  uint16_t port;
} bt_peer_addr_t;

/* Object that holds serialized data to be transmitted over the wire. */
typedef struct {
  size_t length;
  char *data;
} bt_response_buffer_t;

/* Peer information. */
typedef struct {
  int32_t key;        // Random key sent by the client
  uint32_t ipv4_addr; // Peer IPv4 address
  int64_t downloaded; // Number of bytes downloaded so far
  int64_t uploaded;   // Number of bytes uploaded so far
  int64_t left;       // Number of bytes left to be downloaded
  uint16_t port;      // Peer port
} bt_peer_t;

/*
 * Configuration.
 */

/* Loads configuration file to a `bt_config_t` object. */
bool bt_load_config(const char *filename, bt_config_t *config);


/*
 * Redis.
 */

/* Connects to the specified redis instance. */
redisContext *bt_redis_connect(const char *host, int port, long timeout, int db);

/* Checks whether the Redis connection is still valid. */
bool bt_redis_ping(redisContext *redis);


/*
 * Connections.
 */

/* Adds a new connection ID to the hash table of active connections. */
void bt_insert_connection(redisContext *redis, const bt_config_t *config,
                          int64_t connection_id);

/* Returns whether this connection id exists. */
bool bt_connection_valid(redisContext *redis, const bt_config_t *config,
                         int64_t connection_id);


/*
 * Torrents.
 */

/* Converts the info hash byte array to string. */
void bt_bytearray_to_hexarray(int8_t *bin, size_t binsz, char **result);

/* Increments the number of times a torrent has been downloaded. */
void bt_increment_downloads(redisContext *redis, const bt_config_t *config,
                            const char *info_hash_str);

/* Returns whether the given torrent is blacklisted. */
bool bt_info_hash_blacklisted(redisContext *redis, const char *info_hash_str,
                              const bt_config_t *config);

/*
 * Peer management.
 */

/* Creates a new `bt_peer_t` containing the peer data from announce request. */
bt_peer_t *bt_new_peer(bt_announce_req_t *request, uint32_t sockaddr);

/* Creates a new `bt_peer_addr_t` with the peer address data. */
bt_peer_addr_t *bt_new_peer_addr(uint32_t ipv4_addr, uint16_t port);

/* Inserts a peer (seeder or leecher) to the swarm of a torrent. */
void bt_insert_peer(redisContext *redis, const bt_config_t *config,
                    const char *info_hash_str, const int8_t *peer_id,
                    const bt_peer_t *peer_data, bool is_seeder);

/* Removes a peer from the swarm of a torrent. */
void bt_remove_peer(redisContext *redis, const bt_config_t *config,
                    const char *info_hash_str, const int8_t *peer_id,
                    bool is_seeder);

/* Promotes a peer from leecher to seeder. */
void bt_promote_peer(redisContext *redis, const bt_config_t *config,
                     const char *info_hash_str, const int8_t *peer_id);

/* Fills `stats` with the latests stats for a torrent. */
void bt_get_torrent_stats(redisContext *redis, const bt_config_t *config,
                          const char *info_hash_str, bt_torrent_stats_t *stats);

/* Returns a random list containing a random subset of leechers or seeders. */
bt_list_t *bt_peer_list(redisContext *redis, const bt_config_t *config,
                        const char *info_hash_str, int32_t num_want,
                        int *peer_count, bool seeder);

#endif // BTTRACKER_DATA_H_
