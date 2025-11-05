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
#include "influxdb_http_events.h"
#include "input_sanitizer.h"
#include "influxdb_line.h"

#include <rxcpp/rx.hpp>
#include <chrono>
#include <atomic>
#include <format>
#include <iterator>
#include <thread>

using namespace influxdb::utility;

struct influxdb::async_api::simple_db::impl {
    influxdb::raw::db_utf8 db;
    influxdb::api::simple_db simpledb;
    std::atomic<bool> started;
    rxcpp::subscription listener;
    rxcpp::subjects::subject<influxdb::api::line> subj;
    rxcpp::subjects::subject<influxdb::api::http_result> http_events_subj;
    unsigned window_max_lines;
    std::chrono::milliseconds window_max_ms;
    // Single shared scheduler and worker for all operations (both batching and no-batching)
    // This prevents thread explosion by reusing a single thread pool
    // Worker is stored to ensure it outlives all subscriptions (RxCpp issue #437)
    rxcpp::schedulers::scheduler shared_scheduler;
    rxcpp::schedulers::worker shared_worker;

    impl(std::string const& url, std::string const& name, unsigned window_max_lines, unsigned window_max_ms) :
        db(url, name),
        simpledb(url, name),
        started(false),
        window_max_lines(window_max_lines),
        window_max_ms(std::chrono::milliseconds(window_max_ms)),
        // Always use shared event loop scheduler for all scenarios
        // Single thread pool prevents thread explosion
        // Create worker immediately to avoid RxCpp issue #185 (make_event_loop inner empty)
        shared_scheduler(rxcpp::schedulers::make_event_loop()),
        shared_worker(shared_scheduler.create_worker())
    {
        throw_on_invalid_identifier(name);
        start_once();
    }

    impl(std::string const& url, std::string const& name, influxdb::api::db_config const& config) :
        db(url, name, config.http),
        simpledb(url, name, config.http),
        started(false),
        window_max_lines(config.batch.max_lines),
        window_max_ms(config.batch.max_time_ms),
        // Always use shared event loop scheduler for all scenarios
        // Single thread pool prevents thread explosion
        // Create worker immediately to avoid RxCpp issue #185 (make_event_loop inner empty)
        shared_scheduler(rxcpp::schedulers::make_event_loop()),
        shared_worker(shared_scheduler.create_worker())
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

            // Create coordination from our instance scheduler to ensure proper lifetime
            // This avoids the crash-prone static synchronize_event_loop() pattern
            // The coordination is tied to shared_scheduler/shared_worker lifetime
            // Use observe_on_one_worker with scheduler (not worker) to create coordination
            auto coordination = rxcpp::observe_on_one_worker(shared_scheduler);
            
            // For true "no-batching" mode (1 line / 0ms), bypass windowing entirely
            // and send each line immediately. This ensures we get ~1 request per line
            // instead of batching due to scheduler tick granularity.
            if (window_max_lines == 1 && window_max_ms.count() == 0) {
                // No-batching: send each line immediately without windowing
                listener = incoming_requests
                    .observe_on(coordination)
                    .subscribe([this](std::string const& line_str) {
                        if (!started.load()) {
                            return;
                        }
                        
                        auto start_time = std::chrono::steady_clock::now();
                        auto bytes_sent = line_str.size();
                        
                        try {
                            db.insert_async(line_str);
                            
                            // Success event
                            if (started.load()) {
                                auto end_time = std::chrono::steady_clock::now();
                                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                                influxdb::api::http_result result(true, "insert", bytes_sent);
                                result.status_code = 204; // NoContent
                                result.duration_ms = duration;
                                result.bytes_received = 0;
                                try {
                                    http_events_subj.get_subscriber().on_next(result);
                                } catch (...) {
                                    // Subject may be destroyed, ignore during shutdown
                                }
                            }
                        } catch (const std::runtime_error& e) {
                            // Failure event
                            if (started.load()) {
                                auto end_time = std::chrono::steady_clock::now();
                                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                                influxdb::api::http_result result(false, "insert", bytes_sent);
                                result.error_message = e.what();
                                result.duration_ms = duration;
                                try {
                                    http_events_subj.get_subscriber().on_next(result);
                                } catch (...) {
                                    // Subject may be destroyed, ignore during shutdown
                                }
                            }
                            // Still print error but don't throw to avoid breaking the observable chain
                            std::cerr << "async_api::insert failed: " << e.what() << " -> Dropping " << bytes_sent << " bytes" << std::endl;
                        }
                    },
                    [this](std::exception_ptr ep) {
                        if (!started.load()) {
                            return;
                        }
                        
                        try { std::rethrow_exception(ep); }
                        catch (const std::runtime_error& ex) {
                            influxdb::api::http_result result(false, "insert", 0);
                            result.error_message = ex.what();
                            try {
                                http_events_subj.get_subscriber().on_next(result);
                            } catch (...) {
                                // Subject may be destroyed, ignore during shutdown
                            }
                            std::cerr << ex.what() << std::endl;
                        }
                    });
            } else {
                // Batching mode: use windowing
                listener = incoming_requests
                    .window_with_time_or_count(window_max_ms, (int)window_max_lines, coordination)
                .subscribe(
                    [this, coordination](rxcpp::observable<std::string> window) {
                        // Use observe_on to ensure the nested subscription runs on the correct thread
                        // This prevents crashes from accessing subjects across threads
                        // Don't capture subscriber - check validity before each use to avoid crashes
                        window.scan(
                            std::make_shared<std::string>(),
                            [](std::shared_ptr<std::string> const& w, std::string const& v) {
                                std::format_to(std::back_inserter(*w), "{}\n", v);
                                return w;
                            })
                        .start_with(std::make_shared<std::string>())
                        .last()
                        .observe_on(coordination)
                        .subscribe([this](std::shared_ptr<std::string> const& w) {
                            // Check if we're still started before accessing subject
                            // This prevents crashes when the object is being destroyed
                            if (!started.load()) {
                                return;
                            }
                            
                            if (!w->empty()) {
                                auto start_time = std::chrono::steady_clock::now();
                                auto bytes_sent = w->size();
                                
                                try {
                                    db.insert_async(*w);
                                    
                                    // Success event - check started again before accessing subject
                                    if (started.load()) {
                                        auto end_time = std::chrono::steady_clock::now();
                                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                                        influxdb::api::http_result result(true, "insert", bytes_sent);
                                        result.status_code = 204; // NoContent
                                        result.duration_ms = duration;
                                        result.bytes_received = 0; // No response body for successful inserts
                                        try {
                                            http_events_subj.get_subscriber().on_next(result);
                                        } catch (...) {
                                            // Subject may be destroyed, ignore during shutdown
                                        }
                                    }
                                } catch (const std::runtime_error& e) {
                                    // Failure event - check started again before accessing subject
                                    if (started.load()) {
                                        auto end_time = std::chrono::steady_clock::now();
                                        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                                        influxdb::api::http_result result(false, "insert", bytes_sent);
                                        result.error_message = e.what();
                                        result.duration_ms = duration;
                                        try {
                                            http_events_subj.get_subscriber().on_next(result);
                                        } catch (...) {
                                            // Subject may be destroyed, ignore during shutdown
                                        }
                                    }
                                    
                                    // Still throw to maintain existing error behavior
                                    throw std::runtime_error(std::string("async_api::insert failed: ") + e.what() + " -> Dropping " + std::to_string(w->size()) + " bytes");
                                }
                            }
                        },
                        [this](std::exception_ptr ep) {
                            // Check started before accessing subject
                            if (!started.load()) {
                                return;
                            }
                            
                            try { std::rethrow_exception(ep); }
                            catch (const std::runtime_error& ex) {
                                // Emit error event for unhandled exceptions
                                influxdb::api::http_result result(false, "insert", 0);
                                result.error_message = ex.what();
                                try {
                                    http_events_subj.get_subscriber().on_next(result);
                                } catch (...) {
                                    // Subject may be destroyed, ignore during shutdown
                                }
                                std::cerr << ex.what() << std::endl;
                            }
                        });
                    })
                ;
            }
        }

    ~impl() {
        // Proper shutdown sequence:
        // 1. Stop accepting new operations
        started = false;
        
        // 2. Unsubscribe the listener to stop processing new windows
        try {
            if (listener.is_subscribed()) {
                listener.unsubscribe();
            }
        } catch (...) {
            // Ignore errors during shutdown
        }
        
        // 3. Give a moment for in-flight operations to complete before unsubscribing
        // This is especially important for the event loop scheduler
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // 4. Unsubscribe the worker (RxCpp issue #437: workers need explicit unsubscribe)
        try {
            shared_worker.unsubscribe();
        } catch (...) {
            // Ignore errors during shutdown
        }
        
        // 5. Give one more moment after worker unsubscribe for final cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
};


influxdb::async_api::simple_db::simple_db(std::string const& url, std::string const& name) :
    // Default: no batching (1 line, 0ms) for backward compatibility
    // Each line is sent immediately via HTTP
    // Uses shared event loop scheduler to prevent thread exhaustion with rapid inserts
    // For high-throughput scenarios, use explicit batching config (e.g., batch_config{1000, 100})
    simple_db(url, name, 1, 0)
{
}

influxdb::async_api::simple_db::simple_db(std::string const & url, std::string const & name, unsigned window_max_lines, unsigned window_max_ms) :
    pimpl(std::make_unique<impl>(url, name, window_max_lines, window_max_ms))
{
}

influxdb::async_api::simple_db::simple_db(std::string const & url, std::string const & name, influxdb::api::db_config const& config) :
    pimpl(std::make_unique<impl>(url, name, config))
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

rxcpp::observable<influxdb::api::http_result> influxdb::async_api::simple_db::http_events() const
{
    return pimpl->http_events_subj.get_observable();
}

void influxdb::async_api::simple_db::wait_for_submission(std::chrono::milliseconds quiet_period_ms) const
{
    // Use RxCpp debounce to wait until no HTTP events occur for quiet_period_ms
    // This is a simple reactive approach - no mutexes, no polling
    // Use current_thread scheduler to avoid creating new threads
    auto scheduler = rxcpp::observe_on_one_worker(rxcpp::schedulers::make_current_thread());
    http_events()
        .start_with(influxdb::api::http_result(true, "wait", 0))
        .debounce(quiet_period_ms, scheduler)
        .take(1)
        .as_blocking()
        .subscribe(
            [](const influxdb::api::http_result&) {
                // Debounce completed - no events for quiet_period_ms, all submissions are done
            },
            [](std::exception_ptr) {
                // Ignore errors during wait
            }
        );
}
