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

                            if (!msg.empty()) {
                                auto msg_s(msg.popstr());
                                out.on_next(msg_s);
                            }
                        }
                        out.on_completed();
            })
            .as_dynamic()
            .subscribe_on(rxcpp::synchronize_new_thread())
            ;

            listener = incoming_requests
                .window_with_time_or_count(std::chrono::milliseconds(100), 1000, rxcpp::synchronize_new_thread())
                .observe_on(rxcpp::synchronize_new_thread())
                .subscribe(
                    [this](rxcpp::observable<std::string> window) {
                        window.scan(std::make_shared<fmt::MemoryWriter>(), [](std::shared_ptr<fmt::MemoryWriter> const& w, std::string const& v) {
                            *w << v << '\n';
                            return w;
                        })
                        .start_with(std::make_shared<fmt::MemoryWriter>())
                        .last()
                        .observe_on(rxcpp::synchronize_new_thread())
                        .subscribe([this](std::shared_ptr<fmt::MemoryWriter> const& w) {
                            if (w->size() > 0u) {
                                db.insert(name, w->str());
                            }
                        },
                        [](std::exception_ptr ep) {
                            try { std::rethrow_exception(ep); }
                            catch (const std::runtime_error& ex) {
                                std::cerr << ex.what() << std::endl;
                            }
                        },
                        [] {});
                })
            ;
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
