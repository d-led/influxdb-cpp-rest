//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influxdb_raw_db_utf8.h"
#include "influxdb_raw_db.h"

#include <cpprest/http_client.h>

using namespace utility;

struct influxdb::raw::db_utf8::impl {
    db db_utf16;

public:
    impl(std::string const& url,std::string const& name)
        :
        db_utf16(conversions::utf8_to_utf16(url), conversions::utf8_to_utf16(name))
    {}
};

influxdb::raw::db_utf8::db_utf8(std::string const & url, std::string const& name) :
    pimpl(std::make_unique<impl>(url, name))
{
}

influxdb::raw::db_utf8::~db_utf8()
{
}

void influxdb::raw::db_utf8::post(std::string const& query) {
    pimpl->db_utf16.post(conversions::utf8_to_utf16(query));
}

std::string influxdb::raw::db_utf8::get(std::string const& query) {
    return conversions::utf16_to_utf8(
        pimpl->db_utf16.get(
            conversions::utf8_to_utf16(query)
        )
    );
}

void influxdb::raw::db_utf8::insert(std::string const & lines) {
    pimpl->db_utf16.insert(lines);
}

void influxdb::raw::db_utf8::insert_async(std::string const & lines)
{
    pimpl->db_utf16.insert_async(lines);
}
