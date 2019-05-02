// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "influxdb_simple_async_api.h"
#include "influxdb_line.h"
#include "influxdb_raw_db_utf8.h"
#include "influxdb_simple_api.h"
#include "input_sanitizer.h"
#include "influxdb_line.h"

#include <rx.hpp>
#include <chrono>
#include <atomic>
#include <fmt/format.h>

using namespace influxdb::utility;

struct influxdb::async_api::simple_db::impl {
    influxdb::raw::db_utf8 db;
    influxdb::api::simple_db simpledb;
    std::atomic<bool> started;
    rxcpp::subscription listener;
    rxcpp::subjects::subject<influxdb::api::line> subj;
    unsigned window_max_lines;
    std::chrono::milliseconds window_max_ms;

    impl(std::string const& url, std::string const& name, unsigned window_max_lines, unsigned window_max_ms) :
        db(url, name),
        simpledb(url, name),
        started(false),
        window_max_lines(window_max_lines),
        window_max_ms(window_max_ms)
    {
        throw_on_invalid_identifier(name);
        start_once();
    }

    void start_once()
    {
        if (started)
            return;

        started = true;

        auto incoming_requests = subj.get_observable()
            .map([](auto&& line) {
                return line.get();
            });

            listener = incoming_requests
                .window_with_time_or_count(window_max_ms, (int)window_max_lines, rxcpp::synchronize_new_thread())
                .subscribe(
                    [this](rxcpp::observable<std::string> window) {
                        window.scan(
                            std::make_shared<fmt::memory_buffer>(),
                            [this](std::shared_ptr<fmt::memory_buffer> const& w, std::string const& v) {
                                format_to(*w, "{}\n", v);
                                return w;
                            })
                        .start_with(std::make_shared<fmt::memory_buffer>())
                        .last()
                        .observe_on(rxcpp::synchronize_new_thread())
                        .subscribe([this](std::shared_ptr<fmt::memory_buffer> const& w) {
                            if (w->size() > 0u) {
                                try {
                                    db.insert_async(std::string(w->data(),w->size()));
                                } catch (const std::runtime_error& e) {
                                    throw std::runtime_error(std::string("async_api::insert failed: ") + e.what() + " -> Dropping " + std::to_string(w->size()) + " bytes");
                                }
                            }
                        },
                        [](std::exception_ptr ep) {
                            try { std::rethrow_exception(ep); }
                            catch (const std::runtime_error& ex) {
                                std::cerr << ex.what() << std::endl;
                            }
                        });
                })
            ;
    }

    ~impl() {
        started = false;
        listener.unsubscribe();
    }
};


influxdb::async_api::simple_db::simple_db(std::string const& url, std::string const& name) :
    simple_db(url, name, 50000, 100)
{
}

influxdb::async_api::simple_db::simple_db(std::string const & url, std::string const & name, unsigned window_max_lines, unsigned window_max_ms) :
    pimpl(std::make_unique<impl>(url, name, window_max_lines, window_max_ms))
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
    auto subscriber = pimpl->subj.get_subscriber();

    if (!subscriber.is_subscribed()) {
        return;
    }

    subscriber.on_next(lines);
}


void influxdb::async_api::simple_db::with_authentication(std::string const& username, std::string const& password)
{
    pimpl->db.with_authentication(username, password);
}
