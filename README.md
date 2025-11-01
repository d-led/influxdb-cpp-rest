# influxdb-cpp-rest

[![CI](https://github.com/d-led/influxdb-cpp-rest/workflows/CI/badge.svg)](https://github.com/d-led/influxdb-cpp-rest/actions)

A modern C++20 [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) + a C wrapper of the asynchronous API as a shared library.

## C++ Standard Requirements

**This project requires C++20 or later.**

- Minimum compiler versions:
  - GCC 11+
  - Clang 14+
  - MSVC 2019+ (19.29+) with `/std:c++20` or later
- The project uses C++20
- All dependencies are C++20 compatible
- **Note:** The legacy C++03 version is available in the [`v0.0.1-legacy`](https://github.com/d-led/influxdb-cpp-rest/releases/tag/v0.0.1-legacy) tag

See [the demo source](src/demo/main.cpp) for the current api example.

The unbatched approach (and without connection reuse) may not be sufficient in some situations, as without batching, about 200 lines/sec can be inserted.

A batching api leans towards thousands inserts per second. Behind the scenes, the API uses [RxCpp](https://github.com/Reactive-Extensions/RxCpp) and [cppformat](https://github.com/fmtlib/fmt).

## Status

- Modern C++20 build system with CMake and Conan
- CI/CD with GitHub Actions (Linux, macOS, Windows)
- Tested with InfluxDB v1.8+

## Synchronous insertion

```cpp
using namespace influxdb::api;
using namespace std::string_literals;

auto db = simple_db("http://localhost:8086"s, "my_db"s);
db.insert(
    line("log"s, 
         key_value_pairs("my_tag"s, 42L), 
         key_value_pairs("value"s, "hello world!"s)));
```

## Asynchronous insertion

The asynchronous API inserts the points on an active object with automatic batching, thus increasing throughput.

```cpp
using namespace influxdb::api;  // For line, key_value_pairs
using namespace std::string_literals;
using async_db = influxdb::async_api::simple_db;  // Type alias to avoid ambiguity

auto db = async_db("http://localhost:8086"s, "my_db"s);

for (auto i = 0; i < 123456; ++i) {
    db.insert(
        line(
            "my_measurements"s,
            key_value_pairs("my_count"s, i % MAX_VALUES_PER_TAG),
            key_value_pairs("value"s, "hi!"s)
        )
    );
}
```

## C API

see [async_c_test.cpp](src/test-shared/async_c_test.cpp) and the related headers.

## Timestamps

Timestamps can be added as the last parameter to the `line` constructor, and only need to return
a serializable value on `TTimestamp::now()`. There is a default `std::chrono`-based implementation:

```cpp
using namespace influxdb::api;
using namespace std::string_literals;

line(
    "my_measurements"s,
    key_value_pairs("my_count"s, 42),
    key_value_pairs("value"s, "hi!"s),
    default_timestamp()  // Optional: uses std::chrono for timestamps
)
```

`MAX_VALUES_PER_TAG` for demo purposes here, as there [is such a maximum](https://docs.influxdata.com/influxdb/v1.4/administration/config#max-values-per-tag-100000) and it has to be observed by the clients.

## Multiple lines in synchronous API

Add lines using the `()` operator on the line:

```cpp
using namespace influxdb::api;
using namespace std::string_literals;

line
    ("multiple"s, key_value_pairs("v1"s, 1), key_value_pairs())
    ("multiple"s, key_value_pairs("v2"s, 2), key_value_pairs())
```

## Query

```cpp
using namespace influxdb::raw;
using namespace std::string_literals;

auto db = db_utf8("http://localhost:8086"s, "my_db"s);
auto query = "select count(*) from my_db..my_measurements"s;
auto json_response = db.get(query);
```

&darr;

```
{"results":[{"series":[{"name":"asynctest","columns":["time","count_value"],"values":[["...Z",123456]]}]}]}
```

## Authentication

Basic authentication can be used with all API variants:

```cpp
using namespace influxdb::raw;
using namespace std::string_literals;

auto db = db_utf8("http://localhost:8086"s, "my_db"s);
db.with_authentication("username"s, "password"s);
auto response = db.get("select * from my_measurements");
```

## Error Handling

- Synchronous C++ API will throw exceptions on HTTP errors
- Asynchronous APIs will drop inserts on HTTP errors and print to `stderr`
- C api tries to catch CPP exceptions and
  - print exceptions to `stderr`
  - return non-zero `int` or `nullptr` where sensible
- Open issue: [#18](https://github.com/d-led/influxdb-cpp-rest/issues/18)

## Build & Test

### Prerequisites

- CMake 3.20+
- Conan 2.0+
- C++20 compatible compiler
- Python 3.x (for Conan)
- Docker (for running tests)

### Quick Development Workflow

```bash
# 1. Install Conan (one-time setup)
pip install conan

# 2. Build the library
./scripts/build.sh Release        # Linux/macOS
# or
scripts\build.bat Release         # Windows

# 3. Start InfluxDB for testing
./scripts/start-influxdb.sh       # Linux/macOS
# or
scripts\start-influxdb.bat         # Windows

# 4. Run tests
./scripts/test.sh                  # Linux/macOS
# or
scripts\test.bat                   # Windows

# 5. Stop InfluxDB when done
./scripts/stop-influxdb.sh        # Linux/macOS
# or
scripts\stop-influxdb.bat         # Windows
```

**Using docker compose directly:**

```bash
# Linux/macOS
docker compose up -d              # Start InfluxDB
docker compose down               # Stop InfluxDB

# Windows
docker compose -f docker-compose.win.yml up -d    # Start InfluxDB
docker compose -f docker-compose.win.yml down     # Stop InfluxDB
```

For detailed development instructions, see [docs/development.md](docs/development.md).

## Thanks to

- @kirkshoop for indispensable help with [RxCpp](https://github.com/Reactive-Extensions/RxCpp)
- @nikkov for pointing out the missing essential features
- @promgamer, @garaemon for the identifiers PRs


## License

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fd-led%2Finfluxdb-cpp-rest.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fd-led%2Finfluxdb-cpp-rest?ref=badge_large)
