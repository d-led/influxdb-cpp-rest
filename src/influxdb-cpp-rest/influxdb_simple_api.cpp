#include "influxdb_simple_api.h"
#include "influxdb_raw_db_utf8.h"
#include "input_sanitizer.h"

using namespace influxdb::utility;

struct influxdb::api::simple_db::impl {
    std::string name;
    influxdb::raw::db_utf8 db;

    impl(std::string const& url, std::string const&name) :
        db(url),
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
    pimpl->db.post(std::string("create database ") + pimpl->name);
}

void influxdb::api::simple_db::drop()
{
    pimpl->db.post(std::string("drop database ") + pimpl->name);
}
