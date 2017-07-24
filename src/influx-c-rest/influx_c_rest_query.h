/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "influx_c_rest_api.h"

#if defined(__cplusplus)
extern "C" {
#endif

    typedef struct _influx_c_rest_query_t influx_c_rest_query_t;
    typedef char* influx_c_rest_result_t;
    typedef char influx_c_rest_result_vt;

    /* lifetime */
    INFLUX_C_REST influx_c_rest_query_t *influx_c_rest_query_new(const char* url, const char* name);
    INFLUX_C_REST influx_c_rest_query_t *influx_c_rest_query_new_auth(const char* url, const char* name, const char* username, const char* password);
    INFLUX_C_REST void influx_c_rest_query_destroy(influx_c_rest_query_t * self);
    INFLUX_C_REST void influx_c_rest_query_result_destroy(influx_c_rest_result_t result);

    /* behavior */
    INFLUX_C_REST influx_c_rest_result_t influx_c_rest_query_get(influx_c_rest_query_t * self, const char* query);

#if defined(__cplusplus)
}
#endif
