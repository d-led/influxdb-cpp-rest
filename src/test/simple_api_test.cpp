//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch.hpp>

#include "../influxdb-cpp-rest/influxdb_simple_api.h"
#include "../influxdb-cpp-rest/influxdb_simple_async_api.h"
#include "../influxdb-cpp-rest/influxdb_line.h"

#include "fixtures.h"

#include <chrono>
#include <thread>
#include <iostream>

using influxdb::api::simple_db;
using influxdb::api::key_value_pairs;
using influxdb::api::line;
using influxdb::api::default_timestamp;

struct simple_connected_test : connected_test {
    simple_db db;

    // drop and create test db
    simple_connected_test();

    // drop the db
    ~simple_connected_test();

    bool db_exists();

    struct result_t {
        std::string line;
        result_t(std::string const& line):line(line) {}

        inline bool contains(std::string const& what) const {
            return line.find(what) != std::string::npos;
        }
    };

    inline result_t result(std::string const& measurement) {
        return result_t(raw_db.get(std::string("select * from ") + db_name + ".." + measurement));
    }
};

TEST_CASE_METHOD(simple_connected_test, "creating the db using the simple api", "[connected]") {
    CHECK(db_exists());
}


TEST_CASE("tags and values should be formatted according to the line protocol") {
    auto kvp=key_value_pairs("a", "b").add("b", 42).add("c", 33.01);

    CHECK(kvp.get().find("a=\"b\",b=42i,c=33.01") != std::string::npos);
}


struct dummy_timestamp {
    std::string stamp;
    std::string now() const {
        return stamp;
    }
};


TEST_CASE("default time stamp is plausible") {
    auto timestamps = default_timestamp();
    auto t1 = timestamps.now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(timestamps.now() - t1 > 0);
}


TEST_CASE("adding a timestamp to a line") {
    auto without = line("test",
        key_value_pairs("kvp1", 42),
        key_value_pairs("kvp2", "hi!")
    );

    auto dummy = dummy_timestamp{ "12345" };

    auto with = line("test",
        key_value_pairs("kvp1", 42),
        key_value_pairs("kvp2", "hi!"),
        dummy
    );

    auto without_timestamp = without.get();
    auto with_timestamp = with.get();
    CHECK(with_timestamp.find(without_timestamp) == 0);
    // same content in the beginning
    CHECK(with_timestamp.substr(without_timestamp.length()) == " " + dummy.now());
}


TEST_CASE_METHOD(simple_connected_test, "inserting values using the simple api", "[connected]") {
    db.insert(line("test", key_value_pairs("mytag", 424242L), key_value_pairs("value", "hello world!")));

    wait_for([] {return false; },3);

    auto res = result("test");
    CHECK(res.contains("424242i"));
    CHECK(res.contains("mytag"));
    CHECK(res.contains("hello world!"));
}


TEST_CASE_METHOD(simple_connected_test, "inserting values using the simple api with timestamps", "[connected]") {
    auto dummy = dummy_timestamp{ "63169445000000000" }; //1972-01-02
    db.insert(line("test_timestamp", key_value_pairs("a1", "b2"), key_value_pairs("c3", "d4"), dummy));
    dummy.stamp = "97553045000000000"; //1973-02-03
    db.insert(line("test_timestamp", key_value_pairs("e1", "f2"), key_value_pairs("g3", "h4"), dummy));
    
    wait_for([] {return false; }, 3);

    auto res = result("test_timestamp");
    CHECK(res.contains("1972-01-02"));
    CHECK(res.contains("1973-02-03"));
    CHECK(res.contains("a1"));
    CHECK(res.contains("d4"));
    CHECK(res.contains("e1"));
    CHECK(res.contains("h4"));
}


SCENARIO_METHOD(simple_connected_test, "more than 1000 inserts per second") {
    GIVEN("A connection to the db") {
        influxdb::async_api::simple_db asyncdb("http://localhost:8086", db_name);
        using Clock = std::chrono::high_resolution_clock;
        auto many_times = 123456_times;

        //https://www.influxdata.com/influxdb-1-1-released-with-up-to-60-performance-increase-and-new-query-functionality/
        const int MAX_VALUES_PER_TAG = 100000;

        WHEN("I send a large number of unique entries") {
            auto t1 = Clock::now();
            many_times([&](unsigned long long i) {
                asyncdb.insert(
                    line("asynctest",
                        key_value_pairs("my_count", i % MAX_VALUES_PER_TAG),
                        key_value_pairs("value", "hi!")
                    ));
            });
            auto t2 = Clock::now();

            THEN("More than 1000 lines per second can be sent") {
                auto diff = t2 - t1;
                auto count_per_second = static_cast<double>(many_times.count) / (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1000.);
                CHECK(count_per_second > 1000.0);
                std::cout << "async inserts per second: " << count_per_second << std::endl;


                AND_THEN("All entries arrive at the database") {
                    // wait for asynchronous fill
                    auto query = std::string("select count(*) from ") + db_name + "..asynctest";
                    wait_for([this, query, many_times] { return raw_db.get(query).find(std::to_string(many_times.count)) != std::string::npos; }, 100);
                    bool all_entries_arrived = raw_db.get(query).find(std::to_string(many_times.count)) != std::string::npos;

                    CHECK(all_entries_arrived);

                    auto new_t2 = Clock::now();
                    auto count_per_second = static_cast<double>(many_times.count) / (std::chrono::duration_cast<std::chrono::milliseconds>(new_t2 - t1).count() / 1000.);
                    std::cout << "actual inserts per second >~: " << count_per_second << std::endl;

                    if (!all_entries_arrived)
                        std::cout << "Response: " << raw_db.get(query) << std::endl;
                }
            }
        }
    }
}

simple_connected_test::simple_connected_test() :
    db("http://localhost:8086", db_name)
{
}


simple_connected_test::~simple_connected_test()
{
}


bool simple_connected_test::db_exists()
{
    return database_exists(db_name);
}
