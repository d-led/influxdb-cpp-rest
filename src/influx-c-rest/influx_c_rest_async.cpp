//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influx_c_rest_async.h"

#include "../influxdb-cpp-rest/influxdb_simple_api.h"
#include "../influxdb-cpp-rest/influxdb_simple_async_api.h"
#include "../influxdb-cpp-rest/influxdb_line.h"

#include <memory>
#include <cassert>
#include <string>
#include <iostream>

extern "C" {

    struct _influx_c_rest_async_t {
        std::unique_ptr<influxdb::async_api::simple_db> asyncdb;
    };

    INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new(const char* url, const char* name) {
        assert(url);
        assert(name);

        try {

            influx_c_rest_async_t *res = new influx_c_rest_async_t {
                std::make_unique<influxdb::async_api::simple_db>(url, name)
            };

            assert(res);
            return res;
        } catch (std::exception& e) {
            std::cerr<<e.what()<<std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST int influx_c_rest_async_drop(influx_c_rest_async_t * self) {
        assert(self);
        assert(self->asyncdb.get());
        try {
            self->asyncdb->drop();
            return 0;
        } catch(std::exception& e) {
            std::cerr<<e.what()<<std::endl;
            return 1;
        }
    }

    INFLUX_C_REST int influx_c_rest_async_create(influx_c_rest_async_t * self) {
        assert(self);
        assert(self->asyncdb.get());
        try {
            self->asyncdb->create();
            return 0;
        } catch(std::exception& e) {
            std::cerr<<e.what()<<std::endl;
            return 1;
        }
    }

    INFLUX_C_REST void influx_c_rest_async_destroy(influx_c_rest_async_t * self) {
        assert(self);
        delete self;
    }

    INFLUX_C_REST void influx_c_rest_async_insert(influx_c_rest_async_t * self, const char* line) {
        assert(self);
        assert(self->asyncdb.get());
        assert(line);
        self->asyncdb->insert(influxdb::api::line(std::string(line)));
    }

}
