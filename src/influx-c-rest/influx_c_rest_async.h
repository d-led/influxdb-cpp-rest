/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "influx_c_rest_api.h"

#if defined(__cplusplus)
extern "C" {
#endif

    typedef struct _influx_c_rest_async_t influx_c_rest_async_t;

    /* lifetime */
    INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new(const char* url, const char* name);
    INFLUX_C_REST influx_c_rest_async_t *influx_c_rest_async_new_auth(const char* url, const char* name, const char* username, const char* password);
    INFLUX_C_REST void influx_c_rest_async_destroy(influx_c_rest_async_t * self);

    /* behavior */
    INFLUX_C_REST int influx_c_rest_async_drop(influx_c_rest_async_t * self);
    INFLUX_C_REST int influx_c_rest_async_create(influx_c_rest_async_t * self);
    INFLUX_C_REST void influx_c_rest_async_insert(influx_c_rest_async_t * self, const char* line);
    INFLUX_C_REST void influx_c_rest_async_insert_default_timestamp(influx_c_rest_async_t * self, const char* line);

#if defined(__cplusplus)
}
#endif
