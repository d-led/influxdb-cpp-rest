#include <iostream>

#include "influxdb_raw_db.h"


int main(int argc, char* argv[])
{
    try
    {
        influxdb::raw::db db(U("http://localhost:8086"));

        db.post(U("create database mydb"));

        std::wcout << db.get(U("show databases")).serialize() << std::endl;

        db.measure(U("mydb"), U("test value=42"));

        std::wcout << db.get(U("select * from mydb..test")).serialize() << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
