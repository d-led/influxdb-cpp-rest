#include <benchmark/benchmark.h>
#include <format>
#include <string>

// Simulate key_value_pairs formatting: "key=value"
static void BM_FormatKVP(benchmark::State& state) {
    for (auto _ : state) {
        std::string out;
        out.reserve(100);
        std::format_to(std::back_inserter(out), "{}={}", "tag1", "value1");
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(BM_FormatKVP);

// Simulate line formatting: "measurement,tag1=value1 field1=42i"
static void BM_FormatLine(benchmark::State& state) {
    for (auto _ : state) {
        std::string out;
        out.reserve(100);
        std::format_to(std::back_inserter(out), "{},{}={} {}={}i", 
                       "measurement", "tag1", "value1", "field1", 42);
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(BM_FormatLine);

// Combined: simulate a typical usage pattern
static void BM_FormatCombined(benchmark::State& state) {
    for (auto _ : state) {
        std::string out;
        out.reserve(100);
        
        // Simulate key_value_pairs formatting
        std::format_to(std::back_inserter(out), "{}={}", "tag1", "value1");
        
        // Simulate line formatting
        std::format_to(std::back_inserter(out), "{},{}={} {}={}i", 
                       "measurement", "tag1", "value1", "field1", 42);
        
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(BM_FormatCombined);

BENCHMARK_MAIN();

