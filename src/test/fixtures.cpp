#include "fixtures.h"

#include <iostream>
#include <chrono>
#include <thread>

connected_test::connected_test() :
    raw_db("http://localhost:8086")
{
    raw_db.post(std::string("drop database ") + db_name);
    wait_for_no_db(db_name);
    raw_db.post(std::string("create database ") + db_name);
    wait_for_db(db_name);
}

connected_test::~connected_test() {
    try {
        raw_db.post(std::string("drop database ") + db_name);
        wait_for_no_db(db_name);
    }
    catch (std::exception& e) {
        std::cerr << "FAILED: " << e.what() << std::endl;
    }
}

bool connected_test::database_exists(std::string const & name)
{
    return raw_db.get("show databases").find(name) != std::string::npos;
}

void connected_test::wait_for(std::function<bool()> predicate, unsigned retries)
{
    for (unsigned i = 0; i < retries; i++) {
        if (predicate()) {
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds_waiting_time));
    }
}

void connected_test::wait_for_db(std::string const & name)
{
    wait_for([&, this] {return database_exists(name); });
}

void connected_test::wait_for_no_db(std::string const & name)
{
    wait_for([&, this] {return !database_exists(name); });
}
