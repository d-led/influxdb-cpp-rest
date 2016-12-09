/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cpprest/http_client.h>

using utility::string_t;
using web::http::client::http_client;

namespace influxdb {
    namespace raw {
        class db {
            http_client client;

        public:
            db(string_t const& url);

            /// post queries
            void post(string_t const& query);

            /// read queries
            string_t get(string_t const& query);

            /// post measurements
            void insert(string_t const& db, std::string const& lines);
        };
    }
}
