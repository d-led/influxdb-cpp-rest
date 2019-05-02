#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

./influxdb-1.7.6-1/usr/bin/influxd &
sleep 1
$1/test-influxdb-cpp-rest -d yes
export LD_LIBRARY_PATH=$1
$1/test-influxdb-c-rest -d yes
pkill influxd
./influxdb-1.2.4-1/usr/bin/influxd -config src/auth_test/influxdb.conf &
sleep 1
$1/test-influxdb-cpp-auth -d yes
pkill influxd
