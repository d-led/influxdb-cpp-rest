// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "fixtures.h"

#include <iostream>
#include <chrono>
#include <thread>

connected_test::connected_test() 
{
    // Generate random database name to ensure tests are idempotent
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    db_name = "testdb_" + std::to_string(dis(gen));
    
    raw_db = influxdb::raw::db_utf8("http://localhost:8086", db_name);
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
