/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <type_traits>
#include "input_sanitizer.h"
#include <fmt/format.h>

namespace influxdb {

    namespace api {

        // https://docs.influxdata.com/influxdb/v1.0/write_protocols/line_protocol_tutorial/
        class key_value_pairs {
            fmt::memory_buffer res;

        public:

            key_value_pairs() {};
            ~key_value_pairs() {};

            key_value_pairs(key_value_pairs const& other) {
                format_to(res, "{}", other.get());
            }

            key_value_pairs& operator=(key_value_pairs const& other) {
                format_to(res, "{}", other.get());
                return *this;
            }


            key_value_pairs(key_value_pairs && other) {
                res = std::move(other.res);
            }

            template<typename V>
            key_value_pairs(std::string const& key, V const& value) {
                add(key, value);
            }

            template<
                class V,
                typename std::enable_if<
                    std::is_integral<V>::value
                >::type* = nullptr
            >
                key_value_pairs& add(std::string const& key, V const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                format_to(res, "{}={}i", key, value);

                return *this;
            }

            template<
                class V,
                typename std::enable_if<
                std::is_floating_point<V>::value
                >::type* = nullptr
            >
                key_value_pairs& add(std::string const& key, V const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                format_to(res, "{}={}", key, value);

                return *this;
            }

            key_value_pairs& add(std::string const& key, std::string const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                format_to(res, "{}=\"{}\"", key, value);

                return *this;
            }

            inline std::string get() const {
                return res.data();
            }

            inline bool empty() const {
                return !res.size();
            }

        private:
            inline void add_comma_if_necessary() {
                if (!this->empty())
                    format_to(res, ",");
            }
        };

        /// https://docs.influxdata.com/influxdb/v1.2/write_protocols/line_protocol_tutorial/#timestamp
        struct default_timestamp {
            inline size_t now() const {
                return std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();
            }
        };

        /// simplest, probably slow implementation
        class line {
            fmt::memory_buffer res;

        public:
            line() {};
            ~line() {};

            line& operator=(line const& other) {
                format_to(res, "{}", other.get());
                return *this;
            }

            line(line const& other) {
                format_to(res, "{}", other.get());
            }

            line(line && other) {
                res = std::move(other.res);
            }

            explicit line(std::string const& raw) {
                format_to(res, "{}", raw);
            }

            template<typename TTimestamp>
            explicit line(std::string const& raw, TTimestamp const& timestamp) {
                format_to(res, "{} {}", raw, timestamp.now());
            }

            template<typename TMap>
            inline line(std::string const& measurement, TMap const& tags, TMap const& values) {
                ::influxdb::utility::throw_on_invalid_identifier(measurement);

                format_to(res, "{}", measurement);
                if (!tags.empty()) {
                    format_to(res, ",{}", tags.get());
                }

                if (!values.empty()) {
                    format_to(res, " {}", values.get());
                }
            }

            template<typename TMap,typename TTimestamp>
            inline line(std::string const& measurement, TMap const& tags, TMap const& values, TTimestamp const& timestamp):
            line(measurement, tags, values) {
                format_to(res, " {}", timestamp.now());
            }

            template<typename TMap>
            inline line& operator()(std::string const& measurement, TMap const& tags, TMap const& values) {
                format_to(res, "\n{}", line(measurement, tags, values).get());
                return *this;
            }

            template<typename TMap, typename TTimestamp>
            inline line& operator()(std::string const& measurement, TMap const& tags, TMap const& values, TTimestamp const& timestamp) {
                format_to(res, "\n{}", line(measurement, tags, values, timestamp).get());
                return *this;
            }
        public:
            inline std::string get() const {
                return res.data();
            }
        };

    }
}
