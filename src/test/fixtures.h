#pragma once

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"

#include <functional>

struct connected_test {
    influxdb::raw::db_utf8 raw_db;
    static const int milliseconds_waiting_time = 100;

    static constexpr const char* db_name = "testdb";

    // drop and create test db
    connected_test();

    // drop test db
    ~connected_test();

    bool database_exists(std::string const& name);

    void wait_for(std::function<bool()> predicate, unsigned retries = 10);
    void wait_for_db(std::string const& name);
    void wait_for_no_db(std::string const & name);
};
