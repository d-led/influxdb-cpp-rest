# influxdb-cpp-rest

A naive C++ [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk).

See [the demo source](src/demo/main.cpp) for the current api example.

The current approach can be considered abandoned, as without batching, about 200 lines/sec can be inserted.
