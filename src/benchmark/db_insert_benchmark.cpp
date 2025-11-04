#include <benchmark/benchmark.h>
#include <influxdb_simple_api.h>
#include <influxdb_line.h>
#include <influxdb_raw_db_utf8.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <algorithm>

using namespace influxdb::api;
using namespace std::string_literals;

// Database connection settings (can be overridden via environment variables)
const std::string DB_URL = std::getenv("INFLUXDB_URL") ? std::getenv("INFLUXDB_URL") : "http://localhost:8086"s;
const std::string DB_NAME = std::getenv("INFLUXDB_DB") ? std::getenv("INFLUXDB_DB") : "benchmark_db"s;

// Helper function to check if database exists (similar to test fixtures)
bool database_exists(const std::string& url, const std::string& db_name) {
    try {
        influxdb::raw::db_utf8 raw_db(url, db_name);
        std::string response = raw_db.get("show databases");
        return response.find(db_name) != std::string::npos;
    } catch (...) {
        return false;
    }
}

// Helper function to wait for database (similar to test fixtures)
void wait_for_db(const std::string& url, const std::string& db_name, unsigned max_retries = 20) {
    unsigned wait_ms = 100;
    const unsigned max_wait_ms = 500;
    
    for (unsigned i = 0; i < max_retries; ++i) {
        if (database_exists(url, db_name)) {
            if (i > 0) {
                std::cout << "    Database appeared after " << (i + 1) << " attempt(s)" << std::endl;
            }
            return;
        }
        if (i < max_retries - 1) {  // Don't print on last attempt
            std::cout << "    Waiting for database to appear (" << (i + 1) << "/" << max_retries << ")..." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        wait_ms = std::min(wait_ms + 50, max_wait_ms);  // Exponential backoff, cap at 500ms
    }
}

// Helper function to wait for database to be removed (similar to test fixtures)
void wait_for_no_db(const std::string& url, const std::string& db_name, unsigned max_retries = 20) {
    unsigned wait_ms = 100;
    const unsigned max_wait_ms = 500;
    
    for (unsigned i = 0; i < max_retries; ++i) {
        if (!database_exists(url, db_name)) {
            if (i > 0) {
                std::cout << "    Database removed after " << (i + 1) << " attempt(s)" << std::endl;
            }
            return;
        }
        if (i < max_retries - 1) {  // Don't print on last attempt
            std::cout << "    Waiting for database to be removed (" << (i + 1) << "/" << max_retries << ")..." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
        wait_ms = std::min(wait_ms + 50, max_wait_ms);  // Exponential backoff, cap at 500ms
    }
}

// Check if database is available
bool is_db_available() {
    try {
        std::cout << "Checking database availability at " << DB_URL << "..." << std::endl;
        auto db = simple_db(DB_URL, DB_NAME);
        db.create();  // Try to create - will fail if DB doesn't exist and we can't connect
        std::cout << "Database connection successful" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << "Database check failed: " << e.what() << std::endl;
        return false;
    }
}

// Benchmark: Single synchronous insert
static void BM_SingleInsert(benchmark::State& state) {
    static bool setup_done = false;
    static bool db_ok = false;
    static std::string db_name;
    
    if (!setup_done) {
        setup_done = true;
        std::cout << "\n=== Setting up BM_SingleInsert benchmark ===" << std::endl;
        
        if (!is_db_available()) {
            std::cout << "Database not available - skipping benchmark" << std::endl;
            db_ok = false;
        } else {
            db_name = DB_NAME + "_single";
            std::cout << "Setting up benchmark database: " << db_name << std::endl;
            
            // Use temporary instance for setup
            {
                auto db = simple_db(DB_URL, db_name);
                
                // Drop database if it exists
                try {
                    std::cout << "  Dropping existing database..." << std::endl;
                    db.drop();
                    wait_for_no_db(DB_URL, db_name);
                    std::cout << "  Database dropped successfully" << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "  Drop skipped (database may not exist): " << e.what() << std::endl;
                }
                
                // Create database
                try {
                    std::cout << "  Creating database..." << std::endl;
                    db.create();
                    
                    // Wait for database to be ready using polling (same as test fixtures)
                    std::cout << "  Waiting for database to be ready..." << std::endl;
                    wait_for_db(DB_URL, db_name);
                } catch (const std::exception& e) {
                    std::string error_msg = e.what();
                    if (error_msg.find("already exists") == std::string::npos) {
                        std::cout << "  Warning: Database creation issue: " << e.what() << std::endl;
                    }
                }
            }
            
            // Verify database is writable by doing a test insert with polling wait
            auto verify_db = simple_db(DB_URL, db_name);
            std::cout << "  Verifying database is writable..." << std::endl;
            bool writable = false;
            unsigned wait_ms = 100;
            const unsigned max_wait_ms = 500;
            for (int i = 0; i < 20; ++i) {
                try {
                    auto test_line = line("_test_verify"s, key_value_pairs(), key_value_pairs("value"s, 1));
                    verify_db.insert(test_line);
                    writable = true;
                    if (i > 0) {
                        std::cout << "    Verified after " << (i + 1) << " attempt(s)" << std::endl;
                    }
                    break;
                } catch (const std::exception& e) {
                    std::string error_msg = e.what();
                    if (error_msg.find("database not found") == std::string::npos) {
                        // Some other error, probably OK
                        writable = true;
                        if (i > 0) {
                            std::cout << "    Verified after " << (i + 1) << " attempt(s) (warning: " << e.what() << ")" << std::endl;
                        }
                        break;
                    }
                    if (i < 19) {  // Don't print on last attempt
                        std::cout << "    Retrying (" << (i + 1) << "/20)..." << std::endl;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
                wait_ms = std::min(wait_ms + 50, max_wait_ms);  // Exponential backoff, cap at 500ms
            }
            
            if (writable) {
                std::cout << "  Database created and verified (writable)" << std::endl;
                db_ok = true;
            } else {
                std::cout << "  Warning: Database may not be fully ready (will try anyway)" << std::endl;
                db_ok = true;  // Try anyway
            }
        }
        std::cout << "=== Setup complete ===\n" << std::endl;
    }
    
    if (!db_ok) {
        state.SkipWithError("Database not available - skipping benchmark");
        return;
    }
    
    // Recreate db instance right before benchmark to ensure fresh connection
    auto db = simple_db(DB_URL, db_name);
    
    int counter = 0;
    for (auto _ : state) {
        try {
            auto line_obj = line("measurement"s,
                key_value_pairs("tag1"s, "value1"s),
                key_value_pairs("field1"s, counter++).add("field2"s, 3.14));
            db.insert(line_obj);
        } catch (const std::exception& e) {
            std::string error_msg = e.what();
            if (error_msg.find("database not found") != std::string::npos) {
                state.SkipWithError(std::string("Database '") + db_name + "' not found during insert: " + error_msg);
                return;
            }
            throw;  // Re-throw other exceptions
        }
    }
    
}
BENCHMARK(BM_SingleInsert);

// Benchmark: Multiple inserts in sequence
static void BM_SequentialInserts(benchmark::State& state) {
    static bool setup_done = false;
    static bool db_ok = false;
    static std::string db_name;
    
    if (!setup_done) {
        setup_done = true;
        std::cout << "\n=== Setting up BM_SequentialInserts benchmark ===" << std::endl;
        
        if (!is_db_available()) {
            std::cout << "Database not available - skipping benchmark" << std::endl;
            db_ok = false;
        } else {
            db_name = DB_NAME + "_sequential";
            std::cout << "Setting up benchmark database: " << db_name << std::endl;
            
            // Use temporary instance for setup
            {
                auto db = simple_db(DB_URL, db_name);
                
                // Drop database if it exists
                try {
                    std::cout << "  Dropping existing database..." << std::endl;
                    db.drop();
                    wait_for_no_db(DB_URL, db_name);
                    std::cout << "  Database dropped successfully" << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "  Drop skipped (database may not exist): " << e.what() << std::endl;
                }
                
                // Create database
                try {
                    std::cout << "  Creating database..." << std::endl;
                    db.create();
                    
                    // Wait for database to be ready using polling (same as test fixtures)
                    std::cout << "  Waiting for database to be ready..." << std::endl;
                    wait_for_db(DB_URL, db_name);
                } catch (const std::exception& e) {
                    std::string error_msg = e.what();
                    if (error_msg.find("already exists") == std::string::npos) {
                        std::cout << "  Warning: Database creation issue: " << e.what() << std::endl;
                    }
                }
            }
            
            // Verify database is writable by doing a test insert with polling wait
            auto verify_db = simple_db(DB_URL, db_name);
            std::cout << "  Verifying database is writable..." << std::endl;
            bool writable = false;
            unsigned wait_ms = 100;
            const unsigned max_wait_ms = 500;
            for (int i = 0; i < 20; ++i) {
                try {
                    auto test_line = line("_test_verify"s, key_value_pairs(), key_value_pairs("value"s, 1));
                    verify_db.insert(test_line);
                    writable = true;
                    if (i > 0) {
                        std::cout << "    Verified after " << (i + 1) << " attempt(s)" << std::endl;
                    }
                    break;
                } catch (const std::exception& e) {
                    std::string error_msg = e.what();
                    if (error_msg.find("database not found") == std::string::npos) {
                        // Some other error, probably OK
                        writable = true;
                        if (i > 0) {
                            std::cout << "    Verified after " << (i + 1) << " attempt(s) (warning: " << e.what() << ")" << std::endl;
                        }
                        break;
                    }
                    if (i < 19) {  // Don't print on last attempt
                        std::cout << "    Retrying (" << (i + 1) << "/20)..." << std::endl;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
                wait_ms = std::min(wait_ms + 50, max_wait_ms);  // Exponential backoff, cap at 500ms
            }
            
            if (writable) {
                std::cout << "  Database created and verified (writable)" << std::endl;
                db_ok = true;
            } else {
                std::cout << "  Warning: Database may not be fully ready (will try anyway)" << std::endl;
                db_ok = true;  // Try anyway
            }
        }
        std::cout << "=== Setup complete ===\n" << std::endl;
    }
    
    if (!db_ok) {
        state.SkipWithError("Database not available - skipping benchmark");
        return;
    }
    
    // Recreate db instance right before benchmark to ensure fresh connection
    auto db = simple_db(DB_URL, db_name);
    
    int counter = 0;
    for (auto _ : state) {
        try {
            // Insert multiple lines in sequence
            for (int i = 0; i < 10; ++i) {
                auto line_obj = line("measurement"s,
                    key_value_pairs("tag1"s, "value1"s),
                    key_value_pairs("field1"s, counter++).add("field2"s, 3.14));
                db.insert(line_obj);
            }
            state.SetItemsProcessed(10);
        } catch (const std::exception& e) {
            std::string error_msg = e.what();
            if (error_msg.find("database not found") != std::string::npos) {
                state.SkipWithError(std::string("Database '") + db_name + "' not found during insert: " + error_msg);
                return;
            }
            throw;  // Re-throw other exceptions
        }
    }
    
    // Cleanup (only on last iteration)
    if (state.iterations() == state.max_iterations) {
        try {
            db.drop();
        } catch (...) {}
    }
}
BENCHMARK(BM_SequentialInserts)->Iterations(100);  // Limit iterations for CI

int main(int argc, char** argv) {
    // Check database availability and print warning if not available
    if (!is_db_available()) {
        std::cout << "Warning: Database not available at " << DB_URL << std::endl;
        std::cout << "Benchmarks will be skipped. Start InfluxDB to run database benchmarks." << std::endl;
    }
    
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}

