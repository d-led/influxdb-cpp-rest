# influxdb-cpp-rest

[![CI](https://github.com/d-led/influxdb-cpp-rest/workflows/CI/badge.svg)](https://github.com/d-led/influxdb-cpp-rest/actions)

A modern C++20 [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk) + a C wrapper of the asynchronous API as a shared library.

See [the demo source](src/demo/main.cpp) for the current api example.

The unbatched aprroach (and without connection reuse) may not be sufficient in some situations, as without batching, about 200 lines/sec can be inserted.

A batching api leans towards thousands inserts per second. Behind the scenes, the API uses [RxCpp](https://github.com/Reactive-Extensions/RxCpp) and [cppformat](https://github.com/fmtlib/fmt).

## Status

- Modern C++20 build system with CMake and Conan
- CI/CD with GitHub Actions (Linux, macOS, Windows)
- Tested with InfluxDB v1.8+

## Synchronous insertion

```cpp
influxdb::api::simple_db simpledb("http://localhost:8086", "my_db");
db.insert(
	line("log", key_value_pairs("my_tag", 42L), key_value_pairs("value", "hello world!")));
```

## Asynchronous insertion

The asynchronous API inserts the points on an active object with automatic batching, thus increasing throughput.

```cpp
influxdb::async_api::simple_db asyncdb("http://localhost:8086", "my_db");

for (int i = 0; i < 123456; i++) {
  asyncdb.insert(
    line(
      "my_measurements",
      key_value_pairs("my_count", i % MAX_VALUES_PER_TAG),
      key_value_pairs("value", "hi!")
    ));
}
```

## C API

see [async_c_test.cpp](src/test-shared/async_c_test.cpp) and the related headers.

## Timestamps

Timestamps can be added as the last parameter to the `line` constructor, and only need to return
a serializable value on `TTimestamp::now()`. There is a default `std::chrono`-based implementation:

```cpp
    line(
      "my_measurements",
      key_value_pairs("my_count", i % MAX_VALUES_PER_TAG),
      key_value_pairs("value", "hi!"),
      default_timestamp()
//    ^^^^^^^^^^^^^^^^^^^
    )
```

`MAX_VALUES_PER_TAG` for demo purposes here, as there [is such a maximum](https://docs.influxdata.com/influxdb/v1.4/administration/config#max-values-per-tag-100000) and it has to be observed by the clients.

## Multiple lines in synchronous API

Add lines using the `()` operator on the line:

```cpp
  line
    ("multiple", key_value_pairs("v1", 1), key_value_pairs())
    ("multiple", key_value_pairs("v2", 2), key_value_pairs())
```

## Query

```cpp
influxdb::raw::db_utf8 raw_db("http://localhost:8086", "my_db");
auto query = std::string("select count(*) from my_db..my_measurements");
auto json_response = raw_db.get(query);
```

&darr;

```
{"results":[{"series":[{"name":"asynctest","columns":["time","count_value"],"values":[["...Z",123456]]}]}]}
```

## Authentication

Basic authentication can be used with all API variants

```cpp
influxdb::raw::db_utf8 raw_db("http://localhost:8086", "my_db");
raw_db.with_authentication(username, password);
auto query = ...
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
docker compose up -d              # Start InfluxDB
docker compose down               # Stop InfluxDB
```

For detailed development instructions, see [docs/development.md](docs/development.md).

## Thanks to

- @kirkshoop for indispensable help with [RxCpp](https://github.com/Reactive-Extensions/RxCpp)
- @nikkov for pointing out the missing essential features
- @promgamer, @garaemon for the identifiers PRs


## License

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fd-led%2Finfluxdb-cpp-rest.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fd-led%2Finfluxdb-cpp-rest?ref=badge_large)
