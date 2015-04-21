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
bt_valid_request(redisContext *redis, const bt_config_t *config,
                 const bt_req_t *req, size_t packetlen)
{
  switch (req->action) {
  case BT_ACTION_CONNECT:
    if (BT_PROTOCOL_ID == req->connection_id && packetlen >= 16) {
      return true;
    }
    syslog(LOG_ERR, "Invalid connect packet");
    break;

  case BT_ACTION_ANNOUNCE:
    if (packetlen >= 20 &&
        bt_connection_valid(redis, config, req->connection_id)) {
      return true;
    }
    syslog(LOG_ERR, "Invalid announce packet");
    break;

  case BT_ACTION_SCRAPE:
    if (packetlen >= 8 &&
        bt_connection_valid(redis, config, req->connection_id)) {
      return true;
    }
    syslog(LOG_ERR, "Invalid scrape packet");
    break;

  default:
    syslog(LOG_DEBUG, "Action not supported");
  }

  return false;
}
