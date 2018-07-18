// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influxdb_raw_db.h"

#include <cpprest/streams.h>
#include <cpprest/http_client.h>

using namespace utility;
using namespace web;
using namespace web::http;

namespace {
    inline void throw_response(http_response const& response) {
#ifndef _MSC_VER
        throw std::runtime_error(response.extract_string().get());
#else
        throw std::runtime_error(conversions::utf16_to_utf8(response.extract_string().get()));
#endif
    }

    inline http_request request_from(
            uri const& uri_with_db,
            std::string const& lines,
            std::string const& username,
            std::string const& password,
            web::http::method const& m = methods::POST
    ) {
        http_request request;

        request.set_request_uri(uri_with_db);
        request.set_method(m);

        if (!username.empty()) {
            auto auth = username + ":" + password;
            std::vector<unsigned char> bytes(auth.begin(), auth.end());
            request
                .headers()
                .add(
                    header_names::authorization,
                    U("Basic ") +
                    conversions::to_base64(bytes)
                )
            ;
        }

        request.set_body(lines);

        return request;
    }
}

influxdb::raw::db::db(string_t const & url, string_t const & name)
    :
    client(url)
{
    uri_builder builder(client.base_uri());
    builder.append(U("/write"));
    builder.append_query(U("db"), name);
    uri_with_db = builder.to_uri();
}

void influxdb::raw::db::post(string_t const & query)
{
    uri_builder builder(U("/query"));

    builder.append_query(U("q"), query);

    // synchronous for now
    auto response = client.request(
        request_from(builder.to_string(), "", username, password)
    ).get();

    if (response.status_code() != status_codes::OK) {
        throw_response(response);
    }
}

string_t influxdb::raw::db::get(string_t const & query)
{
    uri_builder builder(U("/query"));

    builder.append_query(U("q"), query);

    // synchronous for now
    auto response = client.request(
        request_from(builder.to_string(), "", username, password)
    ).get();
    if (response.status_code() == status_codes::OK)
    {
        return response.extract_string().get();
    }
    else
    {
        throw_response(response);
        return string_t();
    }
}

void influxdb::raw::db::insert(std::string const & lines)
{
    auto response = client.request(request_from(uri_with_db, lines, username, password)).get();
    if (!(response.status_code() == status_codes::OK || response.status_code() == status_codes::NoContent)) {
        throw_response(response);
    }
}

void influxdb::raw::db::insert_async(std::string const & lines)
{
    client.request(request_from(uri_with_db, lines, username, password));
}

void influxdb::raw::db::with_authentication(std::string const& username, std::string const& password)
{
    this->username = username;
    this->password = password;
}
