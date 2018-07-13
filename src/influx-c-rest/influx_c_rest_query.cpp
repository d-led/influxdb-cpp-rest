// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "influx_c_rest_query.h"

#include "../influxdb-cpp-rest/influxdb_raw_db_utf8.h"

#include <memory>
#include <cassert>
#include <string>
#include <iostream>

#include <string.h>

extern "C" {

    struct _influx_c_rest_query_t {
        std::unique_ptr<influxdb::raw::db_utf8> raw_db;
    };

    extern "C" INFLUX_C_REST influx_c_rest_query_t *influx_c_rest_query_new(const char* url, const char* name) {
        assert(url);
        assert(name);

        try {
            influx_c_rest_query_t *res = new influx_c_rest_query_t {
                std::make_unique<influxdb::raw::db_utf8>(url, name)
            };

            assert(res);
            return res;
        } catch (std::exception& e) {
            std::cerr<<e.what()<<std::endl;
            return nullptr;
        }
    }

    extern "C" INFLUX_C_REST influx_c_rest_query_t *influx_c_rest_query_new_auth(const char* url, const char* name, const char* username, const char* password) {
        assert(username);
        assert(password);
        auto res = influx_c_rest_query_new(url, name);
        if (res) {
            res->raw_db->with_authentication(username, password);
        }
        return res;
    }

    extern "C" INFLUX_C_REST void influx_c_rest_query_destroy(influx_c_rest_query_t * self) {
        delete self;
    }

    extern "C" INFLUX_C_REST void influx_c_rest_query_result_destroy(influx_c_rest_result_t result) {
        free(result);
    }

    extern "C" INFLUX_C_REST influx_c_rest_result_t influx_c_rest_query_get(influx_c_rest_query_t * self, const char* query) {
        assert(self);
        assert(self->raw_db);
        assert(query);
        try {
            auto res = self->raw_db->get(query);
            return strdup(res.c_str());
        } catch (std::exception& e) {
            std::cerr<<e.what()<<std::endl;
            return nullptr;
        }
    }
}
