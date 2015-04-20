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

bt_response_buffer_t *bt_serialize_scrape_response(bt_scrape_resp_t *response_data) {

  int num_entries = g_list_length(response_data->scrape_entries);

  /* Creates the object where the serialized information will be written to. */
  size_t resp_length = 8 + num_entries * 12;
  bt_response_buffer_t *resp_buffer = (bt_response_buffer_t *)
    malloc(sizeof(bt_response_buffer_t));

  if (resp_buffer == NULL) {
    syslog(LOG_ERR, "Cannot allocate memory for response buffer");
    exit(BT_EXIT_MALLOC_ERROR);
  }

  /* Serializes the response. */
  resp_buffer->length = resp_length;
  resp_buffer->data = (char *) malloc(resp_length);

  if (resp_buffer->data == NULL) {
    syslog(LOG_ERR, "Cannot allocate memory for response buffer data");
    exit(BT_EXIT_MALLOC_ERROR);
  }

  syslog(LOG_DEBUG, "Sending scrape data for %d torrents", num_entries);
  bt_write_scrape_response_data(resp_buffer->data, response_data);

  g_list_free_full(response_data->scrape_entries, free);

  return resp_buffer;
}

bt_response_buffer_t *bt_handle_scrape(const bt_req_t *request,
                                       bt_config_t *config, char *buff,
                                       size_t buflen, redisContext *redis) {

  /* Ignores this request if it's not valid. */
  if (!bt_valid_request(redis, config, request)) {
    return NULL;
  }

  /* Unpacks the data retrieved via network socket. */
  bt_scrape_req_t scrape_request;
  bt_read_scrape_request_data(buff, buflen, &scrape_request);

  syslog(LOG_DEBUG, "Handling scrape");

  bt_list_t *scrape_entries = NULL;

  for (uint8_t i = 0; i < scrape_request.info_hash_len; i++) {
    char *info_hash_str;
    int8_t *info_hash = (int8_t *) scrape_request.info_hash + i * 20;

    bt_bytearray_to_hexarray(info_hash, 20, &info_hash_str);
    if (bt_info_hash_blacklisted(redis, info_hash_str, config)) {
      syslog(LOG_DEBUG, "Blacklisted info hash: %s", info_hash_str);

      free(info_hash_str);
      g_list_free_full(scrape_entries, free);

      return bt_send_error(request, "Blacklisted info hash");
    }

    bt_torrent_stats_t *stats = (bt_torrent_stats_t *)
      malloc(sizeof(bt_torrent_stats_t));
    bt_get_torrent_stats(redis, config, info_hash_str, stats);

    scrape_entries = g_list_prepend(scrape_entries, stats);

    free(info_hash_str);
  }

  /* Fixed announce response fields. */
  bt_scrape_resp_t response_header = {
    .action = request->action,
    .transaction_id = request->transaction_id,
    .scrape_entries = g_list_reverse(scrape_entries)
  };

  return bt_serialize_scrape_response(&response_header);
}
