/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>

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
            ~simple_db();

        public:
            void create();
            void drop();
            void insert(influxdb::api::line const& lines);
            void with_authentication(std::string const& username, std::string const& password);
        };
    }

}
