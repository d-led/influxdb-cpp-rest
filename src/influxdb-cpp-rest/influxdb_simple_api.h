/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>
#include "influxdb_config.h"

namespace influxdb {

    namespace api {

        class line;

        class simple_db {
            struct impl;
            std::unique_ptr<impl> pimpl;

        public:
            simple_db(std::string const& url, std::string const& name);
            simple_db(std::string const& url, std::string const& name, influxdb::api::http_config const& config);
            ~simple_db();

        public:
            void create();
            void drop();
            void insert(line const& lines);
            void with_authentication(std::string const& username, std::string const& password);
        };
    }

}
