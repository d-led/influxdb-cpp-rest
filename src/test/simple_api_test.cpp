#include <catch.hpp>

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"
#include "../influxdb-cpp-rest/influxdb_simple_api.h"
#include "../influxdb-cpp-rest/influxdb_line.h"

#include <chrono>
#include <thread>
#include <iostream>

using influxdb::api::simple_db;
using influxdb::api::key_value_pairs;
using influxdb::api::line;

namespace {
    struct simple_connected_test {
        simple_db db;
        influxdb::raw::db_utf8 tdb;
        static const int milliseconds_waiting_time = 10;
        static constexpr const char* db_name = "simpletestdb";

        // drop and create test db once
        simple_connected_test();

        bool db_exists();

        // eventually consistent
        void wait();

        struct result_t {
            std::string line;
            result_t(std::string const& line):line(line) {}

            inline bool contains(std::string const& what) const {
                return line.find(what) != std::string::npos;
            }
        };

        inline result_t result(std::string const& measurement) {
            return result_t(tdb.get(std::string("select * from ") + db_name + ".." + measurement));
        }
    };
}

TEST_CASE_METHOD(simple_connected_test, "creating the db using the simple api", "[connected]") {
    CHECK(db_exists());
}

TEST_CASE("tags and values should be formatted according to the line protocol") {
    auto kvp = key_value_pairs("a", "b").add("b", 42).add("c", 33.0);

    CHECK(kvp.get().find("a=\"b\",b=42i,c=33.") != std::string::npos);
}

TEST_CASE_METHOD(simple_connected_test, "inserting values using the simple api", "[connected]") {
    wait();
    db.insert(line("test", key_value_pairs("mytag", 424242L), key_value_pairs("value", "hello world!")));
    wait();
    
    auto res = result("test");
    CHECK(res.contains("424242i"));
    CHECK(res.contains("mytag"));
    CHECK(res.contains("hello world!"));
}

simple_connected_test::simple_connected_test() :
    db("http://localhost:8086", db_name),
    tdb("http://localhost:8086")
{
    db.drop();
    wait();
    db.create();
    wait();
}


// eventually consistent
void simple_connected_test::wait() {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_waiting_time));
}

bool simple_connected_test::db_exists()
{
    return tdb.get("show databases").find(db_name) != std::string::npos;
}
