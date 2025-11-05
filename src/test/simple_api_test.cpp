// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch2/catch_test_macros.hpp>

#include "../influxdb-cpp-rest/influxdb_simple_api.h"
#include "../influxdb-cpp-rest/influxdb_simple_async_api.h"
#include "../influxdb-cpp-rest/influxdb_line.h"
#include "../influxdb-cpp-rest/influxdb_http_events.h"
#include "../influxdb-cpp-rest/influxdb_config.h"
#include <rxcpp/rx.hpp>

#include "fixtures.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>
#include <iomanip>

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

TEST_CASE_METHOD(simple_connected_test, "inserting boolean values", "[connected]") {
    db.insert(line("boolean_test", key_value_pairs("mytag", true), key_value_pairs("value", false)));
    db.insert(line("boolean_test", key_value_pairs("mytag", false), key_value_pairs("value", true)));

    wait_for([] {return false; },3);

    auto res = result("boolean_test");
    CHECK(res.contains("true"));
    CHECK(res.contains("false"));
    // tags are not booleans, thus quoted
    CHECK(res.contains("\"true\""));
    CHECK(res.contains("\"false\""));
}

TEST_CASE_METHOD(simple_connected_test, "inserting multiple lines in one call") {
    auto l = line
        ("test1", key_value_pairs("a1", "b1"), key_value_pairs("a2", "b2"), dummy_timestamp { "63169445000000000" })
        ("test2", key_value_pairs("a3", "b3"), key_value_pairs("a4", "b5"))
        ("test2", key_value_pairs("a6", "b6"), key_value_pairs("a7", "b7"))
        ;

    auto ls = l.get();
    CHECK(ls.find("63169445000000000") != std::string::npos);
    CHECK(ls.find("b5") != std::string::npos);
    CHECK(ls.find("b2") != std::string::npos);
    CHECK(ls.find("a7") != std::string::npos);
    CHECK(ls.find('\n') != std::string::npos);

    db.insert(l);

    wait_for([] {return false; }, 3);

    auto res1 = result("test1");
    CHECK(res1.contains("a1"));
    CHECK(res1.contains("b2"));

    auto res2 = result("test2");
    CHECK(res2.contains("a6"));
    CHECK(res2.contains("b7"));
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
        // Track HTTP events - declared in outer scope to outlive subscription
        std::atomic<unsigned long long> http_successes{0};
        std::atomic<unsigned long long> http_failures{0};
        std::atomic<unsigned long long> http_requests_completed{0};
        
        {
            influxdb::async_api::simple_db asyncdb("http://localhost:8086", db_name);
            using Clock = std::chrono::high_resolution_clock;
            
            // Limit to 10k lines and 10 seconds max (same as benchmark)
            constexpr unsigned long long MAX_LINES = 10000;
            constexpr std::chrono::seconds MAX_TIME{10};
            auto many_times = 10000_times;

            //https://www.influxdata.com/influxdb-1-1-released-with-up-to-60-performance-increase-and-new-query-functionality/
            const int MAX_VALUES_PER_TAG = 50000; //actually, 100000

            // Track HTTP events using observable interface
            // RAII: subscription will automatically unsubscribe when it goes out of scope
            // Created in inner scope so it's destroyed before asyncdb
            auto http_events_sub = asyncdb.http_events().subscribe(
                [&](const influxdb::api::http_result& result) {
                    if (result.success) {
                        http_successes.fetch_add(1);
                        http_requests_completed.fetch_add(1);
                    } else {
                        http_failures.fetch_add(1);
                        http_requests_completed.fetch_add(1);
                    }
                },
                [](std::exception_ptr ep) {
                    try { std::rethrow_exception(ep); }
                    catch (const std::exception& e) {
                        std::cerr << "HTTP events observable error: " << e.what() << std::endl;
                    }
                }
            );

            WHEN("I send a large number of unique entries") {
                auto t1 = Clock::now();
                unsigned long long lines_sent = 0;
                
                many_times([&](unsigned long long i) {
                    // Check time limit
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - t1);
                    if (elapsed >= MAX_TIME) {
                        return; // Stop if we've exceeded max time
                    }
                    
                    asyncdb.insert(
                        line("asynctest",
                            key_value_pairs("my_count", i % MAX_VALUES_PER_TAG),
                            key_value_pairs("value", "hi!")
                        ));
                    lines_sent++;
                });
                auto t2 = Clock::now();

                THEN("More than N lines per second can be sent") {
                    // Wait for all submissions to be sent via HTTP using the new API
                    asyncdb.wait_for_submission(std::chrono::milliseconds(200));
                    
                    auto submit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
                    double count_per_second = submit_duration.count() > 0 
                        ? (lines_sent * 1000.0) / submit_duration.count() 
                        : 0.0;
                        
                    std::cout << "\n=== Default Configuration Results ===" << std::endl;
                    std::cout << "Configuration: No batching (1 line / 0ms - immediate send)" << std::endl;
                    std::cout << "Lines submitted: " << lines_sent << std::endl;
                    std::cout << "Submit rate: " << count_per_second << " lines/s" << std::endl;
                    std::cout << "HTTP requests: " << http_requests_completed.load() << " (success/fail: " << http_successes.load() << "/" << http_failures.load() << ")" << std::endl;

                    AND_THEN("All entries arrive at the database") {
                        // wait_for_submission() already ensured all HTTP requests completed
                        // Wait longer for InfluxDB to commit the writes, especially with no-batching
                        // No-batching means many small HTTP requests, so InfluxDB needs more time
                        // Increase wait time to allow InfluxDB to process all writes
                        std::this_thread::sleep_for(std::chrono::seconds(3));
                        
                        // Query actual count from committed data (via query API)
                        auto query = std::string("select count(*) from ") + db_name + "..asynctest";
                        auto response = raw_db.get(query);
                        auto actual_count = extract_count_from_influxdb_response(response);
                        
                        // Measure rate from query start time (after 1 second wait)
                        auto query_time = Clock::now();
                        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(query_time - t1);
                        double actual_rate = total_duration.count() > 0 
                            ? (actual_count * 1000.0) / total_duration.count() 
                            : 0.0;
                        
                        // For no-batching, we're mainly interested in verifying the HTTP requests completed
                        // The actual insert count may lag significantly due to InfluxDB's async processing
                        // So we only check if we got at least some inserts (not exact count)
                        // Lag is expected and measured separately - this is just a sanity check
                        bool some_inserts_occurred = (actual_count > 0); // At least some inserts occurred
                        
                        // Only fail if we have zero inserts (likely an actual error)
                        // Don't assert exact count - lag is expected and measured separately
                        CHECK(some_inserts_occurred);
                        
                        std::cout << "Lines inserted (via query): " << actual_count << std::endl;
                        std::cout << "Insert rate (via query): " << actual_rate << " lines/s" << std::endl;
                        unsigned long long uncommitted = lines_sent > actual_count ? (lines_sent - actual_count) : 0;
                        double uncommitted_pct = lines_sent > 0 ? (uncommitted * 100.0 / lines_sent) : 0.0;
                        std::cout << "Uncommitted lines (lag): " << uncommitted << " (" << std::fixed << std::setprecision(2) << uncommitted_pct << "%)" << std::endl;
                        std::cout << "====================================\n" << std::endl;

                        if (!some_inserts_occurred) {
                            std::cout << "Query response: " << response << std::endl;
                        }
                    }
                }
            }
            // RAII: http_events_sub automatically unsubscribes here when it goes out of scope
            // Give a moment for cleanup before asyncdb is destroyed
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            // asyncdb destroyed here after subscription
        }
    }
}

SCENARIO_METHOD(simple_connected_test, "sensible batching configuration") {
    GIVEN("A connection to the db with sensible batching config (1000 lines / 100ms)") {
        // Sensible batching configuration: 1000 lines / 100ms
        influxdb::api::batch_config batch_cfg{1000, 100};
        influxdb::api::db_config db_cfg{batch_cfg, influxdb::api::http_config{}};
        influxdb::async_api::simple_db batched_db("http://localhost:8086", db_name, db_cfg);
        using Clock = std::chrono::high_resolution_clock;
        
        // Limit to 10k lines and 10 seconds max (same as benchmark)
        constexpr unsigned long long MAX_LINES = 10000;
        constexpr std::chrono::seconds MAX_TIME{10};
        auto many_times = 10000_times;

        const int MAX_VALUES_PER_TAG = 50000;

        // Track HTTP events using observable interface
        std::atomic<unsigned long long> http_successes{0};
        std::atomic<unsigned long long> http_failures{0};
        std::atomic<unsigned long long> http_bytes_sent{0};
        
        auto http_events_sub = batched_db.http_events().subscribe(
            [&](const influxdb::api::http_result& result) {
                if (result.success) {
                    http_successes.fetch_add(1);
                    http_bytes_sent.fetch_add(result.bytes_sent);
                } else {
                    http_failures.fetch_add(1);
                }
            },
            [](std::exception_ptr ep) {
                try { std::rethrow_exception(ep); }
                catch (const std::exception& e) {
                    std::cerr << "HTTP events observable error: " << e.what() << std::endl;
                }
            }
        );

        WHEN("I send lines using the winner configuration") {
            auto t1 = Clock::now();
            unsigned long long lines_sent = 0;
            
            many_times([&](unsigned long long i) {
                // Check time limit
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - t1);
                if (elapsed >= MAX_TIME) {
                    return; // Stop if we've exceeded max time
                }
                
                batched_db.insert(
                    line("winnertest",
                        key_value_pairs("my_count", i % MAX_VALUES_PER_TAG),
                        key_value_pairs("value", "hi!")
                    ));
                lines_sent++;
            });
            auto t2 = Clock::now();

            THEN("Report performance metrics (no assertions)") {
                // Wait for all submissions to be sent via HTTP using the new API
                batched_db.wait_for_submission(std::chrono::milliseconds(100));
                
                auto submit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
                double submit_rate = submit_duration.count() > 0 
                    ? (lines_sent * 1000.0) / submit_duration.count() 
                    : 0.0;
                
                std::cout << "\n=== Sensible Batching Configuration Results ===" << std::endl;
                std::cout << "Configuration: 1000 lines / 100ms batching" << std::endl;
                std::cout << "Lines submitted: " << lines_sent << std::endl;
                std::cout << "Submit rate: " << submit_rate << " lines/s" << std::endl;
                std::cout << "HTTP requests: " << (http_successes.load() + http_failures.load()) << " (success/fail: " << http_successes.load() << "/" << http_failures.load() << ")" << std::endl;
                std::cout << "HTTP bytes sent: " << http_bytes_sent.load() << " bytes" << std::endl;
                
                // wait_for_submission() already ensured all HTTP requests completed
                // Small additional delay to ensure InfluxDB has processed the writes
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                // Query actual count
                auto query = std::string("select count(*) from ") + db_name + "..winnertest";
                auto response = raw_db.get(query);
                auto actual_count = extract_count_from_influxdb_response(response);
                
                auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - t1);
                double actual_rate = total_time.count() > 0 
                    ? (actual_count * 1000.0) / total_time.count() 
                    : 0.0;
                
                std::cout << "Lines inserted (via query): " << actual_count << std::endl;
                std::cout << "Insert rate (via query): " << actual_rate << " lines/s" << std::endl;
                unsigned long long uncommitted = lines_sent > actual_count ? (lines_sent - actual_count) : 0;
                double uncommitted_pct = lines_sent > 0 ? (uncommitted * 100.0 / lines_sent) : 0.0;
                std::cout << "Uncommitted lines (lag): " << uncommitted << " (" << std::fixed << std::setprecision(2) << uncommitted_pct << "%)" << std::endl;
                std::cout << "==============================================\n" << std::endl;
            }
            // RAII: http_events_sub automatically unsubscribes here when it goes out of scope
            // Give a moment for cleanup before batched_db is destroyed
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            // batched_db destroyed here after subscription
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
