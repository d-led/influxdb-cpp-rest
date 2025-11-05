#include <benchmark/benchmark.h>
#include <influxdb_simple_api.h>
#include <influxdb_simple_async_api.h>
#include <influxdb_line.h>
#include <influxdb_raw_db_utf8.h>
#include <influxdb_http_events.h>
#include <rxcpp/rx.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>
#include <regex>
#include <vector>
#include <mutex>
#include <iomanip>
#include <atomic>
#include <set>

using namespace influxdb::api;
using namespace influxdb::async_api;
using namespace std::string_literals;

// Database connection settings (can be overridden via environment variables)
const std::string DB_URL = std::getenv("INFLUXDB_URL") ? std::getenv("INFLUXDB_URL") : "http://localhost:8086"s;
const std::string DB_NAME = std::getenv("INFLUXDB_DB") ? std::getenv("INFLUXDB_DB") : "benchmark_db"s;

// Structure to store benchmark results for summary table
// Using InfluxDB terminology: "lines" (not "items" or "records")
struct BenchmarkResult {
    unsigned batch_size;
    unsigned batch_time_ms;
    double submit_rate_req_s;      // API calls per second (lines/s)
    double http_request_rate_req_s; // HTTP requests per second
    double http_bytes_rate_bytes_s; // Bytes per second
    double actual_throughput_req_s;  // Verified inserts per second (lines/s)
    unsigned long long http_successes;
    unsigned long long http_failures;
    unsigned long long http_bytes_sent;
    unsigned long long http_bytes_received;
    unsigned long long actual_count;  // Number of lines actually inserted
    unsigned long long submitted;      // Number of lines submitted
    unsigned long long submit_time_ms;
    unsigned long long total_time_ms;
    double efficiency;
    bool aborted;  // True if waiting was aborted due to count not increasing
};

// Global storage for results (protected by mutex for thread safety)
static std::mutex results_mutex;
static std::vector<BenchmarkResult> benchmark_results;

// Maximum number of lines to insert for each benchmark (using InfluxDB terminology: "lines" not "records")
// Each test is limited to max 10k lines and max 10 seconds to ensure fair comparison
// 
// Note on InfluxDB batch size limits (from https://docs.influxdata.com/influxdb/v1/guides/write_data/):
// - No hard limit on batch size, but InfluxDB docs recommend splitting files with >5,000 points
// - HTTP request timeout is 5 seconds by default (though we use longer timeouts)
// - Batching multiple points significantly improves performance
// - Our MAX_LINES of 10,000 is conservative and well above the 5,000 point recommendation
constexpr unsigned long long MAX_LINES = 10000;
constexpr std::chrono::milliseconds MAX_TEST_TIME{10000}; // 10 seconds max per test

// Helper function to extract count from InfluxDB JSON response
unsigned long long extract_count_from_response(const std::string& response) {
    // Parse count from JSON response
    // With count(*), InfluxDB returns multiple counts: {"values":[["1970-01-01T00:00:00Z",count1,count2,...]]}
    // We want the first count value (after the timestamp)
    // Pattern: "values":[["timestamp",COUNT,...
    // Example: "values":[["1970-01-01T00:00:00Z",10000,10000]]
    std::regex count_regex(R"("values":\[\["[^"]+",\s*(\d+))");
    std::smatch match;
    if (std::regex_search(response, match, count_regex)) {
        try {
            return std::stoull(match[1].str());
        } catch (...) {
            return 0;
        }
    }
    // Fallback: try simpler pattern - find first number after "values":[[
    std::regex fallback_regex(R"("values":\[\[[^,]*,\s*(\d+))");
    if (std::regex_search(response, match, fallback_regex)) {
        try {
            return std::stoull(match[1].str());
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

// Helper function to wait for async inserts to complete
// Returns the final count and a boolean indicating if waiting was aborted (count stopped increasing)
std::pair<unsigned long long, bool> wait_for_async_inserts(
    influxdb::raw::db_utf8& raw_db, 
    const std::string& db_name,
    const std::string& measurement,
    unsigned long long expected_count,
    std::chrono::milliseconds max_wait_time = std::chrono::milliseconds(10000)) {
    
    auto query = std::string("select count(*) from ") + db_name + ".." + measurement;
    unsigned long long current_count = 0;
    unsigned long long last_count = 0;
    unsigned no_change_count = 0;
    const unsigned MAX_NO_CHANGE_POLLS = 3;
    
    auto start_time = std::chrono::steady_clock::now();
    auto poll_interval = std::chrono::milliseconds(100);
    
    while (true) {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed >= max_wait_time) {
            // Timeout - return current count
            return {current_count, false};
        }
        
        std::this_thread::sleep_for(poll_interval);
        
        try {
            auto response = raw_db.get(query);
            current_count = extract_count_from_response(response);
            
            // If we've reached the expected count, we're done
            if (current_count >= expected_count) {
                return {current_count, false};
            }
            
            // Check if count is increasing
            if (current_count > last_count) {
                // Count increased, reset no-change counter
                no_change_count = 0;
                last_count = current_count;
            } else if (current_count == last_count && current_count > 0) {
                // Count hasn't changed
                no_change_count++;
                if (no_change_count >= MAX_NO_CHANGE_POLLS) {
                    // Count stopped increasing - abort waiting
                    return {current_count, true};
                }
            }
        } catch (const std::exception& e) {
            // Query might fail, but don't count this as "no change" if we haven't seen any count yet
            if (current_count == 0) {
                continue; // Keep trying if we haven't seen any data yet
            }
            // If we have seen data, increment no-change counter
            no_change_count++;
            if (no_change_count >= MAX_NO_CHANGE_POLLS) {
                return {current_count, true};
            }
        }
    }
}

// Helper function to check if database is available
bool is_db_available() {
    try {
        auto db = influxdb::api::simple_db(DB_URL, DB_NAME);
        db.create();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Generic benchmark for async inserts with different batching strategies
static void BM_AsyncBatchStrategy(benchmark::State& state) {
    static bool setup_done = false;
    static bool db_ok = false;
    
    // Get batch parameters from state
    unsigned batch_size = static_cast<unsigned>(state.range(0));
    unsigned batch_time_ms = static_cast<unsigned>(state.range(1));
    
    if (!setup_done) {
        setup_done = true;
        db_ok = is_db_available();
        if (!db_ok) {
            std::cout << "Database not available - skipping benchmarks" << std::endl;
        }
    }
    
    if (!db_ok) {
        state.SkipWithError("Database not available");
        return;
    }
    
    // Create unique database name for this benchmark run
    // Each benchmark uses its own database to avoid interference between tests
    std::string db_name = DB_NAME + "_batch_" + std::to_string(batch_size) + "_" + std::to_string(batch_time_ms);
    
    // Setup database
    {
        auto db = influxdb::api::simple_db(DB_URL, db_name);
        try {
            db.drop();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } catch (...) {
            // Ignore if database doesn't exist
        }
        
        try {
            db.create();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } catch (...) {
            // May already exist
        }
    }
    
    // Create async db with custom batching parameters
    auto async_db = influxdb::async_api::simple_db(DB_URL, db_name, batch_size, batch_time_ms);
    auto raw_db = influxdb::raw::db_utf8(DB_URL, db_name);
    
    // Subscribe to HTTP events to track successes and failures
    std::atomic<unsigned long long> http_successes{0};
    std::atomic<unsigned long long> http_failures{0};
    std::atomic<unsigned long long> http_total_bytes_sent{0};
    std::atomic<unsigned long long> http_total_bytes_received{0};
    std::atomic<unsigned long long> http_total_duration_ms{0};
    
    auto http_events_sub = async_db.http_events().subscribe(
        [&](const influxdb::api::http_result& result) {
            if (result.success) {
                http_successes.fetch_add(1, std::memory_order_relaxed);
                http_total_bytes_sent.fetch_add(result.bytes_sent, std::memory_order_relaxed);
                http_total_bytes_received.fetch_add(result.bytes_received, std::memory_order_relaxed);
                http_total_duration_ms.fetch_add(result.duration_ms.count(), std::memory_order_relaxed);
            } else {
                http_failures.fetch_add(1, std::memory_order_relaxed);
                std::cerr << "HTTP error: " << result.error_message << std::endl;
            }
        },
        [](std::exception_ptr ep) {
            try { std::rethrow_exception(ep); }
            catch (const std::exception& e) {
                std::cerr << "HTTP events observable error: " << e.what() << std::endl;
            }
        }
    );
    
    const std::string measurement = "batch_test";
    
    for (auto _ : state) {
        // Reset HTTP event counters
        http_successes.store(0);
        http_failures.store(0);
        http_total_bytes_sent.store(0);
        http_total_bytes_received.store(0);
        http_total_duration_ms.store(0);
        // Clear previous data
        try {
            raw_db.post(std::string("delete from ") + measurement + " where time < now()");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } catch (...) {
            // Ignore errors
        }
        
        // Record start time
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Insert lines, but limit to MAX_LINES and MAX_TEST_TIME
        unsigned long long lines_inserted = 0;
        for (unsigned long long i = 0; i < MAX_LINES; ++i) {
            // Check if we've exceeded max time
            auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
            if (elapsed >= MAX_TEST_TIME) {
                break; // Stop inserting if we've exceeded max time
            }
            
            async_db.insert(
                line(measurement,
                    key_value_pairs("tag1", i % 1000),
                    key_value_pairs("field1", static_cast<int>(i)).add("field2", 3.14)
                )
            );
            lines_inserted++;
        }
        
        // Record time when all inserts are submitted
        auto submit_time = std::chrono::high_resolution_clock::now();
        
        // Calculate remaining time for waiting (max 10 seconds total)
        auto elapsed_so_far = std::chrono::duration_cast<std::chrono::milliseconds>(submit_time - start_time);
        auto remaining_wait_time = MAX_TEST_TIME - elapsed_so_far;
        if (remaining_wait_time.count() < 0) {
            remaining_wait_time = std::chrono::milliseconds(0);
        }
        
        // Wait for all inserts to complete and be queryable in InfluxDB
        // Use the remaining time as max wait time, but with a minimum of 1 second
        auto max_wait_time = std::max(remaining_wait_time, std::chrono::milliseconds(1000));
        auto [actual_count, aborted] = wait_for_async_inserts(raw_db, db_name, measurement, lines_inserted, max_wait_time);
        
        // Record end time
        auto end_time = std::chrono::high_resolution_clock::now();
        
        // If waiting was aborted (count stopped increasing), mark this but continue to show results
        // This indicates that messages may have been dropped or InfluxDB stopped processing
        if (aborted) {
            // Note: We continue the benchmark to show partial results, but mark as aborted
            // This allows comparison of different batching strategies even when some fail
        }
        
        // Calculate metrics
        auto submit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(submit_time - start_time);
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Get HTTP event counts and bytes
        unsigned long long successes = http_successes.load();
        unsigned long long failures = http_failures.load();
        unsigned long long total_requests = successes + failures;
        unsigned long long bytes_sent = http_total_bytes_sent.load();
        unsigned long long bytes_received = http_total_bytes_received.load();
        
        double submit_rate_req_s = submit_duration.count() > 0 ? (lines_inserted * 1000.0) / submit_duration.count() : 0.0;
        double actual_throughput_req_s = total_duration.count() > 0 ? (actual_count * 1000.0) / total_duration.count() : 0.0;
        double http_request_rate_req_s = total_duration.count() > 0 && total_requests > 0 ? (total_requests * 1000.0) / total_duration.count() : 0.0;
        double http_bytes_rate_bytes_s = total_duration.count() > 0 && bytes_sent > 0 ? (bytes_sent * 1000.0) / total_duration.count() : 0.0;
        
        // Report metrics (using InfluxDB terminology: "lines" instead of "items")
        state.counters["submitted_lines"] = benchmark::Counter(lines_inserted);
        state.counters["actual_inserted_lines"] = benchmark::Counter(static_cast<double>(actual_count));
        state.counters["http_successes"] = benchmark::Counter(static_cast<double>(successes));
        state.counters["http_failures"] = benchmark::Counter(static_cast<double>(failures));
        state.counters["http_bytes_sent"] = benchmark::Counter(static_cast<double>(bytes_sent));
        state.counters["submit_time_ms"] = benchmark::Counter(static_cast<double>(submit_duration.count()));
        state.counters["total_time_ms"] = benchmark::Counter(static_cast<double>(total_duration.count()));
        state.counters["submit_rate_lines_per_s"] = benchmark::Counter(submit_rate_req_s, benchmark::Counter::kIsRate);
        state.counters["insert_rate_lines_per_s"] = benchmark::Counter(actual_throughput_req_s, benchmark::Counter::kIsRate);
        state.counters["http_request_rate_req_s"] = benchmark::Counter(http_request_rate_req_s, benchmark::Counter::kIsRate);
        state.counters["http_bytes_rate_bytes_s"] = benchmark::Counter(http_bytes_rate_bytes_s, benchmark::Counter::kIsRate);
        
        // Note: We don't use SetItemsProcessed() to avoid Google Benchmark's "items_per_second" terminology
        // Instead, we use custom counters with "lines_per_s" to use InfluxDB terminology
        
        // Store results for summary table (only once per benchmark configuration)
        // Google Benchmark runs multiple iterations, we only want to store once per benchmark configuration
        static thread_local std::set<std::pair<unsigned, unsigned>> stored_configs;
        auto config_key = std::make_pair(batch_size, batch_time_ms);
        
        // Store on first iteration of each configuration
        if (state.iterations() == 1 && stored_configs.find(config_key) == stored_configs.end()) {
            stored_configs.insert(config_key);
            
            BenchmarkResult result;
            result.batch_size = batch_size;
            result.batch_time_ms = batch_time_ms;
            result.submit_rate_req_s = submit_rate_req_s;
            result.actual_throughput_req_s = actual_throughput_req_s;
            result.http_request_rate_req_s = http_request_rate_req_s;
            result.http_bytes_rate_bytes_s = http_bytes_rate_bytes_s;
            result.http_successes = successes;
            result.http_failures = failures;
            result.http_bytes_sent = bytes_sent;
            result.http_bytes_received = bytes_received;
            result.actual_count = actual_count;
            result.submitted = lines_inserted;
            result.submit_time_ms = submit_duration.count();
            result.total_time_ms = total_duration.count();
            result.efficiency = lines_inserted > 0 ? (actual_count * 100.0 / lines_inserted) : 0.0;
            result.aborted = aborted;
            
            {
                std::lock_guard<std::mutex> lock(results_mutex);
                benchmark_results.push_back(result);
            }
        }
    }
    
    // Cleanup
    try {
        auto db = influxdb::api::simple_db(DB_URL, db_name);
        db.drop();
    } catch (...) {
        // Ignore cleanup errors
    }
}

// Register benchmarks with different batching strategies
// Format: BM_AsyncBatchStrategy(batch_size, batch_time_ms)
// Note: batch_size must be <= MAX_LINES (10000) to be meaningful in this benchmark
//       If batch_size > MAX_LINES, batching will be purely time-based
// Based on InfluxDB hardware sizing guide: https://docs.influxdata.com/influxdb/v1/guides/hardware_sizing/
// Targeting realistic throughputs for different hardware tiers

// Small batch strategies (targeting < 5,000 writes/sec on modest hardware)
BENCHMARK(BM_AsyncBatchStrategy)->Args({100, 1000})->ArgNames({"lines", "ms"});       // Batch every 100 lines OR 1 second
BENCHMARK(BM_AsyncBatchStrategy)->Args({500, 1000})->ArgNames({"lines", "ms"});       // Batch every 500 lines OR 1 second
BENCHMARK(BM_AsyncBatchStrategy)->Args({1000, 1000})->ArgNames({"lines", "ms"});     // Batch every 1000 lines OR 1 second

// Medium batch strategies (targeting < 250,000 writes/sec on moderate hardware)
BENCHMARK(BM_AsyncBatchStrategy)->Args({1000, 100})->ArgNames({"lines", "ms"});       // Batch every 1000 lines OR 100ms
BENCHMARK(BM_AsyncBatchStrategy)->Args({5000, 100})->ArgNames({"lines", "ms"});       // Batch every 5000 lines OR 100ms
BENCHMARK(BM_AsyncBatchStrategy)->Args({10000, 100})->ArgNames({"lines", "ms"});       // Batch every 10000 lines OR 100ms (max batch size)

// Large batch strategies (targeting > 250,000 writes/sec on high-end hardware)
// Note: These use MAX_LINES as batch_size since we're limited to 10k lines per test
BENCHMARK(BM_AsyncBatchStrategy)->Args({10000, 50})->ArgNames({"lines", "ms"});       // Batch every 10000 lines OR 50ms

// Time-based only strategies (for low-rate, time-critical scenarios)
// Using MAX_LINES as batch_size since we'll never reach it - purely time-based batching
BENCHMARK(BM_AsyncBatchStrategy)->Args({10000, 100})->ArgNames({"lines", "ms"});      // Batch every 100ms (time-based, batch_size=MAX_LINES)
BENCHMARK(BM_AsyncBatchStrategy)->Args({10000, 1000})->ArgNames({"lines", "ms"});     // Batch every 1 second (time-based, batch_size=MAX_LINES)

// Helper function to print executive summary
void print_executive_summary() {
    std::lock_guard<std::mutex> lock(results_mutex);
    
    if (benchmark_results.empty()) {
        std::cerr << "WARNING: No benchmark results collected - executive summary is empty!" << std::endl;
        return;
    }
    
    // Find best performing strategy
    auto best_it = std::max_element(benchmark_results.begin(), benchmark_results.end(),
        [](const BenchmarkResult& a, const BenchmarkResult& b) {
            return a.actual_throughput_req_s < b.actual_throughput_req_s;
        });
    
    // Count aborted tests
    size_t aborted_count = std::count_if(benchmark_results.begin(), benchmark_results.end(),
        [](const BenchmarkResult& r) { return r.aborted; });
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "EXECUTIVE SUMMARY" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Total tests run: " << benchmark_results.size() << std::endl;
    std::cout << "Aborted tests: " << aborted_count << std::endl;
    
    if (best_it != benchmark_results.end()) {
        std::cout << "\nBest performing strategy (by insert rate):" << std::endl;
        std::cout << "  Batch size: " << best_it->batch_size << " lines" << std::endl;
        std::cout << "  Time window: " << best_it->batch_time_ms << " ms" << std::endl;
        std::cout << "  Insert rate: " << std::fixed << std::setprecision(0) << best_it->actual_throughput_req_s << " lines/s" << std::endl;
        std::cout << "  HTTP rate: " << std::fixed << std::setprecision(1) << best_it->http_request_rate_req_s << " req/s" << std::endl;
        std::cout << "  HTTP throughput: " << std::fixed << std::setprecision(2) << (best_it->http_bytes_rate_bytes_s / (1024.0 * 1024.0)) << " MB/s" << std::endl;
        std::cout << "  Efficiency: " << std::fixed << std::setprecision(1) << best_it->efficiency << "%" << std::endl;
        std::cout << "  Note: 'actual_inserted_lines' in benchmark output is cumulative across iterations" << std::endl;
        std::cout << "        Use 'insert_rate_lines_per_s' to compare performance" << std::endl;
    }
    std::cout << std::string(80, '=') << std::endl;
}

// Helper function to print summary table
void print_summary_table() {
    std::lock_guard<std::mutex> lock(results_mutex);
    
    if (benchmark_results.empty()) {
        std::cout << "\nNo benchmark results to display.\n" << std::endl;
        return;
    }
    
    std::cout << "\n" << std::string(150, '=') << std::endl;
    std::cout << "BATCHING STRATEGY PERFORMANCE SUMMARY" << std::endl;
    std::cout << std::string(150, '=') << std::endl;
    
    // Table header
    std::cout << std::left 
              << std::setw(10) << "Batch"
              << std::setw(10) << "Time"
              << std::setw(15) << "API (lines/s)"
              << std::setw(15) << "HTTP (req/s)"
              << std::setw(18) << "HTTP (MB/s)"
              << std::setw(15) << "Insert (lines/s)"
              << std::setw(15) << "HTTP Success/Fail"
              << std::setw(12) << "Lines"
              << std::setw(10) << "Efficiency"
              << std::setw(10) << "Status"
              << std::endl;
    std::cout << std::string(150, '-') << std::endl;
    
    // Sort by actual throughput (descending)
    std::sort(benchmark_results.begin(), benchmark_results.end(),
              [](const BenchmarkResult& a, const BenchmarkResult& b) {
                  return a.actual_throughput_req_s > b.actual_throughput_req_s;
              });
    
    // Print results
    for (const auto& result : benchmark_results) {
        std::string http_status = std::to_string(result.http_successes);
        if (result.http_failures > 0) {
            http_status += "/" + std::to_string(result.http_failures);
        }
        
        // Format bytes rate as MB/s
        double mb_per_sec = result.http_bytes_rate_bytes_s / (1024.0 * 1024.0);
        
        std::string status = result.aborted ? "ABORTED" : "OK";
        
        std::cout << std::left 
                  << std::setw(10) << result.batch_size
                  << std::setw(10) << result.batch_time_ms
                  << std::setw(15) << std::fixed << std::setprecision(0) << result.submit_rate_req_s
                  << std::setw(15) << std::fixed << std::setprecision(0) << result.http_request_rate_req_s
                  << std::setw(18) << std::fixed << std::setprecision(2) << mb_per_sec
                  << std::setw(15) << std::fixed << std::setprecision(0) << result.actual_throughput_req_s
                  << std::setw(15) << http_status
                  << std::setw(12) << (std::to_string(result.actual_count) + "/" + std::to_string(result.submitted) + " lines")
                  << std::setw(10) << std::fixed << std::setprecision(1) << result.efficiency << "%"
                  << std::setw(10) << status
                  << std::endl;
    }
    
    std::cout << std::string(150, '=') << std::endl;
    std::cout << "\nNotes:" << std::endl;
    std::cout << "  - Each test is limited to max 10k lines and max 10 seconds for fair comparison" << std::endl;
    std::cout << "  - API (lines/s) = async_db.insert() call rate (submission rate of lines)" << std::endl;
    std::cout << "  - HTTP (req/s) = actual HTTP request rate (from observable events)" << std::endl;
    std::cout << "  - HTTP (MB/s) = bytes sent per second (from HTTP events)" << std::endl;
    std::cout << "  - Insert (lines/s) = verified database insert rate of lines (from query count)" << std::endl;
    std::cout << "  - HTTP Success/Fail = number of successful/failed HTTP requests" << std::endl;
    std::cout << "  - Status: ABORTED = count stopped increasing (messages may have been dropped)" << std::endl;
    std::cout << "  - Note: Google Benchmark may display 'items_per_second' which refers to lines" << std::endl;
}

int main(int argc, char** argv) {
    // Check database availability and print warning if not available
    if (!is_db_available()) {
        std::cout << "Warning: Database not available at " << DB_URL << std::endl;
        std::cout << "Benchmarks will be skipped. Start InfluxDB to run database benchmarks." << std::endl;
    }
    
    // Note: Google Benchmark may print warnings about clock rate and thread affinity on macOS
    // These are harmless and don't affect measurements - they're just metadata warnings
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    
    // Run benchmarks - each test uses its own database to avoid interference
    ::benchmark::RunSpecifiedBenchmarks();
    
    // Flush stdout to ensure benchmark output is complete before printing summary
    std::cout.flush();
    std::cerr.flush();
    
    // Print executive summary first
    print_executive_summary();
    
    // Print detailed summary table after all benchmarks complete
    print_summary_table();
    
    ::benchmark::Shutdown();
    return 0;
}

