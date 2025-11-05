/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "influx_c_rest_api.h"
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

    typedef struct _influx_c_rest_lines_t influx_c_rest_lines_t;
    typedef struct _influx_c_rest_key_value_pairs_t influx_c_rest_key_value_pairs_t;

    /* lifetime */
    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new(void);
    INFLUX_C_REST void influx_c_rest_lines_destroy(influx_c_rest_lines_t * self);

    INFLUX_C_REST influx_c_rest_key_value_pairs_t *influx_c_rest_key_value_pairs_new(void);
    INFLUX_C_REST void influx_c_rest_key_value_pairs_destroy(influx_c_rest_key_value_pairs_t * self);

    /* line construction */
    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new_raw(const char* raw);
    INFLUX_C_REST influx_c_rest_lines_t *influx_c_rest_lines_new_measurement(const char* measurement, influx_c_rest_key_value_pairs_t * tags, influx_c_rest_key_value_pairs_t * values);
    INFLUX_C_REST void influx_c_rest_lines_add_line(influx_c_rest_lines_t * self, const char* measurement, influx_c_rest_key_value_pairs_t * tags, influx_c_rest_key_value_pairs_t * values);

    /* key value pairs */
    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_int(influx_c_rest_key_value_pairs_t * self, const char* key, long long value);
    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_bool(influx_c_rest_key_value_pairs_t * self, const char* key, int value);
    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_float(influx_c_rest_key_value_pairs_t * self, const char* key, double value);
    INFLUX_C_REST void influx_c_rest_key_value_pairs_add_string(influx_c_rest_key_value_pairs_t * self, const char* key, const char* value);

    /* get formatted string */
    INFLUX_C_REST const char* influx_c_rest_lines_get(influx_c_rest_lines_t * self);
    INFLUX_C_REST void influx_c_rest_lines_get_copy(influx_c_rest_lines_t * self, char* buffer, size_t buffer_size);

    /* internal access - returns pointer to internal line structure */
    INFLUX_C_REST void* influx_c_rest_lines_get_internal(influx_c_rest_lines_t * self);

#if defined(__cplusplus)
}
#endif

