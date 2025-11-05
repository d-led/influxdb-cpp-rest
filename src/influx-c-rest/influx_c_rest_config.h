/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "influx_c_rest_api.h"

#if defined(__cplusplus)
extern "C" {
#endif

    typedef struct _influx_c_rest_config_t influx_c_rest_config_t;

    /* lifetime */
    INFLUX_C_REST influx_c_rest_config_t *influx_c_rest_config_new(void);
    INFLUX_C_REST void influx_c_rest_config_destroy(influx_c_rest_config_t * self);

    /* batch configuration */
    INFLUX_C_REST void influx_c_rest_config_set_batch_max_lines(influx_c_rest_config_t * self, unsigned max_lines);
    INFLUX_C_REST void influx_c_rest_config_set_batch_max_time_ms(influx_c_rest_config_t * self, unsigned max_time_ms);

    /* http configuration */
    INFLUX_C_REST void influx_c_rest_config_set_http_keepalive(influx_c_rest_config_t * self, int keepalive);
    INFLUX_C_REST void influx_c_rest_config_set_http_timeout_ms(influx_c_rest_config_t * self, unsigned timeout_ms);
    INFLUX_C_REST void influx_c_rest_config_set_http_max_connections_per_host(influx_c_rest_config_t * self, unsigned max_connections);

    /* internal access - returns pointer to internal config structure */
    INFLUX_C_REST void* influx_c_rest_config_get_internal(influx_c_rest_config_t * self);

#if defined(__cplusplus)
}
#endif

