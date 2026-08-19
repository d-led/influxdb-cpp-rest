/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdlib>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <type_traits>

namespace influxdb {
    namespace utility {

        // Formats a floating-point value with the shortest decimal
        // representation that round-trips, matching std::format("{}", value)
        // and std::to_chars general format. This is implemented by hand because
        // std::format and std::to_chars for floating point are not available on
        // every toolchain we support (for example libc++ before macOS 13.3),
        // which would otherwise force a very new deployment target and compiler
        // for the whole library.
        template <typename T>
        std::string format_float(T value) {
            std::ostringstream out;
            out.imbue(std::locale::classic());

            for (int precision = 1; precision <= std::numeric_limits<T>::max_digits10; ++precision) {
                out.str("");
                out.clear();
                out << std::setprecision(precision) << value;

                const std::string candidate = out.str();
                T parsed;
                if constexpr (std::is_same_v<T, float>) {
                    parsed = std::strtof(candidate.c_str(), nullptr);
                } else if constexpr (std::is_same_v<T, long double>) {
                    parsed = std::strtold(candidate.c_str(), nullptr);
                } else {
                    parsed = std::strtod(candidate.c_str(), nullptr);
                }

                if (parsed == value) {
                    return candidate;
                }
            }

            return out.str();
        }

    }
}
