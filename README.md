# influxdb-cpp-rest

A naive C++ [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk).

See [the demo source](src/demo/main.cpp) for the current api example.

The unbatched aprroach (and without connection reuse) may not be sufficient in some situations, as without batching, about 200 lines/sec can be inserted.

A batching api leans towards thousands inserts per second. Behind the scenes, the API uses [RxCpp](https://github.com/Reactive-Extensions/RxCpp) and [cppformat](https://github.com/fmtlib/fmt).


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
 