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
            void insert(string_t const& db, string_t const& lines);
        };
    }
}
