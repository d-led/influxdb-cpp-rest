// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influx_c_rest_lines.h"

#include "../influxdb-cpp-rest/influxdb_line.h"

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <cstring>
#include <cstddef>

extern "C" {

    struct _influx_c_rest_key_value_pairs_t {
        influxdb::api::key_value_pairs kvp;
    };

    struct _influx_c_rest_lines_t {
        influxdb::api::line line_obj;
        std::string cached_string;
        bool string_cached;
    };

    INFLUX_C_REST influx_c_rest_key_value_pairs_t *influx_c_rest_key_value_pairs_new(void) {
        try {
            influx_c_rest_key_value_pairs_t *res = new influx_c_rest_key_value_pairs_t();
            assert(res);
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST void influx_c_rest_key_value_pairs_destroy(influx_c_rest_key_value_pairs_t * self) {
        assert(self);
        delete self;
    }

    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_int(influx_c_rest_key_value_pairs_t * self, const char* key, long long value) {
        assert(self);
        assert(key);
        try {
            self->kvp.add(std::string(key), value);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_bool(influx_c_rest_key_value_pairs_t * self, const char* key, int value) {
        assert(self);
        assert(key);
        try {
            self->kvp.add(std::string(key), value != 0);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_float(influx_c_rest_key_value_pairs_t * self, const char* key, double value) {
        assert(self);
        assert(key);
        try {
            self->kvp.add(std::string(key), value);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_string(influx_c_rest_key_value_pairs_t * self, const char* key, const char* value) {
        assert(self);
        assert(key);
        assert(value);
        try {
            self->kvp.add(std::string(key), std::string(value));
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new(void) {
        try {
            influx_c_rest_lines_t *res = new influx_c_rest_lines_t();
            assert(res);
            res->string_cached = false;
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST void influx_c_rest_lines_destroy(influx_c_rest_lines_t * self) {
        assert(self);
        delete self;
    }

    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new_raw(const char* raw) {
        assert(raw);
        try {
            influx_c_rest_lines_t *res = new influx_c_rest_lines_t();
            assert(res);
            res->line_obj = influxdb::api::line(std::string(raw));
            res->string_cached = false;
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new_measurement(const char* measurement, influx_c_rest_key_value_pairs_t * tags, influx_c_rest_key_value_pairs_t * values) {
        assert(measurement);
        try {
            influx_c_rest_lines_t *res = new influx_c_rest_lines_t();
            assert(res);
            influxdb::api::key_value_pairs tags_kvp = tags ? tags->kvp : influxdb::api::key_value_pairs();
            influxdb::api::key_value_pairs values_kvp = values ? values->kvp : influxdb::api::key_value_pairs();
            
            res->line_obj = influxdb::api::line(std::string(measurement), tags_kvp, values_kvp);
            res->string_cached = false;
            return res;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        }
    }

    INFLUX_C_REST void influx_c_rest_lines_add_line(influx_c_rest_lines_t * self, const char* measurement, influx_c_rest_key_value_pairs_t * tags, influx_c_rest_key_value_pairs_t * values) {
        assert(self);
        assert(measurement);
        try {
            influxdb::api::key_value_pairs tags_kvp = tags ? tags->kvp : influxdb::api::key_value_pairs();
            influxdb::api::key_value_pairs values_kvp = values ? values->kvp : influxdb::api::key_value_pairs();

            self->line_obj(std::string(measurement), tags_kvp, values_kvp);
            self->string_cached = false;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    INFLUX_C_REST const char* influx_c_rest_lines_get(influx_c_rest_lines_t * self) {
        assert(self);
        if (!self->string_cached) {
            self->cached_string = self->line_obj.get();
            self->string_cached = true;
        }
        return self->cached_string.c_str();
    }

    INFLUX_C_REST void influx_c_rest_lines_get_copy(influx_c_rest_lines_t * self, char* buffer, size_t buffer_size) {
        assert(self);
        assert(buffer);
        assert(buffer_size > 0);
        const char* str = influx_c_rest_lines_get(self);
        size_t len = strlen(str);
        if (len >= buffer_size) {
            len = buffer_size - 1;
        }
        memcpy(buffer, str, len);
        buffer[len] = '\0';
    }

    INFLUX_C_REST void* influx_c_rest_lines_get_internal(influx_c_rest_lines_t * self) {
        assert(self);
        return &self->line_obj;
    }

}

