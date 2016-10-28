#include <iostream>

#include <influxdb_raw_db_utf8.h>


int main(int argc, char* argv[])
{
    try
    {
        influxdb::raw::db_utf8 db("http://localhost:8086");

        db.post("create database mydb");

        std::cout << db.get("show databases") << std::endl;

        db.measure("mydb", "test value=42");

        std::cout << db.get("select * from mydb..test") << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
