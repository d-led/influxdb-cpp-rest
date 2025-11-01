# Changelog

All notable changes to this project will be documented in this file.

## [1.0.1] - Unreleased

- Prepared for Conan Center

## [1.0.0] - 2025-11-01

### Added

- Initial Conan package release for Conan 2.0
- C++20 client library (`influxdb-cpp-rest`) for InfluxDB
- C wrapper library (`influx-c-rest`) providing a C API over the asynchronous C++ API
- Support for InfluxDB 1.x (tested with InfluxDB 1.8+)
- Synchronous and asynchronous (batched) insertion APIs
- Query API support
- Basic authentication support
- CMake integration via `find_package(influxdb-cpp-rest)`

### Technical Details

- **C++ Standard**: Requires C++20 or later
  - Minimum compiler versions:
    - GCC 11+
    - Clang 14+
    - MSVC 2019+ (19.29+) with `/std:c++20` or later
- **InfluxDB Compatibility**: Supports InfluxDB 1.x (tested with v1.8+)
- **Libraries Included**:
  - `influxdb-cpp-rest`: Static C++ library providing the core functionality
  - `influx-c-rest`: Shared C wrapper library for C interoperability
- **Dependencies**: 
  - cpprestsdk/2.10.19 (HTTP client)
  - rxcpp/4.1.1 (Reactive Extensions for asynchronous operations)

### Notes

- This package replaces the legacy C++03 version available as [`v0.0.1-legacy`](https://github.com/d-led/influxdb-cpp-rest/releases/tag/v0.0.1-legacy)
- The modern C++20 version provides improved performance, type safety, and modern C++ features
- The C wrapper (`influx-c-rest`) enables integration from C projects while maintaining the performance benefits of the underlying C++ implementation

---

## Legacy Versions

### [0.0.1-legacy] - Legacy Release

- Legacy C++03 compatible version
- Available as a GitHub release tag
- Not available as a Conan package

[1.0.0]: https://github.com/d-led/influxdb-cpp-rest/releases/tag/v1.0.0
[0.0.1-legacy]: https://github.com/d-led/influxdb-cpp-rest/releases/tag/v0.0.1-legacy

---

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
