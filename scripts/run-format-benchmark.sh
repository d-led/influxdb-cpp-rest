#!/usr/bin/env bash
# Download and extract InfluxDB binary
# Usage: download-influxdb.sh <version> <architecture>
# Example: download-influxdb.sh 1.8.10 darwin_amd64

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

CMAKE_ARGS="-DBUILD_BENCHMARK=ON" ./scripts/build.sh

 ./build/bin/Release/db_insert_benchmark
 