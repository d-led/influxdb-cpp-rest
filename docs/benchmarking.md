# Formatting Benchmark

This benchmark measures the performance of `std::format` operations used in the library.

## Building

### Using build scripts (recommended)

```bash
# Linux/macOS
CMAKE_ARGS="-DBUILD_BENCHMARK=ON" ./scripts/build.sh

# Or with explicit build type
BUILD_TYPE=Release CMAKE_ARGS="-DBUILD_BENCHMARK=ON" ./scripts/build.sh

# Windows
set CMAKE_ARGS=-DBUILD_BENCHMARK=ON
scripts\build.bat
```

### Manual build

```bash
# Build with benchmark enabled
cmake -B build -DBUILD_BENCHMARK=ON
cmake --build build --config Release
```

## Running

```bash
# Linux/macOS
./build/bin/Release/format_benchmark

# Windows
build\bin\Release\format_benchmark.exe
```

**Note:** By default, each benchmark runs for at least 0.5 seconds (or until it reaches a stable measurement). To run faster benchmarks in CI, use:
```bash
./build/bin/Release/format_benchmark --benchmark_min_time=0.5
```

## Results

The benchmark measures:
- `BM_FormatKVP`: Formatting key-value pairs (e.g., `"tag1=value1"`)
- `BM_FormatLine`: Formatting complete InfluxDB line protocol lines
- `BM_FormatCombined`: Combined formatting operations

Results show time per iteration, CPU time, and iterations per second.
