//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

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
