cmake_minimum_required(VERSION 3.20)

# Set proper sysroot for macOS - Conan toolchain sets it to "macosx" which doesn't work with Unix Makefiles
# Get the actual SDK path before the toolchain overrides it with FORCE
if(APPLE AND NOT DEFINED CMAKE_OSX_SYSROOT)
    execute_process(
        COMMAND xcrun --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(MACOS_SDK_PATH)
        set(CMAKE_OSX_SYSROOT "${MACOS_SDK_PATH}" CACHE STRING "macOS SDK Path" FORCE)
    endif()
endif()

project(
    influxdb-cpp-rest
    VERSION 1.0.0
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
# Make -isystem happen AFTER our includes
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem" CACHE STRING "Flag to force system includes to be treated as system" FORCE)

# Build options
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DEMO "Build demo application" ON)
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
    # Conan 2.0 with cmake_layout generates in build/<build_type>/generators/
    set(CONAN_TOOLCHAIN_PATHS
        "${CMAKE_BINARY_DIR}/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/Release/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/Debug/generators/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/conan_toolchain.cmake"
        "${CMAKE_SOURCE_DIR}/build/generators/conan_toolchain.cmake"
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
        set(GENERATORS_DIR "")
        if(EXISTS "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators")
        elseif(EXISTS "${CMAKE_BINARY_DIR}/build/Release/generators")
            set(GENERATORS_DIR "${CMAKE_BINARY_DIR}/build/Release/generators")
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
    endif()
endif()

# Find packages (works with Conan's CMakeDeps generator)
if(USE_CONAN AND CONAN_TOOLCHAIN_FOUND)
    # Find packages - CMakeDeps generator creates *Config.cmake files
    # These should be in the generators directory
    find_package(Catch2 QUIET)
    find_package(cpprestsdk REQUIRED)
    find_package(fmt REQUIRED)
    
    # Verify packages were found
    set(USE_CONAN_CATCH2 FALSE)
    if(TARGET Catch2::Catch2)
        set(USE_CONAN_CATCH2 TRUE)
        message(STATUS "Found Catch2 from Conan")
    else()
        message(STATUS "Catch2 from Conan not found, will use bundled version for tests")
    endif()
    
    if(TARGET fmt::fmt AND TARGET cpprestsdk::cpprestsdk)
        set(CONAN_LIBS 
            cpprestsdk::cpprestsdk
            fmt::fmt
        )
        set(FMT_TARGET fmt::fmt)
        message(STATUS "Using Conan packages: fmt::fmt, cpprestsdk::cpprestsdk")
    else()
        message(WARNING "Conan packages not found as targets - falling back to bundled fmt")
        if(NOT TARGET fmt::fmt)
            message(WARNING "  - fmt::fmt target not found")
        endif()
        if(NOT TARGET cpprestsdk::cpprestsdk)
            message(WARNING "  - cpprestsdk::cpprestsdk target not found")
        endif()
        set(USE_CONAN OFF)
    endif()
else()
    # Fallback to bundled fmt (may not work with C++20)
    message(WARNING "Using bundled fmt - may not be compatible with C++20")
    add_library(fmt STATIC
        deps/fmt/src/format.cc
        deps/fmt/src/posix.cc
    )
    target_include_directories(fmt PUBLIC deps/fmt/include)
    set(FMT_TARGET fmt)
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
    deps/rxcpp/Rx/v2/src/rxcpp
)
if(NOT USE_CONAN)
    target_include_directories(influxdb-cpp-rest PUBLIC
        deps/fmt/include
    )
endif()
target_link_libraries(influxdb-cpp-rest
    PUBLIC
        ${FMT_TARGET}
)
if(USE_CONAN)
    target_link_libraries(influxdb-cpp-rest
        PUBLIC
            ${CONAN_LIBS}
    )
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
target_compile_definitions(influx-c-rest PRIVATE BUILDING_INFLUX_C_REST)

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
    if(USE_CONAN_CATCH2 AND TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influxdb-cpp-rest
            Catch2::Catch2WithMain
        )
    else()
        # Fallback to bundled catch
        target_include_directories(test-influxdb-cpp-rest PRIVATE
            deps/catch/single_include
        )
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
    if(USE_CONAN_CATCH2 AND TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influx-c-rest
            Catch2::Catch2WithMain
        )
    else()
        # Fallback to bundled catch
        target_include_directories(test-influx-c-rest PRIVATE
            deps/catch/single_include
        )
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
    if(USE_CONAN_CATCH2 AND TARGET Catch2::Catch2WithMain)
        target_link_libraries(test-influxdb-cpp-auth
            Catch2::Catch2WithMain
        )
    else()
        # Fallback to bundled catch
        target_include_directories(test-influxdb-cpp-auth PRIVATE
            deps/catch/single_include
        )
    endif()
    if(USE_CONAN)
        target_link_libraries(test-influxdb-cpp-auth
            ${CONAN_LIBS}
        )
    endif()
endif()

# Set output directories for executables
set_target_properties(test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(BUILD_DEMO)
    set_target_properties(demo
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
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


