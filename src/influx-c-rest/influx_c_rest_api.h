/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#ifdef _MSC_VER
    #ifdef BUILDING_INFLUX_C_REST
        #define INFLUX_C_REST __declspec(dllexport)
    #else
        #define INFLUX_C_REST __declspec(dllimport)
    #endif
#else
    #define INFLUX_C_REST
#endif
