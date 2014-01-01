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

/* Configuration data. */
typedef struct {

  // Misc options
  uint16_t bttracker_port;
  uint16_t bttracker_num_threads;
  bool bttracker_debug_mode;

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
} bt_config_t;

/* Peer information. */
typedef struct {
  int32_t key;        // Random key sent by the client
  int64_t downloaded; // Number of bytes downloaded so far
  int64_t uploaded;   // Number of bytes uploaded so far
  int64_t left;       // Number of bytes left to be downloaded
  uint16_t port;      // Peer port
  uint32_t ipv4_addr; // Peer IPv4 address
} bt_peer_t;

/*
 * Configuration.
 */
bool bt_load_config(const char *filename, bt_config_t *config);


/*
 * Redis.
 */

/* Connects to the specified redis instance. */
redisContext *bt_redis_connect(const char *host, int port, long timeout, int db);

/* Pings the Redis instance and returns true if it everything's okay. */
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

/* Increments the number of times a torrent has been downloaded. */
void bt_increment_downloads(redisContext *redis, const bt_config_t *config,
                            const int8_t *info_hash);


/*
 * Peer management.
 */

/* Creates a new `bt_peer_t` containing the peer data from announce request. */
bt_peer_t *bt_new_peer(bt_announce_req_t *request, uint32_t sockaddr);

/* Creates a new `bt_peer_addr_t` with the peer address data. */
bt_peer_addr_t *bt_new_peer_addr(uint32_t ipv4_addr, uint16_t port);

/* Inserts a peer (seeder or leecher) to the swarm of a torrent. */
void bt_insert_peer(redisContext *redis, const bt_config_t *config,
                    const int8_t *info_hash, const int8_t *peer_id,
                    const bt_peer_t *peer_data, bool is_seeder);

/* Removes a peer from the swarm of a torrent. */
void bt_remove_peer(redisContext *redis, const bt_config_t *config,
                    const int8_t *info_hash, const int8_t *peer_id,
                    bool is_seeder);

/* Promotes a peer from leecher to seeder. */
void bt_promote_peer(redisContext *redis, const bt_config_t *config,
                     const int8_t *info_hash, const int8_t *peer_id);

/* Return the number of peers (leechers or seeders) on the swarm. */
int32_t bt_peer_count(redisContext *redis, const bt_config_t *config,
                      const int8_t *info_hash, bool seeder);

/* Returns a random list containing a random subset of leechers or seeders. */
bt_list_t *bt_peer_list(redisContext *redis, const bt_config_t *config,
                        const int8_t *info_hash, int32_t num_want,
                        int *peer_count, bool seeder);

#endif // BTTRACKER_DATA_H_
