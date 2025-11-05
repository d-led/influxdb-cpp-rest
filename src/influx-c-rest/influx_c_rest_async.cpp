// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influx_c_rest_async.h"
#include "influx_c_rest_config.h"
#include "influx_c_rest_lines.h"

#include "../influxdb-cpp-rest/influxdb_simple_api.h"
#include "../influxdb-cpp-rest/influxdb_simple_async_api.h"
#include "../influxdb-cpp-rest/influxdb_line.h"
#include "../influxdb-cpp-rest/influxdb_config.h"

#include <memory>
#include <cassert>
#include <string>
#include <iostream>

extern "C" {

    struct _influx_c_rest_async_t {
        std::unique_ptr<influxdb::async_api::simple_db> asyncdb;
        influxdb::api::default_timestamp timestamp;
    };

    extern "C" INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new(const char* url, const char* name) {
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

    extern "C" INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new_auth(const char* url, const char* name, const char* username, const char* password) {
        assert(username);
        assert(password);
        auto res = influx_c_rest_async_new(url, name);
        if (res) {
            res->asyncdb->with_authentication(username, password);
        }
        return res;
    }

    extern "C" INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new_config(const char* url, const char* name, influx_c_rest_config_t * config) {
        assert(url);
        assert(name);
        assert(config);

        try {
            void* config_ptr = influx_c_rest_config_get_internal(config);
            influxdb::api::db_config* cpp_config = static_cast<influxdb::api::db_config*>(config_ptr);
            influx_c_rest_async_t *res = new influx_c_rest_async_t {
                std::make_unique<influxdb::async_api::simple_db>(url, name, *cpp_config)
            };

            assert(res);
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    extern "C" INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new_auth_config(const char* url, const char* name, const char* username, const char* password, influx_c_rest_config_t * config) {
        assert(username);
        assert(password);
        assert(config);
        auto res = influx_c_rest_async_new_config(url, name, config);
        if (res) {
            res->asyncdb->with_authentication(username, password);
        }
        return res;
    }

    extern "C" INFLUX_C_REST int influx_c_rest_async_drop(influx_c_rest_async_t * self) {
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

    extern "C" INFLUX_C_REST int influx_c_rest_async_create(influx_c_rest_async_t * self) {
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

    extern "C" INFLUX_C_REST void influx_c_rest_async_destroy(influx_c_rest_async_t * self) {
        delete self;
    }

    extern "C" INFLUX_C_REST void influx_c_rest_async_insert(influx_c_rest_async_t * self, const char* line) {
        assert(self);
        assert(self->asyncdb.get());
        assert(line);
        self->asyncdb->insert(influxdb::api::line(std::string(line)));
    }

    extern "C" INFLUX_C_REST void influx_c_rest_async_insert_default_timestamp(influx_c_rest_async_t * self, const char* line) {
        assert(self);
        assert(self->asyncdb.get());
        assert(line);
        self->asyncdb->insert(influxdb::api::line(std::string(line), self->timestamp));
    }

    extern "C" INFLUX_C_REST void influx_c_rest_async_insert_lines(influx_c_rest_async_t * self, influx_c_rest_lines_t * lines) {
        assert(self);
        assert(self->asyncdb.get());
        assert(lines);
        void* line_ptr = influx_c_rest_lines_get_internal(lines);
        influxdb::api::line* line_obj = static_cast<influxdb::api::line*>(line_ptr);
        self->asyncdb->insert(*line_obj);
    }

    extern "C" INFLUX_C_REST void influx_c_rest_async_insert_lines_default_timestamp(influx_c_rest_async_t * self, influx_c_rest_lines_t * lines) {
        assert(self);
        assert(self->asyncdb.get());
        assert(lines);
        void* line_ptr = influx_c_rest_lines_get_internal(lines);
        influxdb::api::line* line_obj = static_cast<influxdb::api::line*>(line_ptr);
        influxdb::api::line line_with_timestamp(line_obj->get(), self->timestamp);
        self->asyncdb->insert(line_with_timestamp);
    }

    extern "C" INFLUX_C_REST void influx_c_rest_async_wait_quiet_ms(influx_c_rest_async_t * self, unsigned quiet_period_ms) {
        assert(self);
        assert(self->asyncdb.get());
        try {
            self->asyncdb->wait_for_submission(std::chrono::milliseconds(quiet_period_ms));
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
