#pragma once

#include <cpprest/http_client.h>

using utility::string_t;
using web::http::client::http_client;
using web::json::value;

namespace influxdb {
    namespace raw {
        class db {
            http_client client;

        public:
            db(string_t const& url);

            /// post queries
            void post(string_t const& query);

            /// read queries
            value get(string_t const& query);

            /// post measurements
            void measure(string_t const& db, string_t const& lines);
        };
    }
}
