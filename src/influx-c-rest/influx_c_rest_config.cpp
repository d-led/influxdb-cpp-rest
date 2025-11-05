// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influx_c_rest_config.h"

#include "../influxdb-cpp-rest/influxdb_config.h"

#include <memory>
#include <cassert>
#include <iostream>

extern "C" {

    struct _influx_c_rest_config_t {
        influxdb::api::db_config config;
    };

    INFLUX_C_REST influx_c_rest_config_t *influx_c_rest_config_new(void) {
        try {
            influx_c_rest_config_t *res = new influx_c_rest_config_t();
            assert(res);
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST void influx_c_rest_config_destroy(influx_c_rest_config_t * self) {
        assert(self);
        delete self;
    }

    INFLUX_C_REST void influx_c_rest_config_set_batch_max_lines(influx_c_rest_config_t * self, unsigned max_lines) {
        assert(self);
        self->config.batch.max_lines = max_lines;
    }

    INFLUX_C_REST void influx_c_rest_config_set_batch_max_time_ms(influx_c_rest_config_t * self, unsigned max_time_ms) {
        assert(self);
        self->config.batch.max_time_ms = max_time_ms;
    }

    INFLUX_C_REST void influx_c_rest_config_set_http_keepalive(influx_c_rest_config_t * self, int keepalive) {
        assert(self);
        self->config.http.keepalive = (keepalive != 0);
    }

    INFLUX_C_REST void influx_c_rest_config_set_http_timeout_ms(influx_c_rest_config_t * self, unsigned timeout_ms) {
        assert(self);
        self->config.http.timeout_ms = timeout_ms;
    }

    INFLUX_C_REST void influx_c_rest_config_set_http_max_connections_per_host(influx_c_rest_config_t * self, unsigned max_connections) {
        assert(self);
        self->config.http.max_connections_per_host = max_connections;
    }

    INFLUX_C_REST void* influx_c_rest_config_get_internal(influx_c_rest_config_t * self) {
        assert(self);
        return &self->config;
    }

}

