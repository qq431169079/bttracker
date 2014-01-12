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

void bt_update_peer_list(redisContext *redis, bt_config_t *config, bt_announce_req_t *announce_request,
                         struct sockaddr_in *client_addr, char *info_hash_str, bool is_seeder) {

  bt_peer_t *peer = NULL;
  int8_t *peer_id = announce_request->peer_id;

  switch(announce_request->event) {
  case BT_EVENT_STOPPED:
    bt_remove_peer(redis, config, info_hash_str, peer_id, is_seeder);
    break;

  case BT_EVENT_COMPLETED:
    bt_promote_peer(redis, config, info_hash_str, peer_id);
    break;

  default:
    peer = bt_new_peer(announce_request, (uint32_t) ntohl(client_addr->sin_addr.s_addr));
    bt_insert_peer(redis, config, info_hash_str, peer_id, peer, is_seeder);

    free(peer);
  }
}

bt_response_buffer_t *bt_serialize_announce_response(bt_announce_resp_t* response_data,
                                                     int peer_count,
                                                     bt_list_t *peers) {

  /* Creates the object where the serialized information will be written to. */
  size_t resp_length = 20 + peer_count * 6;
  bt_response_buffer_t *resp_buffer = (bt_response_buffer_t *)
    malloc(sizeof(bt_response_buffer_t));

  if (resp_buffer == NULL) {
    syslog(LOG_ERR, "Cannot allocate memory for response buffer");
    exit(BT_EXIT_MALLOC_ERROR);
  }

  /* Serializes the response. */
  resp_buffer->length = resp_length;
  resp_buffer->data = (char *) malloc(resp_length);

  bt_write_announce_response_data(resp_buffer->data, response_data);

  syslog(LOG_DEBUG, "Sending %d peers in response", peer_count);
  bt_write_announce_peer_data(resp_buffer->data, peers);

  g_list_free_full(peers, free);

  return resp_buffer;
}

bt_response_buffer_t *bt_handle_announce(const bt_req_t *request,
                                         bt_config_t *config, const char *buff,
                                         struct sockaddr_in *client_addr,
                                         redisContext *redis) {
  bt_list_t *peers;
  int peer_count = 0;

  /* Ignores this request if it's not valid. */
  if (!bt_valid_request(redis, config, request)) {
    return NULL;
  }

  /* Unpacks the data retrieved via network socket. */
  bt_announce_req_t announce_request;
  bt_read_announce_request_data(buff, &announce_request);

  /* Uses a more user friendly representation for the info hash byte array. */
  char *info_hash_str;
  bt_bytearray_to_hexarray(announce_request.info_hash, 20, &info_hash_str);

  syslog(LOG_DEBUG, "Handling announce. Event = %" PRId32, announce_request.event);

  /* Whether the requesting peer is a seeder. */
  bool is_seeder = announce_request.left == 0;

  /* Updates the list of peers by updating or removing the requesting peer. */
  bt_update_peer_list(redis, config, &announce_request, client_addr, info_hash_str, is_seeder);

  /* Number of peers to retrieve from the swarm. */
  int32_t num_want = announce_request.num_want;
  num_want = (num_want < 0 || num_want > config->announce_max_numwant)
    ? config->announce_max_numwant : num_want;

  /*
   * First, if the requesting peer is a seeder, we try to get all leechers.
   * Similarly, if the peer is a leecher, we try to get all seeders.
   */
  peers = bt_peer_list(redis, config, info_hash_str, num_want,  &peer_count, !is_seeder);

  /* Fallbacks to sibling peers in order to fill the gap. */
  if (peer_count < num_want) {
    int complement_count = 0;
    bt_list_t *complement = bt_peer_list(redis, config, info_hash_str, (num_want - peer_count),
                                         &complement_count, is_seeder);

    /* There are new peers to add to the previous list. */
    if (complement != NULL && complement_count > 0) {
      peer_count += complement_count;
      peers = g_list_concat(peers, complement);
    }
  }

  /* Retrieves the latest status about this torrent. */
  bt_torrent_stats_t stats;
  bt_get_torrent_stats(redis, config, info_hash_str, &stats);

  /* Fixed announce response fields. */
  bt_announce_resp_t response_header = {
    .action = request->action,
    .transaction_id = request->transaction_id,
    .interval = config->announce_wait_time,
    .leechers = stats.leechers,
    .seeders = stats.seeders
  };

  /* Info hash hex string no longer needed. */
  free(info_hash_str);

  return bt_serialize_announce_response(&response_header, peer_count, peers);
}
