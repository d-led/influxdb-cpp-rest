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
