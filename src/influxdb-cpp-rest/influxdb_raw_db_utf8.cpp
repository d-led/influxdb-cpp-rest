// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influxdb_raw_db_utf8.h"
#include "influxdb_raw_db.h"

#include <cpprest/http_client.h>

using namespace utility;

namespace {
    // Helper function to convert http_config to http_client_config
    // Hidden in compilation unit to avoid exposing cpprestsdk types
    web::http::client::http_client_config make_http_client_config(const influxdb::api::http_config& config) {
        web::http::client::http_client_config client_config;
        
        // Set timeout if specified
        if (config.timeout_ms > 0) {
            client_config.set_timeout(std::chrono::milliseconds(config.timeout_ms));
        }
        
        // Note: cpprestsdk's http_client reuses connections by default when using the same client instance
        // Connection reuse is handled automatically when using the same http_client.
        // The keepalive flag in config is for documentation purposes.
        
        return client_config;
    }
}

struct influxdb::raw::db_utf8::impl {
    db db_utf16;

public:
    impl(std::string const& url,std::string const& name)
        :
#ifndef _MSC_VER
        db_utf16(url, name)
#else
        db_utf16(conversions::utf8_to_utf16(url), conversions::utf8_to_utf16(name))
#endif
    {}
    
    impl(std::string const& url, std::string const& name, influxdb::api::http_config const& config)
        :
#ifndef _MSC_VER
        db_utf16(url, name, make_http_client_config(config))
#else
        db_utf16(conversions::utf8_to_utf16(url), conversions::utf8_to_utf16(name), make_http_client_config(config))
#endif
    {}
};

influxdb::raw::db_utf8::db_utf8(std::string const & url, std::string const& name) :
    pimpl(std::make_unique<impl>(url, name))
{
}

influxdb::raw::db_utf8::db_utf8(std::string const & url, std::string const& name, influxdb::api::http_config const& config) :
    pimpl(std::make_unique<impl>(url, name, config))
{
}

influxdb::raw::db_utf8::~db_utf8()
{
}

void influxdb::raw::db_utf8::post(std::string const& query) {
#ifndef _MSC_VER
    pimpl->db_utf16.post(query);
#else
    pimpl->db_utf16.post(conversions::utf8_to_utf16(query));
#endif
}

std::string influxdb::raw::db_utf8::get(std::string const& query) {
#ifndef _MSC_VER
    return pimpl->db_utf16.get(query);
#else
    return conversions::utf16_to_utf8(
        pimpl->db_utf16.get(
            conversions::utf8_to_utf16(query)
        )
    );
#endif
}

void influxdb::raw::db_utf8::insert(std::string const & lines) {
    pimpl->db_utf16.insert(lines);
}

void influxdb::raw::db_utf8::insert_async(std::string const & lines)
{
    pimpl->db_utf16.insert_async(lines);
}

void influxdb::raw::db_utf8::with_authentication(std::string const& username, std::string const& password)
{
    pimpl->db_utf16.with_authentication(username, password);
}
