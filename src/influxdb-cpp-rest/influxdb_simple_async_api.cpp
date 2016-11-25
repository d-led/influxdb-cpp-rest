#include "influxdb_simple_async_api.h"
#include "influxdb_line.h"
#include "influxdb_raw_db_utf8.h"
#include "influxdb_simple_api.h"
#include "input_sanitizer.h"
#include "influxdb_line.h"

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <thread>
#include <chrono>
#include <atomic>

using namespace influxdb::utility;

struct influxdb::async_api::simple_db::impl {
    std::string name;
    influxdb::raw::db_utf8 db;
    influxdb::api::simple_db simpledb;
    zmq::context_t context;
    zmq::socket_t push,pull;
    std::atomic<bool> started;
    std::thread loop;

    impl(std::string const& url, std::string const& name) :
        db(url),
        simpledb(url, name),
        name(name),
        context(1),
        push(context, ZMQ_PAIR),
        pull(context, ZMQ_PAIR),
        started(false)
    {
        throw_on_invalid_identifier(name);
        start_once();
        auto address(std::string("inproc://") + name + ".inproc");
        pull.setsockopt(ZMQ_RCVTIMEO, 2000);
        pull.bind(address);
        push.connect(address);
    }

    void start_once() {
        if (started) return;

        started = true;

        loop = std::move(std::thread([this] {
            printf("starting to listen\n");
            while (started) {
                zmq::multipart_t msg(pull);
                
                if (!msg.empty())
                    db.insert(name, msg.popstr());
            }
            printf("stopped listening\n");
        }));
    }

    ~impl() {
        started = false;
        if (loop.joinable())
            loop.join();
    }
};


influxdb::async_api::simple_db::simple_db(std::string const& url, std::string const& name) :
    pimpl(std::make_unique<impl>(url, name))
{
}

influxdb::async_api::simple_db::~simple_db()
{
    pimpl->started = false;
}

void influxdb::async_api::simple_db::create()
{
    pimpl->simpledb.create();
}

void influxdb::async_api::simple_db::drop()
{
    pimpl->simpledb.drop();
}

void influxdb::async_api::simple_db::insert(influxdb::api::line const & lines)
{
    zmq::multipart_t msg(lines.get());
    msg.send(pimpl->push);
}
