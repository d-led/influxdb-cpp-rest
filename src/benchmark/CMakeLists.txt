cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use Google Benchmark via FetchContent
include(FetchContent)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Formatting benchmark (no database required)
add_executable(format_benchmark format_benchmark.cpp)
target_compile_features(format_benchmark PRIVATE cxx_std_20)
target_link_libraries(format_benchmark PRIVATE benchmark::benchmark)

# Database insert benchmark (requires InfluxDB)
add_executable(db_insert_benchmark db_insert_benchmark.cpp)
target_compile_features(db_insert_benchmark PRIVATE cxx_std_20)
target_link_libraries(db_insert_benchmark PRIVATE 
    benchmark::benchmark
    influxdb-cpp-rest
)
if(USE_CONAN)
    target_link_libraries(db_insert_benchmark PRIVATE ${CONAN_LIBS})
endif()

