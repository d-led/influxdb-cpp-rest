cmake_minimum_required(VERSION 3.20)

project(
    influxdb-cpp-rest
    VERSION 1.0.1
    LANGUAGES CXX
    DESCRIPTION "A C++ client library for InfluxDB using C++ REST SDK"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer Conan packages over system packages for reproducibility
# Don't search system directories unless explicitly told to
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
# Make -isystem happen AFTER our includes (not for MSVC - it doesn't support -isystem)
if(NOT MSVC)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem" CACHE STRING "Flag to force system includes to be treated as system" FORCE)
endif()

# Build options
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DEMO "Build demo application" ON)
option(BUILD_BENCHMARK "Build benchmark application" OFF)
option(USE_CONAN "Use Conan for dependency management" ON)

# Include directories
include_directories(
    src/influxdb-cpp-rest
    src/influx-c-rest
)

# Find dependencies via Conan (if enabled)
# Remove system include directories to prefer Conan packages
# Conan 2.0 generates CMakeDeps and CMakeToolchain via conan install
if(USE_CONAN)
    # Try multiple possible locations for the toolchain
    # Conan 2.0 with cmake_layout generates in build/build/generators/ (Visual Studio multi-config)
    # or build/build/<build_type>/generators/ (single-config generators)
    set(CONAN_TOOLCHAIN_PATHS
        "${CMAKE_BINARY_DIR}/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/build/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/Release/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/Debug/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/build/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/build/Release/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/build/Debug/generators/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/generators/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/build/generators/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/build/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/build/Release/generators/conan_toolchain.cmake"
    )
    
    set(CONAN_TOOLCHAIN_FOUND FALSE)
    set(CONAN_TOOLCHAIN_FILE "")
    foreach(TOOLCHAIN_PATH ${CONAN_TOOLCHAIN_PATHS})
        if(EXISTS "${TOOLCHAIN_PATH}")
            message(STATUS "Found Conan toolchain at: ${TOOLCHAIN_PATH}")
            include("${TOOLCHAIN_PATH}")
            set(CONAN_TOOLCHAIN_FOUND TRUE)
            set(CONAN_TOOLCHAIN_FILE "${TOOLCHAIN_PATH}")
            break()
        endif()
    endforeach()
    
    if(NOT CONAN_TOOLCHAIN_FOUND)
        message(WARNING "Conan toolchain not found in any of these locations:")
        foreach(TOOLCHAIN_PATH ${CONAN_TOOLCHAIN_PATHS})
            message(WARNING "  - ${TOOLCHAIN_PATH}")
        endforeach()
        message(WARNING "Run 'conan install .. --output-folder=. -s build_type=Release' in the build directory first")
        message(WARNING "Disabling Conan integration - will use bundled dependencies")
        set(USE_CONAN OFF)
    else()
        # Set CMAKE_PREFIX_PATH to help find_package find Conan packages
        # Try multiple possible generator locations
        # For Visual Studio multi-config: build/build/generators/
        # For single-config: build/build/<build_type>/generators/ or build/generators/
        set(GENERATORS_DIR "")
        if(EXISTS "${CMAKE_BINARY_DIR}/build/build/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/build/generators")
        elseif(EXISTS "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators")
        elseif(EXISTS "${CMAKE_BINARY_DIR}/build/Release/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/Release/generators")
        elseif(EXISTS "${CMAKE_BINARY_DIR}/build/build/Release/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/build/Release/generators")
        elseif(EXISTS "${CMAKE_BINARY_DIR}/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/generators")
        endif()
        
        if(GENERATORS_DIR)
            list(APPEND CMAKE_PREFIX_PATH "${GENERATORS_DIR}")
            message(STATUS "Added to CMAKE_PREFIX_PATH: ${GENERATORS_DIR}")
        else()
            # Try to find generators directory
            file(GLOB_RECURSE GENERATOR_DIRS "${CMAKE_BINARY_DIR}/*/generators")
            if(GENERATOR_DIRS)
                get_filename_path(GEN_DIR "${GENERATOR_DIRS}")
                list(APPEND CMAKE_PREFIX_PATH "${GEN_DIR}")
                message(STATUS "Found generators directory: ${GEN_DIR}")
            endif()
        endif()
        
        # Fix macOS sysroot after toolchain inclusion - Conan toolchain sets it to "macosx" which doesn't work with Unix Makefiles
        if(APPLE)
            execute_process(
                COMMAND xcrun --sdk macosx --show-sdk-path
                OUTPUT_VARIABLE MACOS_SDK_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(MACOS_SDK_PATH)
                set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}" CACHE STRING "macOS SDK Path" FORCE)
                message(STATUS "Fixed macOS sysroot to: ${MACOS_SDK_PATH}")
            endif()
        endif()
    endif()
endif()

# Find packages (works with Conan's CMakeDeps generator)
if(USE_CONAN AND CONAN_TOOLCHAIN_FOUND)
    # Find packages - CMakeDeps generator creates *Config.cmake files
    # These should be in the generators directory
    find_package(Catch2 QUIET)
    find_package(cpprestsdk REQUIRED)
    find_package(rxcpp QUIET)
    find_package(OpenSSL REQUIRED)
    
    # Verify packages were found
    if(TARGET Catch2::Catch2)
        message(STATUS "Found Catch2 from Conan")
    else()
        message(STATUS "Catch2 from Conan not found - tests will fail if it's not available")
    endif()
    
    if(TARGET cpprestsdk::cpprestsdk)
        set(CONAN_LIBS 
            cpprestsdk::cpprestsdk
        )
        message(STATUS "Using Conan packages: cpprestsdk::cpprestsdk")
        # Add rxcpp if found from Conan
        if(TARGET rxcpp::rxcpp)
            list(APPEND CONAN_LIBS rxcpp::rxcpp)
            message(STATUS "Found rxcpp from Conan")
        endif()
        # Add openssl explicitly to ensure headers are available  
        if(TARGET OpenSSL::SSL)
            list(APPEND CONAN_LIBS OpenSSL::SSL OpenSSL::Crypto)
            message(STATUS "Found openssl from Conan")
        endif()
    else()
        message(FATAL_ERROR "Required Conan packages not found as targets")
        if(NOT TARGET cpprestsdk::cpprestsdk)
            message(FATAL_ERROR "  - cpprestsdk::cpprestsdk target not found")
        endif()
    endif()
else()
    # Fallback - but deps folder is removed, so this will fail
    message(FATAL_ERROR "USE_CONAN is OFF but bundling is no longer supported. Please enable USE_CONAN.")
endif()

# Static library: influxdb-cpp-rest
file(GLOB_RECURSE INFLUXDB_CPP_REST_SOURCES
    "src/influxdb-cpp-rest/*.cpp"
    "src/influxdb-cpp-rest/*.h"
)
list(FILTER INFLUXDB_CPP_REST_SOURCES EXCLUDE REGEX ".*\\.h$")

add_library(influxdb-cpp-rest STATIC
    ${INFLUXDB_CPP_REST_SOURCES}
)
target_include_directories(influxdb-cpp-rest PUBLIC
    src/influxdb-cpp-rest
)
target_link_libraries(influxdb-cpp-rest
    PUBLIC
        ${CONAN_LIBS}
)
# Workaround: Ensure OpenSSL include directories are available
# Boost (via cpprestsdk) requires OpenSSL headers, but Conan's OpenSSL CMakeDeps 
# might not properly set INTERFACE_INCLUDE_DIRECTORIES
if(TARGET OpenSSL::SSL AND USE_CONAN)
    # Find OpenSSL include directory - try multiple methods
    set(_OPENSSL_INCLUDE_DIR "")
    
    # Method 1: Check CMAKE_PREFIX_PATH (set by Conan generators)
    foreach(_PREFIX_PATH ${CMAKE_PREFIX_PATH})
        set(_POSSIBLE_INCLUDE "${_PREFIX_PATH}/include")
        if(EXISTS "${_POSSIBLE_INCLUDE}/openssl/ssl.h")
            set(_OPENSSL_INCLUDE_DIR "${_POSSIBLE_INCLUDE}")
            message(STATUS "Found OpenSSL includes via CMAKE_PREFIX_PATH: ${_OPENSSL_INCLUDE_DIR}")
            break()
        endif()
    endforeach()
    
    # Method 2: Read from Conan's generator data files
    if(NOT _OPENSSL_INCLUDE_DIR)
        file(GLOB _OPENSSL_DATA_FILES 
            "${CMAKE_BINARY_DIR}/OpenSSL-*-data.cmake"
            "${CMAKE_BINARY_DIR}/generators/OpenSSL-*-data.cmake"
            "${CMAKE_BINARY_DIR}/build/Release/generators/OpenSSL-*-data.cmake"
            "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators/OpenSSL-*-data.cmake"
        )
        if(_OPENSSL_DATA_FILES)
            list(GET _OPENSSL_DATA_FILES 0 _OPENSSL_DATA_FILE)
            file(READ "${_OPENSSL_DATA_FILE}" _OPENSSL_DATA_CONTENT)
            # Match both lowercase and mixed case variants
            string(REGEX MATCH "set\\([Oo]pen[Ss][Ss][Ll]_PACKAGE_FOLDER_RELEASE \"([^\"]+)\"\\)" _MATCH "${_OPENSSL_DATA_CONTENT}")
            if(_MATCH)
                string(REGEX REPLACE ".*PACKAGE_FOLDER_RELEASE \"([^\"]+)\".*" "\\1" _OPENSSL_PKG_FOLDER "${_MATCH}")
                set(_OPENSSL_INCLUDE_DIR "${_OPENSSL_PKG_FOLDER}/include")
                message(STATUS "Found OpenSSL includes via data file: ${_OPENSSL_INCLUDE_DIR}")
            endif()
        endif()
    endif()
    
    # Method 3: Use find_path to search in known locations
    if(NOT _OPENSSL_INCLUDE_DIR)
        find_path(_OPENSSL_INCLUDE_DIR 
            NAMES openssl/ssl.h
            PATHS ${CMAKE_PREFIX_PATH}
            PATH_SUFFIXES include
            NO_DEFAULT_PATH
        )
        if(_OPENSSL_INCLUDE_DIR)
            message(STATUS "Found OpenSSL includes via find_path: ${_OPENSSL_INCLUDE_DIR}")
        endif()
    endif()
    
    # Add the include directory directly to our target
    # This is critical because Boost's ASIO SSL tries to include openssl headers directly
    # Use PRIVATE since OpenSSL is only needed for the library's implementation, not its public API
    if(_OPENSSL_INCLUDE_DIR AND EXISTS "${_OPENSSL_INCLUDE_DIR}")
        target_include_directories(influxdb-cpp-rest PRIVATE "${_OPENSSL_INCLUDE_DIR}")
        message(STATUS "Added OpenSSL include directory to influxdb-cpp-rest (PRIVATE): ${_OPENSSL_INCLUDE_DIR}")
    else()
        message(WARNING "Could not find OpenSSL include directory - compilation may fail")
        message(WARNING "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
    endif()
endif()

# Shared library: influx-c-rest
file(GLOB_RECURSE INFLUX_C_REST_SOURCES
    "src/influx-c-rest/*.cpp"
)
add_library(influx-c-rest SHARED
    ${INFLUX_C_REST_SOURCES}
)
target_include_directories(influx-c-rest PUBLIC
    src/influx-c-rest
)
target_link_libraries(influx-c-rest
    PUBLIC
        influxdb-cpp-rest
)
if(USE_CONAN)
    target_link_libraries(influx-c-rest
        PUBLIC
            ${CONAN_LIBS}
    )
endif()
# OpenSSL includes are inherited from influxdb-cpp-rest via INTERFACE, so no need to add again
target_compile_definitions(influx-c-rest PRIVATE BUILDING_INFLUX_C_REST)

# Put DLL in bin directory (matching test executable location) so tests can find it
# This ensures test-influx-c-rest and test-influxdb-cpp-auth can find influx-c-rest.dll
# On Windows/MSVC (multi-config): bin/Release or bin/Debug via $<CONFIG>
# On Unix (single-config): bin/Release or bin/Debug via CMAKE_BUILD_TYPE
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generator (Visual Studio, Xcode) - use generator expression directly
    set_target_properties(influx-c-rest
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    )
else()
    # Single-config generator (Unix Makefiles, Ninja) - use CMAKE_BUILD_TYPE if set
    if(CMAKE_BUILD_TYPE)
        set_target_properties(influx-c-rest
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
        )
    else()
        set_target_properties(influx-c-rest
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
    endif()
endif()

# Demo application
if(BUILD_DEMO)
    add_executable(demo
        src/demo/main.cpp
    )
    target_link_libraries(demo
        influxdb-cpp-rest
    )
    if(USE_CONAN)
        target_link_libraries(demo
            ${CONAN_LIBS}
        )
    endif()
endif()

# Test executables
if(BUILD_TESTING)
    # Test: test-influxdb-cpp-rest
    file(GLOB_RECURSE TEST_SOURCES
        "src/test/*.cpp"
        "src/test/*.h"
    )
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*\\.h$")
    
    add_executable(test-influxdb-cpp-rest
        ${TEST_SOURCES}
    )
    target_include_directories(test-influxdb-cpp-rest PRIVATE
        src/test
    )
    target_link_libraries(test-influxdb-cpp-rest
        influxdb-cpp-rest
    )
    if(TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influxdb-cpp-rest
            Catch2::Catch2WithMain
        )
    else()
        message(FATAL_ERROR "Catch2 is required for tests but was not found. Install via Conan.")
    endif()
    if(USE_CONAN)
        target_link_libraries(test-influxdb-cpp-rest
            ${CONAN_LIBS}
        )
    endif()
    
    # Test: test-influx-c-rest
    file(GLOB_RECURSE TEST_SHARED_SOURCES
        "src/test-shared/*.cpp"
    )
    add_executable(test-influx-c-rest
        ${TEST_SHARED_SOURCES}
    )
    target_link_libraries(test-influx-c-rest
        influx-c-rest
    )
    if(TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influx-c-rest
            Catch2::Catch2WithMain
        )
    else()
        message(FATAL_ERROR "Catch2 is required for tests but was not found. Install via Conan.")
    endif()
    if(USE_CONAN)
        target_link_libraries(test-influx-c-rest
            ${CONAN_LIBS}
        )
    endif()
    
    # Test: test-influxdb-cpp-auth
    add_executable(test-influxdb-cpp-auth
        src/auth_test/auth_test.cpp
        src/test/fixtures.cpp
    )
    target_include_directories(test-influxdb-cpp-auth PRIVATE
        src/test
    )
    target_link_libraries(test-influxdb-cpp-auth
        influxdb-cpp-rest
        influx-c-rest
    )
    if(TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influxdb-cpp-auth
            Catch2::Catch2WithMain
        )
    else()
        message(FATAL_ERROR "Catch2 is required for tests but was not found. Install via Conan.")
    endif()
    if(USE_CONAN)
        target_link_libraries(test-influxdb-cpp-auth
            ${CONAN_LIBS}
        )
    endif()
endif()

# Enable testing and register tests (only if BUILD_TESTING is ON)
if(BUILD_TESTING)
    enable_testing()

    # Register tests with CTest
    add_test(NAME test-influxdb-cpp-rest COMMAND ${CMAKE_BINARY_DIR}/bin/test-influxdb-cpp-rest)
    add_test(NAME test-influx-c-rest COMMAND ${CMAKE_BINARY_DIR}/bin/test-influx-c-rest)
    add_test(NAME test-influxdb-cpp-auth COMMAND ${CMAKE_BINARY_DIR}/bin/test-influxdb-cpp-auth)

    # Set output directories for executables - match influx-c-rest DLL location
    if(CMAKE_CONFIGURATION_TYPES)
        # Multi-config generator (Visual Studio, Xcode)
        set_target_properties(test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        )
    else()
        # Single-config generator
        if(CMAKE_BUILD_TYPE)
            set_target_properties(test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
            )
        else()
            set_target_properties(test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            )
        endif()
    endif()
endif()

if(BUILD_DEMO)
    # Demo output directory - match test executables location
    if(CMAKE_CONFIGURATION_TYPES)
        # Multi-config generator (Visual Studio, Xcode)
        set_target_properties(demo
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        )
    else()
        # Single-config generator
        if(CMAKE_BUILD_TYPE)
            set_target_properties(demo
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
            )
        else()
            set_target_properties(demo
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            )
        endif()
    endif()
endif()

# Benchmark applications
if(BUILD_BENCHMARK)
    add_subdirectory(src/benchmark)
    
    # Benchmark output directory - match test executables location
    if(CMAKE_CONFIGURATION_TYPES)
        # Multi-config generator (Visual Studio, Xcode)
        set_target_properties(format_benchmark db_insert_benchmark db_batch_benchmark
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
        )
    else()
        # Single-config generator
        if(CMAKE_BUILD_TYPE)
            set_target_properties(format_benchmark db_insert_benchmark db_batch_benchmark
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
            )
        else()
            set_target_properties(format_benchmark db_insert_benchmark db_batch_benchmark
                PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            )
        endif()
    endif()
endif()

# Installation
install(TARGETS influxdb-cpp-rest influx-c-rest
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY src/influxdb-cpp-rest src/influx-c-rest
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)


