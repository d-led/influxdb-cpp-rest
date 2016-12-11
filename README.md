# influxdb-cpp-rest

[![Build status](https://ci.appveyor.com/api/projects/status/68w68vq3nai4794g/branch/master?svg=true)](https://ci.appveyor.com/project/d-led/influxdb-cpp-rest/branch/master)

A naive C++(14) [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk).

See [the demo source](src/demo/main.cpp) for the current api example.

The unbatched aprroach (and without connection reuse) may not be sufficient in some situations, as without batching, about 200 lines/sec can be inserted.

A batching api leans towards thousands inserts per second. Behind the scenes, the API uses [RxCpp](https://github.com/Reactive-Extensions/RxCpp) and [cppformat](https://github.com/fmtlib/fmt).

## Status

Build and test ok on Win10/Ubuntu64/OSX.

Feel free to contribute, as the progress is rather sporadic due to lack of spare time.

## Synchronous insertion

```cpp
influxdb::api::simple_db simpledb("http://localhost:8086", "my_db");
db.insert(
	line("log", key_value_pairs("my_tag", 42L), key_value_pairs("value", "hello world!")));
```

## Asynchronous insertion

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

## Query

```cpp
influxdb::api::simple_db simpledb("http://localhost:8086", "my_db");
auto query = std::string("select count(*) from my_db..my_measurements";
auto json_response = raw_db.get(query).find(std::to_string(count));
```

&darr;

```
{"results":[{"series":[{"name":"asynctest","columns":["time","count_value"],"values":[["...Z",123456]]}]}]}
```

## Build & Test

The library should be easy to build, given `RxCpp` and `cpprestsdk` can be found. The Visual Studio 2015 solution is self-contained. A locally running, authentication-free instance of InfluxDB is required to run the test.

### Dependencies on Linux and OS X

cpprestsdk needs to be built and available, which in turn has platform-specific transient dependencies.

The easiest way to install it on MacOS X and Linux turned out to be via [Homebrew](https://brew.sh) and [Linuxbrew](https://linuxbrew.sh) respectively.

Once the install `brew install cpprestsdk` succeeds, build: `make -C build/<platform>/gmake config=release_x64` and run the test.

If the build fails due to failed dependencies, check [premake5.lua](premake5.lua) for the build config, and regenerate makefiles if necessary via `premake/premake5<os-specific> gmake`
