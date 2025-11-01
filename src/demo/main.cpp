// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <influxdb_raw_db_utf8.h>
#include <influxdb_simple_api.h>
#include <influxdb_line.h>
#include <influxdb_simple_async_api.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace influxdb::api;
using namespace influxdb::raw;
using namespace std::string_literals;

using async_db = influxdb::async_api::simple_db;

int main(int, char**)
{
    try {
        const auto url = "http://localhost:8086"s;
        const auto db_name = "demo"s;

        auto db = db_utf8(url, db_name);
        auto api = simple_db(url, db_name);
        auto async_api = async_db(url, db_name);

        api.drop();
        api.create();

        // {"results":[{"series":[{"columns":["name"],"name":"databases","values":[["_internal"],["mydb"]]}]}]}
        std::cout << db.get("show databases"s) << '\n';

        async_api.insert(line("test"s, key_value_pairs(), key_value_pairs("value"s, 41)));
        api.insert(line("test"s, key_value_pairs(), key_value_pairs("value"s, 42)));

        std::this_thread::sleep_for(std::chrono::milliseconds(101));

        // {"results":[{"series":[{"columns":["time","value"],"name":"test","values":[["2016-10-28T22:11:22.8110348Z",42]]}]}]}
        std::cout << db.get("select * from demo..test"s) << '\n';

        // or if the async call passes through:
        // {"results":[{"series":[{"name":"test","columns":["time","value"],
        //             "values":[["2016-12-09T20:24:18.8239801Z",42],["2016-12-09T20:24:18.9026688Z",41]]}]}]}

        api.drop();

        // multiple lines formatted for one synchronous call:
        // multiple,v1=1i
        // multiple,v2=2i
        std::cout << line
            ("multiple"s, key_value_pairs("v1"s, 1), key_value_pairs())
            ("multiple"s, key_value_pairs("v2"s, 2), key_value_pairs())
        .get() << '\n';
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}