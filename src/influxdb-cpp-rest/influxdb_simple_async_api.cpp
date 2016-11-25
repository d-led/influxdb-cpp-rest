#include "influxdb_simple_async_api.h"
#include "influxdb_line.h"
#include "influxdb_raw_db_utf8.h"
#include "influxdb_simple_api.h"
#include "input_sanitizer.h"
#include "influxdb_line.h"

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <rx.hpp>
#include <chrono>
#include <atomic>
#include <fmt/ostream.h>

using namespace influxdb::utility;

struct influxdb::async_api::simple_db::impl {
    std::string name;
    influxdb::raw::db_utf8 db;
    influxdb::api::simple_db simpledb;
    zmq::context_t context;
    zmq::socket_t push,pull;
    std::atomic<bool> started;
    rxcpp::subscription listener;

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

        auto incoming_requests = rxcpp::observable<>::
                create<std::string>(
                    [this](rxcpp::subscriber<std::string> out) {
                        while (started) {
                            zmq::message_t request;
                            zmq::multipart_t msg(pull);

                            if (!msg.empty())
                                out.on_next(msg.popstr());
                        }
                        out.on_completed();
            })
            .as_dynamic()
            .subscribe_on(rxcpp::synchronize_new_thread())
            ;

            listener = incoming_requests.subscribe([this](std::string const& lines) {
                db.insert(name, lines);
            });

            /* //buffering...

            int counter = 0, count = 0;

            incoming_requests.
                subscribe(
                    [this, &counter, &count](rxcpp::observable<std::string> window) {
                int id = counter++;
                printf("[window %d] Create window\n", id);
                
                window.count()
                    .subscribe([this](int c) {printf("Count in window: %d\n", c); });

                window.scan(std::make_shared<fmt::MemoryWriter>(), [](std::shared_ptr<fmt::MemoryWriter> const& w, std::string const& v) { *w << v << "\n"; return w; })
                    .last()
                    .subscribe([this](std::shared_ptr<fmt::MemoryWriter> const& w) {
                        printf("Len: %zd\n", w->size());
                        db.insert(name, w->str());
                    });
                window.subscribe(
                    [id, &count](std::string const&) {
                    count++; 
                },
                    [id, &count]() {printf("[window %d] OnCompleted: %d\n", id, count); });
            })
            ;*/
    }

    ~impl() {
        started = false;
        listener.unsubscribe();
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
