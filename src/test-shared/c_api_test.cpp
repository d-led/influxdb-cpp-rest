// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch2/catch_test_macros.hpp>
#include "../influx-c-rest/influx_c_rest_async.h"
#include "../influx-c-rest/influx_c_rest_config.h"
#include "../influx-c-rest/influx_c_rest_lines.h"
#include "../influx-c-rest/influx_c_rest_query.h"

#include <memory>
#include <chrono>
#include <thread>
#include <cstring>

struct c_api_test
{
    std::shared_ptr<influx_c_rest_async_t> asyncdb;
    std::shared_ptr<influx_c_rest_query_t> query;

    c_api_test() {
        asyncdb = std::shared_ptr<influx_c_rest_async_t>(
            influx_c_rest_async_new("http://localhost:8086","c_api_test"),
            influx_c_rest_async_destroy
        );

        REQUIRE(asyncdb.get());

        influx_c_rest_async_drop(asyncdb.get());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CHECK(influx_c_rest_async_create(asyncdb.get()) == 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        query = std::shared_ptr<influx_c_rest_query_t>(
            influx_c_rest_query_new("http://localhost:8086","c_api_test"),
            influx_c_rest_query_destroy
        );

        REQUIRE(query.get());
    }
};

TEST_CASE_METHOD(c_api_test, "config API") {
    SECTION("create and destroy config") {
        auto config = std::shared_ptr<influx_c_rest_config_t>(
            influx_c_rest_config_new(),
            influx_c_rest_config_destroy
        );
        REQUIRE(config.get());
    }

    SECTION("set batch configuration") {
        auto config = std::shared_ptr<influx_c_rest_config_t>(
            influx_c_rest_config_new(),
            influx_c_rest_config_destroy
        );
        REQUIRE(config.get());

        influx_c_rest_config_set_batch_max_lines(config.get(), 1000);
        influx_c_rest_config_set_batch_max_time_ms(config.get(), 50);
    }

    SECTION("set http configuration") {
        auto config = std::shared_ptr<influx_c_rest_config_t>(
            influx_c_rest_config_new(),
            influx_c_rest_config_destroy
        );
        REQUIRE(config.get());

        influx_c_rest_config_set_http_keepalive(config.get(), 1);
        influx_c_rest_config_set_http_timeout_ms(config.get(), 5000);
        influx_c_rest_config_set_http_max_connections_per_host(config.get(), 20);
    }

    SECTION("create async db with config") {
        auto config = std::shared_ptr<influx_c_rest_config_t>(
            influx_c_rest_config_new(),
            influx_c_rest_config_destroy
        );
        REQUIRE(config.get());

        influx_c_rest_config_set_batch_max_lines(config.get(), 500);
        influx_c_rest_config_set_batch_max_time_ms(config.get(), 100);

        auto db_with_config = std::shared_ptr<influx_c_rest_async_t>(
            influx_c_rest_async_new_config("http://localhost:8086", "c_api_test_config", config.get()),
            influx_c_rest_async_destroy
        );
        REQUIRE(db_with_config.get());
    }
}

TEST_CASE_METHOD(c_api_test, "key value pairs API") {
    SECTION("create and destroy key value pairs") {
        auto kvp = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        REQUIRE(kvp.get());
    }

    SECTION("add integer values") {
        auto kvp = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        REQUIRE(kvp.get());

        influx_c_rest_key_value_pairs_add_int(kvp.get(), "int_value", 42LL);
        influx_c_rest_key_value_pairs_add_int(kvp.get(), "negative", -10LL);
        influx_c_rest_key_value_pairs_add_int(kvp.get(), "large", 999999LL);
    }

    SECTION("add boolean values") {
        auto kvp = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        REQUIRE(kvp.get());

        influx_c_rest_key_value_pairs_add_bool(kvp.get(), "is_true", 1);
        influx_c_rest_key_value_pairs_add_bool(kvp.get(), "is_false", 0);
    }

    SECTION("add float values") {
        auto kvp = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        REQUIRE(kvp.get());

        influx_c_rest_key_value_pairs_add_float(kvp.get(), "pi", 3.14159);
        influx_c_rest_key_value_pairs_add_float(kvp.get(), "negative", -1.5);
        influx_c_rest_key_value_pairs_add_float(kvp.get(), "zero", 0.0);
    }

    SECTION("add string values") {
        auto kvp = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        REQUIRE(kvp.get());

        influx_c_rest_key_value_pairs_add_string(kvp.get(), "name", "test");
        influx_c_rest_key_value_pairs_add_string(kvp.get(), "message", "hello world");
        influx_c_rest_key_value_pairs_add_string(kvp.get(), "empty", "");
    }
}

TEST_CASE_METHOD(c_api_test, "lines API") {
    SECTION("create and destroy lines") {
        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new(),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());
    }

    SECTION("create lines from raw string") {
        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_raw("test,key=value field=42i"),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        const char* result = influx_c_rest_lines_get(lines.get());
        REQUIRE(result != nullptr);
        REQUIRE(std::string(result).find("test") != std::string::npos);
    }

    SECTION("create lines with measurement and key value pairs") {
        auto tags = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags.get(), "tag1", "value1");
        influx_c_rest_key_value_pairs_add_int(tags.get(), "tag2", 42LL);

        influx_c_rest_key_value_pairs_add_float(values.get(), "value1", 3.14);
        influx_c_rest_key_value_pairs_add_string(values.get(), "value2", "test");

        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_measurement("measurement", tags.get(), values.get()),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        const char* result = influx_c_rest_lines_get(lines.get());
        REQUIRE(result != nullptr);
        REQUIRE(std::string(result).find("measurement") != std::string::npos);
        REQUIRE(std::string(result).find("tag1") != std::string::npos);
        REQUIRE(std::string(result).find("value1") != std::string::npos);
    }

    SECTION("add line to existing lines object") {
        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new(),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        auto tags1 = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values1 = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags1.get(), "tag", "a");
        influx_c_rest_key_value_pairs_add_int(values1.get(), "value", 1LL);

        influx_c_rest_lines_add_line(lines.get(), "measurement1", tags1.get(), values1.get());

        auto tags2 = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values2 = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags2.get(), "tag", "b");
        influx_c_rest_key_value_pairs_add_int(values2.get(), "value", 2LL);

        influx_c_rest_lines_add_line(lines.get(), "measurement2", tags2.get(), values2.get());

        const char* result = influx_c_rest_lines_get(lines.get());
        REQUIRE(result != nullptr);
        REQUIRE(std::string(result).find("measurement1") != std::string::npos);
        REQUIRE(std::string(result).find("measurement2") != std::string::npos);
    }

    SECTION("get copy of formatted string") {
        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_raw("test,key=value field=42i"),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        char buffer[256];
        influx_c_rest_lines_get_copy(lines.get(), buffer, sizeof(buffer));
        REQUIRE(std::string(buffer).find("test") != std::string::npos);
    }
}

TEST_CASE_METHOD(c_api_test, "async API with lines") {
    SECTION("insert lines using lines API") {
        auto tags = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags.get(), "tag1", "test_tag");
        influx_c_rest_key_value_pairs_add_int(values.get(), "value1", 123LL);
        influx_c_rest_key_value_pairs_add_string(values.get(), "value2", "test_string");

        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_measurement("lines_test", tags.get(), values.get()),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        influx_c_rest_async_insert_lines(asyncdb.get(), lines.get());
        influx_c_rest_async_wait_quiet_ms(asyncdb.get(), 200);

        SECTION("query inserted lines") {
            auto res = std::shared_ptr<influx_c_rest_result_vt>(
                influx_c_rest_query_get(query.get(), "select * from c_api_test..lines_test"),
                influx_c_rest_query_result_destroy
            );
            REQUIRE(res.get());

            auto s = std::string(res.get());
            CHECK(s.find("123") != std::string::npos);
            CHECK(s.find("test_string") != std::string::npos);
        }
    }

    SECTION("insert lines with default timestamp") {
        auto tags = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags.get(), "tag", "timestamp_test");
        influx_c_rest_key_value_pairs_add_int(values.get(), "value", 456LL);

        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_measurement("timestamp_test", tags.get(), values.get()),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        influx_c_rest_async_insert_lines_default_timestamp(asyncdb.get(), lines.get());
        influx_c_rest_async_wait_quiet_ms(asyncdb.get(), 200);

        SECTION("query inserted lines") {
            auto res = std::shared_ptr<influx_c_rest_result_vt>(
                influx_c_rest_query_get(query.get(), "select * from c_api_test..timestamp_test"),
                influx_c_rest_query_result_destroy
            );
            REQUIRE(res.get());

            auto s = std::string(res.get());
            CHECK(s.find("456") != std::string::npos);
        }
    }

    SECTION("insert multiple lines") {
        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new(),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        for (int i = 0; i < 3; i++) {
            auto tags = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
                influx_c_rest_key_value_pairs_new(),
                influx_c_rest_key_value_pairs_destroy
            );
            auto values = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
                influx_c_rest_key_value_pairs_new(),
                influx_c_rest_key_value_pairs_destroy
            );

            char tag_key[32];
            char value_key[32];
            snprintf(tag_key, sizeof(tag_key), "index");
            snprintf(value_key, sizeof(value_key), "value");

            influx_c_rest_key_value_pairs_add_int(tags.get(), tag_key, i);
            influx_c_rest_key_value_pairs_add_int(values.get(), value_key, i * 10LL);

            influx_c_rest_lines_add_line(lines.get(), "multi_test", tags.get(), values.get());
        }

        influx_c_rest_async_insert_lines(asyncdb.get(), lines.get());
        influx_c_rest_async_wait_quiet_ms(asyncdb.get(), 200);

        SECTION("query inserted lines") {
            auto res = std::shared_ptr<influx_c_rest_result_vt>(
                influx_c_rest_query_get(query.get(), "select * from c_api_test..multi_test"),
                influx_c_rest_query_result_destroy
            );
            REQUIRE(res.get());

            auto s = std::string(res.get());
            CHECK(s.find("0") != std::string::npos);
            CHECK(s.find("10") != std::string::npos);
            CHECK(s.find("20") != std::string::npos);
        }
    }
}

TEST_CASE_METHOD(c_api_test, "async API with config") {
    SECTION("create async db with custom batch config") {
        auto config = std::shared_ptr<influx_c_rest_config_t>(
            influx_c_rest_config_new(),
            influx_c_rest_config_destroy
        );
        REQUIRE(config.get());

        influx_c_rest_config_set_batch_max_lines(config.get(), 10);
        influx_c_rest_config_set_batch_max_time_ms(config.get(), 200);

        auto db_with_config = std::shared_ptr<influx_c_rest_async_t>(
            influx_c_rest_async_new_config("http://localhost:8086", "c_api_test_config", config.get()),
            influx_c_rest_async_destroy
        );
        REQUIRE(db_with_config.get());

        influx_c_rest_async_drop(db_with_config.get());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CHECK(influx_c_rest_async_create(db_with_config.get()) == 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Insert some data
        auto tags = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );
        auto values = std::shared_ptr<influx_c_rest_key_value_pairs_t>(
            influx_c_rest_key_value_pairs_new(),
            influx_c_rest_key_value_pairs_destroy
        );

        influx_c_rest_key_value_pairs_add_string(tags.get(), "tag", "config_test");
        influx_c_rest_key_value_pairs_add_int(values.get(), "value", 789LL);

        auto lines = std::shared_ptr<influx_c_rest_lines_t>(
            influx_c_rest_lines_new_measurement("config_test", tags.get(), values.get()),
            influx_c_rest_lines_destroy
        );
        REQUIRE(lines.get());

        influx_c_rest_async_insert_lines(db_with_config.get(), lines.get());
        influx_c_rest_async_wait_quiet_ms(db_with_config.get(), 200);
    }
}

