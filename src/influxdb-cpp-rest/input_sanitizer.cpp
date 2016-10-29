#include "input_sanitizer.h"

#include <regex>

namespace influxdb {
    namespace utility {

        const std::regex check_identifier("^[a-zA-Z0-9_\\-]+$");

        bool valid_identifier(std::string const & input)
        {
            return std::regex_match(input, check_identifier);
        }


    }

}
