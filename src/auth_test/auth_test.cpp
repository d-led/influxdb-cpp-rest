// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <influxdb_line.h>
#include <influxdb_raw_db_utf8.h>
#include <influxdb_simple_api.h>
#include <influxdb_simple_async_api.h>

#include "../influx-c-rest/influx_c_rest_async.h"
#include "../influx-c-rest/influx_c_rest_query.h"

#include <chrono>
#include <thread>

const char* url = "http://localhost:8086";
const char* db_name = "auth_test";
const std::string username = "admin";
const std::string password = "auth";

using namespace influxdb::api;

struct authentication_test {
    influxdb::api::simple_db api;
    influxdb::async_api::simple_db async_api;
    influxdb::raw::db_utf8 db;
    std::shared_ptr<influx_c_rest_async_t> asyncdb;
    std::shared_ptr<influx_c_rest_query_t> query;

    authentication_test()
        : api(url, db_name)
        , async_api(url, db_name)
        , db(url, db_name)
    {
        api.with_authentication(username, password);
        async_api.with_authentication(username, password);
        db.with_authentication(username, password);

        asyncdb = std::shared_ptr<influx_c_rest_async_t>(
            influx_c_rest_async_new_auth(url, db_name, username.c_str(), password.c_str()),
            influx_c_rest_async_destroy
        );

        REQUIRE(asyncdb.get());

        query = std::shared_ptr<influx_c_rest_query_t>(
            influx_c_rest_query_new_auth(url, db_name, username.c_str(), password.c_str()),
            influx_c_rest_query_destroy
            );

        REQUIRE(query.get());
    }
};

TEST_CASE_METHOD(authentication_test, "authentication smoke test")
{
    SECTION("query")
    {
        CHECK(db.get("show databases").find("results") != std::string::npos);

        SECTION("insert some data")
        {
            async_api.insert(line("test", key_value_pairs(), key_value_pairs("value", 41)));
            api.insert(line("test", key_value_pairs(), key_value_pairs("value", 42)));

            SECTION("wait some")
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(101));

                SECTION("query points")
                {
                    // {"results":[{"series":[{"columns":["time","value"],"name":"test","values":[["2016-10-28T22:11:22.8110348Z",42]]}]}]}
                    auto res = db.get(std::string("select * from ") + db_name + "..test");
                    CHECK(res.find("41") != std::string::npos);
                    CHECK(res.find("42") != std::string::npos);
                }
            }
        }
    }
}

TEST_CASE_METHOD(authentication_test, "C api authentication smoke test", "[!hide]")
{
    SECTION("query")
    {
        auto res = std::shared_ptr<influx_c_rest_result_vt>(
            influx_c_rest_query_get(query.get(), "show databases"),
            influx_c_rest_query_result_destroy
        );
        REQUIRE(!!res.get());
        printf("%s\n",res.get());
        auto q = std::string(res.get());
        CHECK(q.find("results") != std::string::npos);

        SECTION("insert some data")
        {
            // line construction in the test for simplicity (C API would need to format the string in another way
            influx_c_rest_async_insert(asyncdb.get(), line("ctest", key_value_pairs(), key_value_pairs("value", 41)).get().c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(101));
            influx_c_rest_async_insert(asyncdb.get(), line("ctest", key_value_pairs(), key_value_pairs("value", 42)).get().c_str());

            SECTION("wait some")
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1001));

                SECTION("query points")
                {
                    // {"results":[{"series":[{"columns":["time","value"],"name":"test","values":[["2016-10-28T22:11:22.8110348Z",42]]}]}]}
                    auto cres = std::shared_ptr<influx_c_rest_result_vt>(
                        influx_c_rest_query_get(query.get(), (std::string("select * from ") + db_name + "..ctest").c_str()),
                        influx_c_rest_query_result_destroy
                    );
                    printf("%s\n",cres.get());
                    REQUIRE(!!cres.get());
                    auto r = std::string(cres.get());
                    CHECK(r.find("41") != std::string::npos);
                    CHECK(r.find("42") != std::string::npos);
                }
            }
        }
    }
}

TEST_CASE_METHOD(authentication_test, "wrong credentials")
{
    db.with_authentication("wrong", "credentials");
    CHECK_THROWS(db.get("show databases"));
}

void prepare()
{
    influxdb::raw::db_utf8 db(url, db_name);
    db.with_authentication(username, password);
    db.get("CREATE USER " + username + " WITH PASSWORD '" + password + "' WITH ALL PRIVILEGES");

    influxdb::api::simple_db api(url, db_name);
    api.with_authentication(username, password);
    api.drop();
    std::this_thread::sleep_for(std::chrono::milliseconds(101));
    api.create();
    std::this_thread::sleep_for(std::chrono::milliseconds(404));
}

int main(int argc, char* argv[])
{
    try
    {
        prepare();
    }
    catch (std::exception& e)
    {
        // if the credentials are already set, just continue
        std::cerr << e.what() << std::endl;
    }
    return Catch::Session().run(argc, argv);
}
