#pragma once

#include <string>
#include <vector>
#include <utility>
#include <type_traits>
#include "input_sanitizer.h"

namespace influxdb {

    namespace api {

        // https://docs.influxdata.com/influxdb/v1.0/write_protocols/line_protocol_tutorial/
        class key_value_pairs {
            std::string res;

        public:
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
                influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                res += key + "=" + std::to_string(value) + "i";

                return *this;
            }

            template<
                class V,
                typename std::enable_if<
                std::is_floating_point<V>::value
                >::type* = nullptr
            >
                key_value_pairs& add(std::string const& key, V const& value) {
                influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                res += key + "=" + std::to_string(value);

                return *this;
            }

            key_value_pairs& add(std::string const& key, std::string const& value) {
                influxdb::utility::throw_on_invalid_identifier(key);

                add_comma_if_necessary();

                res += key + "=\"" + value + "\"";

                return *this;
            }

            inline std::string get() const {
                return res;
            }

            inline bool empty() const {
                return res.empty();
            }

        private:
            void add_comma_if_necessary() {
                if (!res.empty())
                    res += ",";
            }
        };

        /// simplest, probably slow implementation
        class line {
            std::string res;

        public:
            template<typename TMap>
            inline line(std::string const& measurement, TMap tags, TMap values) {
                influxdb::utility::throw_on_invalid_identifier(measurement);

                res += measurement;
                if (!tags.empty()) {
                    res += "," + tags.get();
                }

                if (!values.empty()) {
                    res += " " + values.get();
                }
            }

        public:
            inline std::string get() const {
                return res;
            }
        };

    }
}
