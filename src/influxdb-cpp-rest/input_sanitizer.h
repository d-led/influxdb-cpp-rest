/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

namespace influxdb {
    namespace utility {

        //allowing C-like idenifiers for now (no unicode or spaces as in https://docs.influxdata.com/influxdb/v1.0/query_language/spec/#identifiers)
        bool valid_identifier(std::string const& input);

        void throw_on_invalid_identifier(std::string const& input);
    }
}
