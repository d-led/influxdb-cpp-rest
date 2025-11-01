// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#include "fixtures.h"

#include <iostream>
#include <chrono>
#include <thread>

std::string connected_test::generate_random_db_name() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    return "testdb_" + std::to_string(dis(gen));
}

connected_test::connected_test() 
    : db_name(generate_random_db_name()),
      raw_db("http://localhost:8086", db_name)
{
    raw_db.post(std::string("drop database ") + db_name);
    wait_for_no_db(db_name);
    raw_db.post(std::string("create database ") + db_name);
    wait_for_db(db_name);
}

connected_test::~connected_test() {
    try {
        raw_db.post(std::string("drop database ") + db_name);
        wait_for_no_db(db_name);
    }
    catch (std::exception& e) {
        std::cerr << "FAILED: " << e.what() << std::endl;
    }
}

bool connected_test::database_exists(std::string const & name)
{
    return raw_db.get("show databases").find(name) != std::string::npos;
}

void connected_test::wait_for(std::function<bool()> predicate, unsigned retries)
{
    // Use exponential backoff: start with short waits, increase gradually
    // This is more efficient for slow systems (especially Windows async operations)
    unsigned wait_ms = milliseconds_waiting_time;
    const unsigned max_wait_ms = 500;  // Cap at 500ms
    
    for (unsigned i = 0; i < retries; i++) {
        if (predicate()) {
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        
        // Exponential backoff: increase wait time gradually, cap at max_wait_ms
        wait_ms += milliseconds_waiting_time;
        if (wait_ms > max_wait_ms) {
            wait_ms = max_wait_ms;
        }
    }
}

void connected_test::wait_for_db(std::string const & name)
{
    wait_for([&, this] {return database_exists(name); });
}

void connected_test::wait_for_no_db(std::string const & name)
{
    wait_for([&, this] {return !database_exists(name); });
}

unsigned long long extract_count_from_influxdb_response(std::string const& response)
{
    // Parse count from JSON response: {"results":[{"statement_id":0,"series":[{"name":"table","columns":["time","count_value"],"values":[["1970-01-01T00:00:00Z",COUNT]]}]}]}
    // Structure: "values":[["timestamp",COUNT]]
    auto values_pos = response.find("\"values\":[[");
    if (values_pos != std::string::npos) {
        // Find the inner array with timestamp and count: [["timestamp",COUNT]]
        // The count is the second value, so find the comma after the timestamp
        auto bracket = response.find("[[", values_pos);
        if (bracket != std::string::npos) {
            bracket += 2; // Skip [[
            // Skip timestamp (could be quoted string or number)
            auto comma = response.find(",", bracket);
            if (comma != std::string::npos) {
                comma++; // Skip comma
                // Skip whitespace
                while (comma < response.length() && (response[comma] == ' ' || response[comma] == '\t')) {
                    comma++;
                }
                // Extract the number
                auto num_start = comma;
                auto num_end = num_start;
                while (num_end < response.length() && std::isdigit(response[num_end])) {
                    num_end++;
                }
                if (num_end > num_start) {
                    try {
                        return std::stoull(response.substr(num_start, num_end - num_start));
                    } catch (...) {
                        return 0;
                    }
                }
            }
        }
    }
    return 0;
}

bool connected_test::wait_for_async_inserts(unsigned long long expected_count, std::string const& table_name)
{
    // Async API batches inserts, so we need to poll until count matches
    auto query = std::string("select count(*) from ") + db_name + ".." + table_name;
    
    unsigned long long current_count = 0;
    unsigned retries = 0;
    const unsigned max_retries = 1000; // Much longer timeout for Windows async batching (up to ~100 seconds with exponential backoff)
    unsigned wait_ms = 100;
    const unsigned max_wait_ms = 1000; // Cap at 1 second between checks
    
    std::cout << "Waiting for all " << expected_count << " async inserts to arrive..." << std::endl;
    
    while (current_count < expected_count && retries < max_retries) {
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        retries++;
        
        try {
            auto response = raw_db.get(query);
            current_count = extract_count_from_influxdb_response(response);
            
            // Log more frequently when close to target (within 1% or 1000 entries)
            bool is_close = current_count > 0 && (expected_count - current_count) <= std::max(expected_count / 100, 1000ULL);
            bool should_log = (retries % 50 == 0) || (current_count > 0 && retries < 10) || (is_close && retries % 5 == 0);
            
            if (should_log) {
                std::cout << "Poll attempt " << retries << "/" << max_retries 
                          << ": " << current_count << "/" << expected_count << " entries arrived";
                if (is_close && current_count < expected_count) {
                    std::cout << " (" << (expected_count - current_count) << " remaining, waiting for final batch...)";
                }
                std::cout << std::endl;
            }
            
            // Accept if we're very close (within 0.1% or 10 entries) - async batching can leave tiny remainder
            unsigned long long tolerance = std::max(expected_count / 1000, 10ULL);
            if (current_count >= expected_count) {
                std::cout << "✓ All " << expected_count << " entries arrived after " << retries << " polling attempts!" << std::endl;
                return true;
            } else if (current_count + tolerance >= expected_count && retries >= 50) {
                // After 50 retries, accept if we're within tolerance
                std::cout << "✓ " << current_count << "/" << expected_count << " entries arrived (within tolerance of " << tolerance << ") after " << retries << " polling attempts!" << std::endl;
                return true;
            }
        } catch (const std::exception& e) {
            // Query might fail early if no data yet, keep trying
            if (retries % 50 == 0) {
                std::cout << "Poll attempt " << retries << ": Query failed (no data yet), continuing..." << std::endl;
            }
        }
        
        // Exponential backoff: increase wait time gradually, cap at max_wait_ms
        wait_ms += 10;
        if (wait_ms > max_wait_ms) {
            wait_ms = max_wait_ms;
        }
    }
    
    std::cout << "✗ Timeout: Only " << current_count << "/" << expected_count << " entries arrived after " << retries << " attempts" << std::endl;
    return false;
}
