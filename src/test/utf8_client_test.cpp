#include <catch.hpp>
#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"

TEST_CASE("noop") {
    influxdb::raw::db_utf8 db("http://localhost:8086");
}
