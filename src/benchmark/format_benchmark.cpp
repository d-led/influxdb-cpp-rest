#include <benchmark/benchmark.h>
#include <influxdb_line.h>
#include <string>

// Benchmark the library's key_value_pairs formatting.
static void BM_FormatKVP(benchmark::State& state) {
    for (auto _ : state) {
        auto kvp = influxdb::api::key_value_pairs("tag1", "value1");
        benchmark::DoNotOptimize(kvp.get());
    }
}
BENCHMARK(BM_FormatKVP);

// Benchmark the library's line formatting (integer field).
static void BM_FormatLine(benchmark::State& state) {
    for (auto _ : state) {
        auto l = influxdb::api::line("measurement",
                                     influxdb::api::key_value_pairs("tag1", "value1"),
                                     influxdb::api::key_value_pairs("field1", 42));
        benchmark::DoNotOptimize(l.get());
    }
}
BENCHMARK(BM_FormatLine);

// Benchmark the library's line formatting (integer + floating-point fields).
static void BM_FormatCombined(benchmark::State& state) {
    for (auto _ : state) {
        auto l = influxdb::api::line("measurement",
                                     influxdb::api::key_value_pairs("tag1", "value1"),
                                     influxdb::api::key_value_pairs("field1", 42).add("field2", 3.14));
        benchmark::DoNotOptimize(l.get());
    }
}
BENCHMARK(BM_FormatCombined);

BENCHMARK_MAIN();

