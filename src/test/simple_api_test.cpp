#include <catch.hpp>

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"
#include "../influxdb-cpp-rest/influxdb_simple_api.h"

#include <chrono>
#include <thread>
#include <iostream>

using influxdb::api::simple_db;

namespace {
    struct simple_connected_test {
        simple_db db;
        influxdb::raw::db_utf8 tdb;
        static const int milliseconds_waiting_time = 10;
        static constexpr const char* db_name = "simpletestdb";

        // drop and create test db
        simple_connected_test();

        // drop test db
        ~simple_connected_test();

        bool db_exists();

        // eventually consistent
        void wait();
    };
}

TEST_CASE_METHOD(simple_connected_test, "creating the db", "[connected]") {
    db.drop();
    wait();
    CHECK(!db_exists());
    db.create();
    wait();
    CHECK(db_exists());
}


simple_connected_test::simple_connected_test() :
    db("http://localhost:8086", db_name),
    tdb("http://localhost:8086")
{
}



simple_connected_test::~simple_connected_test() {
}



// eventually consistent
void simple_connected_test::wait() {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_waiting_time));
}

bool simple_connected_test::db_exists()
{
    return tdb.get("show databases").find(db_name) != std::string::npos;
}
