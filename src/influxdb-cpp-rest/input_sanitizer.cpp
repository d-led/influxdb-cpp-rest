//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "input_sanitizer.h"

#include <regex>

namespace influxdb {
    namespace utility {

        const std::regex check_identifier("^[a-zA-Z0-9_\\-]+$");

        bool valid_identifier(std::string const & input)
        {
            return std::regex_match(input, check_identifier);
        }

        void throw_on_invalid_identifier(std::string const & input)
        {
            if (!valid_identifier(input))
                throw std::runtime_error(std::string("Invalid identifier: ") + input);
        }
    }

}
