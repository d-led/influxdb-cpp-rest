#include <influxdb_raw_db_utf8.h>
#include <influxdb_simple_api.h>
#include <influxdb_line.h>
#include <influxdb_simple_async_api.h>

#include <iostream>
#include <thread>
#include <chrono>

using namespace influxdb::api;

int main(int argc, char* argv[])
{
    try
    {
        const char* url = "http://localhost:8086";
        influxdb::raw::db_utf8 db(url, "demo");
        influxdb::api::simple_db api(url, "demo");
        influxdb::async_api::simple_db async_api(url, "demo");

        api.drop();
        api.create();

        // {"results":[{"series":[{"columns":["name"],"name":"databases","values":[["_internal"],["mydb"]]}]}]}
        std::cout << db.get("show databases") << std::endl;

        async_api.insert(line("test", key_value_pairs(), key_value_pairs("value", 41)));
        api.insert(line("test", key_value_pairs(), key_value_pairs("value", 42)));

        std::this_thread::sleep_for(std::chrono::milliseconds(101));

        // {"results":[{"series":[{"columns":["time","value"],"name":"test","values":[["2016-10-28T22:11:22.8110348Z",42]]}]}]}
        std::cout << db.get("select * from demo..test") << std::endl;

        // or if the async call passes through:
        // {"results":[{"series":[{"name":"test","columns":["time","value"],
        //             "values":[["2016-12-09T20:24:18.8239801Z",42],["2016-12-09T20:24:18.9026688Z",41]]}]}]}

        api.drop();

        // multiple lines formatted for one synchronous call:
        // multiple,v1=1i
        // multiple,v2=2i
        std::cout << line
            ("multiple", key_value_pairs("v1", 1), key_value_pairs())
            ("multiple", key_value_pairs("v2", 2), key_value_pairs())
        .get() << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
