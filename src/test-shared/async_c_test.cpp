//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch.hpp>
#include "../influx-c-rest/influx_c_rest_async.h"
#include "../influx-c-rest/influx_c_rest_query.h"
#include "../influxdb-cpp-rest/influxdb_line.h"

#include <memory>
#include <chrono>
#include <thread>

using namespace influxdb::api;

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
        CHECK(influx_c_rest_async_create(asyncdb.get()) == 0);


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
        influx_c_rest_async_insert(asyncdb.get(), "test,kvp1=33i kvp2=\"ho!\"");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
