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

void bt_free_redis(void *redis) {
  syslog(LOG_DEBUG, "Disconnecting from Redis");
  redisFree((redisContext *) redis);
}

void bt_request_processor(void *job_params, void *pool_params) {
  static GPrivate redis_key = G_PRIVATE_INIT(bt_free_redis);

  /* Data to be sent to the client. */
  bt_response_buffer_t *resp_buffer = NULL;

  /* Basic request data. */
  bt_req_t request;

  /* Message to be sent to the client in case of error. */
  char *error = NULL;

  /* Cast parameters to their correct types. */
  bt_job_params_t *params = (bt_job_params_t *) job_params;
  bt_config_t *config = (bt_config_t *) pool_params;

  /* Fills object with data in buffer. */
  bt_read_request_data(params->buff, &request);

  int redis_timeout = config->redis_timeout * 1000;
  redisContext *redis = g_private_get(&redis_key);

  /* Connects to the Redis instance where the data should be stored. */
  if (!redis) {
  redis_connect:
    redis = bt_redis_connect(config->redis_host, config->redis_port,
                             redis_timeout, config->redis_db);

    /* Cannot connect, answer this request with an error. */
    if (!redis) {
      error = "Tracker temporarily unavailable: data storage is not working";
      request.action = BT_ACTION_ERROR;
    }

    /* Stores the new redis context in thread local storage. */
    g_private_replace(&redis_key, redis);
  } else {
    if (!bt_redis_ping(redis)) {
      goto redis_connect;
    }
  }

  /* Dispatches the request to the appropriate handler function. */
  switch (request.action) {
  case BT_ACTION_CONNECT:
    resp_buffer = bt_handle_connection(&request, config, params->buflen, redis);
    break;

  case BT_ACTION_ANNOUNCE:
    resp_buffer = bt_handle_announce(&request, config, params->buff,
      params->buflen, params->from_addr, redis);
    break;

  case BT_ACTION_SCRAPE:
    resp_buffer = bt_handle_scrape(&request, config, params->buff,
      params->buflen, redis);
    break;

  case BT_ACTION_ERROR:
    resp_buffer = bt_send_error(&request, error);
    break;
  }

  if (resp_buffer != NULL) {
    syslog(LOG_DEBUG, "Sending response back to the client");
    if (sendto(params->sock, resp_buffer->data, resp_buffer->length, 0,
               (struct sockaddr *) params->from_addr, params->from_addr_len) == -1) {
      syslog(LOG_ERR, "Error in sendto()");
    }

    /* Destroys response data. */
    free(resp_buffer->data);
    free(resp_buffer);
  }

  /* Frees the cloned input buffer. */
  free(params->buff);
}

GThreadPool *bt_new_request_processor_pool(bt_config_t *config) {
  syslog(LOG_DEBUG, "Creating thread pool with %d workers", config->thread_max);

  g_thread_pool_set_max_idle_time(config->thread_max_idle_time * 1000);

  return g_thread_pool_new(bt_request_processor, config,
                           config->thread_max, true, NULL);
}
