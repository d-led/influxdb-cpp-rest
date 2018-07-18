/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <memory>

namespace influxdb {
    namespace raw {
        class db_utf8 {
            struct impl;
            std::unique_ptr<impl> pimpl;

        public:
            db_utf8(std::string const& url, std::string const& name);
            ~db_utf8();

            /// post queries
            void post(std::string const& query);

            /// read queries
            std::string get(std::string const& query);

            /// post measurements
            void insert(std::string const& lines);

            /// post measurements without waiting for an answer
            void insert_async(std::string const& lines);

            /// set username & password for basic authentication
            void with_authentication(std::string const& username, std::string const& password);
        };
    }
}
