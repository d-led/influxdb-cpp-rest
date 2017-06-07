//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <influxdb_line.h>
#include <influxdb_raw_db_utf8.h>
#include <influxdb_simple_api.h>
#include <influxdb_simple_async_api.h>

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

    authentication_test()
        : api(url, db_name)
        , async_api(url, db_name)
        , db(url, db_name)
    {
        api.with_authentication(username, password);
        async_api.with_authentication(username, password);
        db.with_authentication(username, password);
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
