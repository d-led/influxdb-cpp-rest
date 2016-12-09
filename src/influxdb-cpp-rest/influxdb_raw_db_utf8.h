#pragma once

#include <string>
#include <memory>

namespace influxdb {
    namespace raw {
        class db_utf8 {
            struct impl;
            std::unique_ptr<impl> pimpl;

        public:
            db_utf8(std::string const& url);
            ~db_utf8();

            /// post queries
            void post(std::string const& query);

            /// read queries
            std::string get(std::string const& query);

            /// post measurements
            void insert(std::string const& db, std::string const& lines);
        };
    }
}
