# influxdb-cpp-rest

A naive C++ [InfluxDB](https://www.influxdata.com/time-series-platform/influxdb/) client via [C++ REST SDK](https://github.com/Microsoft/cpprestsdk).

See [the demo source](src/demo/main.cpp) for the current api example.

The unbatched aprroach (and without connection reuse) may not be sufficient in some situations, as without batching, about 200 lines/sec can be inserted.

For potentially better performance, the requests can be batched on a separate thread via [cppzmq](https://github.com/zeromq/cppzmq)/[zeromq](http://zeromq.org/community)/[RxCpp](https://github.com/Reactive-Extensions/RxCpp)/[cppformat](https://github.com/fmtlib/fmt) (spike)
