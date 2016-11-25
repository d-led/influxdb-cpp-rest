#include "influxdb_simple_async_api.h"
#include "influxdb_line.h"
#include "influxdb_simple_api.h"

#include <zmq.hpp>
#include <zmq_addon.hpp>

struct influxdb::async_api::simple_db::impl {
    std::string name;
    influxdb::async_api::simple_db db;

    impl(std::string const& url, std::string const& name) :
        db(url,name),
        name(name)
    {
    }

    ~impl() {}
};


influxdb::async_api::simple_db::simple_db(std::string const& url, std::string const& name) :
    pimpl(std::make_unique<impl>(url, name))
{
}

influxdb::async_api::simple_db::~simple_db()
{
}

void influxdb::async_api::simple_db::create()
{
    pimpl->db.create();
}

void influxdb::async_api::simple_db::drop()
{
    pimpl->db.drop();
}

void influxdb::async_api::simple_db::insert(line const & lines)
{
    pimpl->db.insert(lines);
}
