// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include <catch2/catch_test_macros.hpp>

#include "fixtures.h"

TEST_CASE_METHOD(connected_test, "creating a database", "[connected]") {
    CHECK(database_exists(db_name));
}


TEST_CASE_METHOD(connected_test, "posting simple values", "[connected]") {
    auto testdb_test_entries = [this]() {
        return raw_db.get("select * from " + db_name + "..test");
    };
    CHECK(testdb_test_entries().find("hello") == std::string::npos);

    raw_db.insert("test value=\"hello\"");

    wait_for([] {return false; }, 3);

    CHECK(testdb_test_entries().find("hello") != std::string::npos);
}

TEST_CASE_METHOD(connected_test, "line protocol violation results in an exception", "[connected]") {
    CHECK_THROWS(raw_db.insert("bla bla bla"));
}

TEST_CASE_METHOD(connected_test, "gibberish query results in an exception", "[connected]") {
    CHECK_THROWS(raw_db.get("bla bla bla"));
}

TEST_CASE("connecting to a nonexistent db results in an exception") {
    influxdb::raw::db_utf8 db("http://localhost:424242", "testdb");
    CHECK_THROWS(db.get("show databases"));
}
