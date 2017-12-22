cmake_minimum_required (VERSION 3.9)

project (influxdb-cpp-rest CXX)

# we need this for tests
# add_subdirectory(deps/catch)

# Prepare "Catch" library
# set(CATCH_INCLUDE_DIR deps/catch/single_include)
# add_library(Catch INTERFACE)
# target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# todo: remove comments below
# add_subdirectory(deps/fmt)
# add_subdirectory(deps/rxcpp)
# target_include_directories(RxCpp INTERFACE deps/rxcpp/Rx/v2/src/rxcpp)

include_directories(deps/rxcpp/Rx/v2/src/rxcpp)

# Uncomment when cpprestsdk has support for cmake find_package()
# find_package(cpprestsdk REQUIRED)

file(GLOB_RECURSE influx_source_files src/influxdb-cpp-rest/*.cpp)

add_library(influxdb-cpp-rest STATIC ${influx_source_files})
target_link_libraries(influxdb-cpp-rest CONAN_PKG::cpprestsdk fmt)

target_include_directories(influxdb-cpp-rest INTERFACE "${THIRD_PARTY_MODULE_PATH}/influxdb-cpp-rest/src/influxdb-cpp-rest")
