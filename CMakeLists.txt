cmake_minimum_required (VERSION 3.9)

project (influxdb-cpp-rest CXX)

add_subdirectory(deps/catch)
# todo: remove comment below
# add_subdirectory(deps/fmt)
add_subdirectory(deps/rxcpp)
target_include_directories(RxCpp INTERFACE deps/rxcpp/Rx/v2/src/rxcpp)

file(GLOB_RECURSE influx_source_files src/influxdb-cpp-rest/*.cpp)

# Uncomment when cpprestsdk has support for cmake find_package()
# find_package(cpprestsdk REQUIRED)

add_library(influxdb-cpp-rest STATIC ${influx_source_files})
target_link_libraries(influxdb-cpp-rest CONAN_PKG::cpprestsdk fmt RxCpp)

target_include_directories(influxdb-cpp-rest INTERFACE "${THIRD_PARTY_MODULE_PATH}/influxdb-cpp-rest/src/influxdb-cpp-rest")
