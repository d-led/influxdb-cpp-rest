/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>
#include "influxdb_config.h"
#include "influxdb_http_events.h"
#include <rxcpp/rx.hpp>

namespace influxdb {
    namespace api {
        class line;
    }

    namespace async_api {

        class simple_db {
            struct impl;
            std::unique_ptr<impl> pimpl;

        public:
            simple_db(std::string const& url, std::string const& name);
            simple_db(std::string const& url, std::string const& name, unsigned window_max_lines, unsigned window_max_ms);
            simple_db(std::string const& url, std::string const& name, influxdb::api::db_config const& config);
            ~simple_db();

        public:
            void create();
            void drop();
            void insert(influxdb::api::line const& lines);
            void with_authentication(std::string const& username, std::string const& password);
            
            /// Get observable of HTTP operation results (successes and failures)
            /// Subscribe to this to monitor HTTP requests and handle errors
            rxcpp::observable<influxdb::api::http_result> http_events() const;
            
            /// Wait for all pending submissions to be sent via HTTP
            /// Uses RxCpp debounce to wait until no HTTP events occur for the specified duration
            /// @param quiet_period_ms Maximum time to wait for silence (default: 100ms)
            void wait_for_submission(std::chrono::milliseconds quiet_period_ms = std::chrono::milliseconds(100)) const;
        };
    }

}
