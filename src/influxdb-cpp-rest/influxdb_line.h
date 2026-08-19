/* * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <utility>
#include <chrono>
#include <type_traits>
#include <sstream>
#include "input_sanitizer.h"
#include "format_float.h"

namespace influxdb {

    namespace api {

        // https://docs.influxdata.com/influxdb/v1.0/write_protocols/line_protocol_tutorial/
        class key_value_pairs {
            std::string res;

        public:

            key_value_pairs() {};
            ~key_value_pairs() {};

            key_value_pairs(key_value_pairs const& other) {
                res = other.res;
            }

            key_value_pairs& operator=(key_value_pairs const& other) {
                res += other.res;
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
                    std::is_integral<V>::value &&
                    (! std::is_same<bool, V>::value)
                >::type* = nullptr
            >
                key_value_pairs& add(std::string const& key, V const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                std::ostringstream out;
                out << key << '=' << value << 'i';
                res += out.str();

                return *this;
            }

            template<
                class V,
                typename std::enable_if<
                    std::is_integral<V>::value &&
                    ( std::is_same<bool, V>::value)
                >::type* = nullptr
            >
                key_value_pairs& add(std::string const& key, V const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                res += key;
                res.push_back('=');
                res += (value ? "true" : "false");

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

                res += key;
                res.push_back('=');
                res += ::influxdb::utility::format_float(value);

                return *this;
            }

            key_value_pairs& add(std::string const& key, std::string const& value) {
                ::influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                res += key;
                res += "=\"";
                res += value;
                res.push_back('"');

                return *this;
            }

            inline std::string get() const {
                return res;
            }

            inline bool empty() const {
                return res.empty();
            }

        private:
            inline void add_comma_if_necessary() {
                if (!this->empty())
                    res.push_back(',');
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
            std::string res;

        public:
            line() {};
            ~line() {};

            line& operator=(line const& other) {
                res += other.res;
                return *this;
            }

            line(line const& other) {
                res = other.res;
            }

            line(line && other) {
                res = std::move(other.res);
            }

            explicit line(std::string const& raw) {
                res = raw;
            }

            template<typename TTimestamp>
            explicit line(std::string const& raw, TTimestamp const& timestamp) {
                std::ostringstream out;
                out << raw << ' ' << timestamp.now();
                res = out.str();
            }

            template<typename TMap>
            inline line(std::string const& measurement, TMap const& tags, TMap const& values) {
                ::influxdb::utility::throw_on_invalid_identifier(measurement);

                res += measurement;
                if (!tags.empty()) {
                    res.push_back(',');
                    res += tags.get();
                }

                if (!values.empty()) {
                    res.push_back(' ');
                    res += values.get();
                }
            }

            template<typename TMap,typename TTimestamp>
            inline line(std::string const& measurement, TMap const& tags, TMap const& values, TTimestamp const& timestamp):
            line(measurement, tags, values) {
                std::ostringstream out;
                out << ' ' << timestamp.now();
                res += out.str();
            }

            template<typename TMap>
            inline line& operator()(std::string const& measurement, TMap const& tags, TMap const& values) {
                res.push_back('\n');
                res += line(measurement, tags, values).get();
                return *this;
            }

            template<typename TMap, typename TTimestamp>
            inline line& operator()(std::string const& measurement, TMap const& tags, TMap const& values, TTimestamp const& timestamp) {
                res.push_back('\n');
                res += line(measurement, tags, values, timestamp).get();
                return *this;
            }
        public:
            inline std::string get() const {
                return res;
            }
        };

    }
}
