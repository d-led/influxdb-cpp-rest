#include <catch.hpp>

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"

#include <iostream>
#include <chrono>
#include <thread>


struct connected_test {
    influxdb::raw::db_utf8 db;
    static const int milliseconds_waiting_time = 10;

    // drop and create test db
    connected_test();

    // drop test db
    ~connected_test();

    // eventually consistent
    void wait();
};

TEST_CASE_METHOD(connected_test, "creating a database", "[connected]") {
    CHECK(db.get("show databases").find("testdb") != std::string::npos);
}


TEST_CASE_METHOD(connected_test, "posting simple values", "[connected]") {
    CHECK(db.get("select * from testdb..test").find("hello") == std::string::npos);

    db.insert("testdb", "test value=\"hello\"");

    wait();
    CHECK(db.get("select * from testdb..test").find("hello") != std::string::npos);
}

TEST_CASE_METHOD(connected_test, "line protocol violation results in an exception","[connected]") {
    CHECK_THROWS(db.insert("testdb", "bla bla bla"));
}

TEST_CASE_METHOD(connected_test, "gibberish query results in an exception", "[connected]") {
    CHECK_THROWS(db.get("bla bla bla"));
}

TEST_CASE("connecting to a nonexistent db results in an exception") {
    influxdb::raw::db_utf8 db("http://localhost:424242");
    CHECK_THROWS(db.get("show databases"));
}

inline connected_test::connected_test() :
    db("http://localhost:8086")
{
    db.post("drop database testdb; create database testdb");
}



inline connected_test::~connected_test() {
    try {
        db.post("drop database testdb");
    }
    catch (std::exception& e) {
        std::cerr << "FAILED: " << e.what() << std::endl;
    }
}



// eventually consistent

inline void connected_test::wait() {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_waiting_time));
}
