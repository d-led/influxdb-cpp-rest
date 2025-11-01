/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"

#include <functional>
#include <string>
#include <random>

struct connected_test {
    std::string db_name;
    influxdb::raw::db_utf8 raw_db;
    const int milliseconds_waiting_time = 100;

private:
    static std::string generate_random_db_name();

public:
    // drop and create test db
    connected_test();

    // drop test db
    ~connected_test();

    bool database_exists(std::string const& name);

    void wait_for(std::function<bool()> predicate, unsigned retries = 10);
    void wait_for_db(std::string const& name);
    void wait_for_no_db(std::string const & name);
};

// https://github.com/d-led/cpp_declarative_times
struct execute {
    const unsigned long long count;

    template<typename CallableWithIndex>
    void operator() (CallableWithIndex what) {
        for (auto i = 0; i < count; i++)
            what(i);
    }
};

inline execute operator"" _times(unsigned long long count) {
    return execute{ count };
}
