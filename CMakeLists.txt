cmake_minimum_required (VERSION 3.9)

project (influxdb-cpp-rest CXX)

option(BUILD_TESTS "build and run tests" OFF)
option(BUILD_EXTERNAL_DEPS "build and run tests" OF)

if(BUILD_TESTS)
    # we need this for tests
    # add_subdirectory(deps/catch)

    # Prepare "Catch" library
    # set(CATCH_INCLUDE_DIR deps/catch/single_include)
    # add_library(Catch INTERFACE)
    # target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
endif()

if(BUILD_EXTERNAL_DEPS)
    add_subdirectory(deps/fmt)
    include_directories(deps/rxcpp/Rx/v2/src/) # fix it with find_package
else()
#    find_package(fmt REQUIRED) # uncomment when supported
endif()

if(!WIN32)
find_package(cpprestsdk REQUIRED)
endif()

file(GLOB_RECURSE influx_source_files src/influxdb-cpp-rest/*.cpp)

if(UNIX)
add_library(influxdb-cpp-rest SHARED ${influx_source_files})
else(UNIX)
add_library(influxdb-cpp-rest STATIC ${influx_source_files})
endif()

#target_link_libraries(influxdb-cpp-rest cpprestsdk::cpprest fmt dl) # there is some problem with linking... including dl
target_link_libraries(influxdb-cpp-rest CONAN_PKG::cpprestsdk CONAN_PKG::fmt CONAN_PKG::rxcpp)

if(WIN32)
    target_link_libraries(influxdb-cpp-rest bcrypt winhttp)
endif()

target_include_directories(influxdb-cpp-rest INTERFACE "src/influxdb-cpp-rest")
