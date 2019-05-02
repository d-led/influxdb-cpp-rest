#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# first parameter to the test script (directory with the test binaries) is mandatory
echo "running tests from $1"
pkill influxd || true
./influxdb-1.7.6-1/usr/bin/influxd &
sleep 3
$1/test-influxdb-cpp-rest -d yes
export LD_LIBRARY_PATH=$1
$1/test-influxdb-c-rest -d yes
pkill influxd
sleep 3
./influxdb-1.2.4-1/usr/bin/influxd -config src/auth_test/influxdb.conf &
sleep 3
$1/test-influxdb-cpp-auth -d yes
pkill influxd
