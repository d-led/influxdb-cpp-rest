// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <catch.hpp>
#include "../influxdb-cpp-rest/input_sanitizer.h"

using namespace influxdb::utility;

TEST_CASE("spaces are not allowed") {
    CHECK(!valid_identifier("   "));
    CHECK(!valid_identifier(" a"));
    CHECK(!valid_identifier("a "));
    CHECK(!valid_identifier("a b"));
}

TEST_CASE("combinations of letters/numbers/_/- allowed") {
    CHECK(valid_identifier("_-_42Ha-ha"));
}

TEST_CASE("throwing on invalid identifier") {
    CHECK_THROWS(throw_on_invalid_identifier(" "));
}

TEST_CASE("quoted identifiers can contain any character and quoted double quotes") {
    CHECK(valid_identifier("\"bla\""));
    CHECK(valid_identifier("\"bla blup\""));
    CHECK(valid_identifier("\"bla-blup\""));
    CHECK(!valid_identifier("\"bla\"blup\""));
    CHECK(valid_identifier("\"bla\\\"blup\""));
}

TEST_CASE("quoted identifiers must begin and end in double quotes") {
    CHECK(!valid_identifier(" \"bla\""));
    CHECK(!valid_identifier("blup\"bla\""));
    CHECK(!valid_identifier("\"bla\"blup"));
    CHECK(!valid_identifier("\"bla\" "));
}