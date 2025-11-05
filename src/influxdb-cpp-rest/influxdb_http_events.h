/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <string>
#include <memory>

namespace influxdb {
    namespace api {
        
        /// HTTP operation result
        struct http_result {
            bool success;
            std::chrono::steady_clock::time_point timestamp;
            std::string operation;      // "insert", "query", etc.
            size_t bytes_sent;         // Number of bytes in request
            size_t bytes_received;      // Number of bytes in response (0 if failed)
            unsigned status_code;       // HTTP status code (0 if error occurred)
            std::string error_message;  // Error message (empty if success)
            std::chrono::milliseconds duration_ms; // Request duration
            
            http_result(bool success, std::string op, size_t bytes_sent = 0)
                : success(success), timestamp(std::chrono::steady_clock::now()),
                  operation(std::move(op)), bytes_sent(bytes_sent), 
                  bytes_received(0), status_code(0), duration_ms(0) {}
        };
        
    }
}

