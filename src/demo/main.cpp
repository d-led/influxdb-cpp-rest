#include <iostream>

#include <influxdb_raw_db_utf8.h>


int main(int argc, char* argv[])
{
    try
    {
        influxdb::raw::db_utf8 db("http://localhost:8086");

        db.post("drop database mydb; create database mydb");

        // {"results":[{"series":[{"columns":["name"],"name":"databases","values":[["_internal"],["mydb"]]}]}]}
        std::cout << db.get("show databases") << std::endl;

        db.insert("mydb", "test value=42");

        // {"results":[{"series":[{"columns":["time","value"],"name":"test","values":[["2016-10-28T22:11:22.8110348Z",42]]}]}]}
        std::cout << db.get("select * from mydb..test") << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
