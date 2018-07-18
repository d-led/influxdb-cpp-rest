// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influxdb_simple_api.h"
#include "influxdb_raw_db_utf8.h"
#include "input_sanitizer.h"
#include "influxdb_line.h"

using namespace influxdb::utility;

struct influxdb::api::simple_db::impl {
    std::string name;
    influxdb::raw::db_utf8 db;

    impl(std::string const& url, std::string const& name) :
        db(url, name),
        name(name)
    {
        throw_on_invalid_identifier(name);
    }
};

influxdb::api::simple_db::simple_db(std::string const& url, std::string const& name) :
    pimpl(std::make_unique<impl>(url, name))
{
}

influxdb::api::simple_db::~simple_db()
{
}

void influxdb::api::simple_db::create()
{
    pimpl->db.post(std::string("CREATE DATABASE ") + pimpl->name);
}

void influxdb::api::simple_db::drop()
{
    pimpl->db.post(std::string("DROP DATABASE ") + pimpl->name);
}

void influxdb::api::simple_db::insert(line const & lines)
{
    pimpl->db.insert(lines.get());
}

void influxdb::api::simple_db::with_authentication(std::string const& username, std::string const& password)
{
    pimpl->db.with_authentication(username, password);
}
