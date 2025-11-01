// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch2/catch_test_macros.hpp>
#include "../influx-c-rest/influx_c_rest_async.h"
#include "../influx-c-rest/influx_c_rest_query.h"

#include <memory>
#include <chrono>
#include <thread>

struct async_c_test
{
    std::shared_ptr<influx_c_rest_async_t> asyncdb;
    std::shared_ptr<influx_c_rest_query_t> query;

    async_c_test() {
        asyncdb = std::shared_ptr<influx_c_rest_async_t>(
            influx_c_rest_async_new("http://localhost:8086","c_test"),
            influx_c_rest_async_destroy
        );

        REQUIRE(asyncdb.get());

        influx_c_rest_async_drop(asyncdb.get());
        // todo: wait loops
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CHECK(influx_c_rest_async_create(asyncdb.get()) == 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        query = std::shared_ptr<influx_c_rest_query_t>(
            influx_c_rest_query_new("http://localhost:8086","c_test"),
            influx_c_rest_query_destroy
        );

        REQUIRE(query.get());
    }
};

TEST_CASE_METHOD(async_c_test, "post values from C asynchronously") {
    SECTION("insert some lines") {
        influx_c_rest_async_insert(asyncdb.get(), "test,kvp1=42i kvp2=\"hi!\"");
        influx_c_rest_async_insert_default_timestamp(asyncdb.get(), "test,kvp1=33i kvp2=\"ho!\"");
        std::this_thread::sleep_for(std::chrono::seconds(1));

        SECTION("query") {
            auto res = std::shared_ptr<influx_c_rest_result_vt>(
                influx_c_rest_query_get(query.get(),"select * from c_test..test"),
                influx_c_rest_query_result_destroy
            );
            REQUIRE(res.get());

            auto s = std::string(res.get());

            CHECK(s.find("42i")!=std::string::npos);
            CHECK(s.find("33i")!=std::string::npos);
            CHECK(s.find("hi!")!=std::string::npos);
            CHECK(s.find("ho!")!=std::string::npos);
        }
    }
}
