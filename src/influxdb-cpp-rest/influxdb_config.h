/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace influxdb {
    namespace api {
        
        /// Configuration for batching strategy in async API
        struct batch_config {
            /// Maximum number of lines to batch before sending
            unsigned max_lines = 50000;
            
            /// Maximum time in milliseconds to wait before sending a batch
            unsigned max_time_ms = 100;
            
            batch_config() = default;
            batch_config(unsigned max_lines, unsigned max_time_ms) 
                : max_lines(max_lines), max_time_ms(max_time_ms) {}
        };
        
        /// HTTP client configuration
        struct http_config {
            /// Enable HTTP keepalive (reuse connections)
            bool keepalive = true;
            
            /// Timeout for requests in milliseconds (0 = no timeout)
            unsigned timeout_ms = 0;
            
            /// Maximum connections per host
            unsigned max_connections_per_host = 10;
            
            http_config() = default;
            http_config(bool keepalive, unsigned timeout_ms = 0, unsigned max_connections_per_host = 10)
                : keepalive(keepalive), timeout_ms(timeout_ms), max_connections_per_host(max_connections_per_host) {}
        };
        
        /// Combined configuration for database connections
        struct db_config {
            batch_config batch;
            http_config http;
            
            db_config() = default;
            db_config(const batch_config& batch, const http_config& http = http_config())
                : batch(batch), http(http) {}
            db_config(const http_config& http, const batch_config& batch = batch_config())
                : batch(batch), http(http) {}
        };
        
    }
}

