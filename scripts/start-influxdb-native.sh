#!/usr/bin/env bash
# Start InfluxDB from a native binary (not Docker)
# Used on macOS-ARM where Docker is not available

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

INFLUXDB_VERSION="${INFLUXDB_VERSION:-1.8.10}"
INFLUXDB_BIN="./influxdb-${INFLUXDB_VERSION}-1/usr/bin/influxd"
INFLUXDB_CONFIG="influxdb.conf"

if [ ! -f "${INFLUXDB_BIN}" ]; then
    echo "ERROR: InfluxDB binary not found at ${INFLUXDB_BIN}"
    echo "Make sure InfluxDB has been downloaded and extracted."
    exit 1
fi

if [ ! -f "${INFLUXDB_CONFIG}" ]; then
    echo "ERROR: InfluxDB config not found at ${INFLUXDB_CONFIG}"
    exit 1
fi

echo "Starting InfluxDB in background..."
"${INFLUXDB_BIN}" -config "${INFLUXDB_CONFIG}" &
INFLUXDB_PID=$!
echo "InfluxDB PID: ${INFLUXDB_PID}"

# Export PID for cleanup (only if GITHUB_ENV is set, otherwise ignore)
if [ -n "${GITHUB_ENV:-}" ]; then
    echo "INFLUXDB_PID=${INFLUXDB_PID}" >> "${GITHUB_ENV}"
fi

# Give InfluxDB a moment to start the process
sleep 1

# Verify the process is still running
if ! kill -0 ${INFLUXDB_PID} 2>/dev/null; then
    echo "ERROR: InfluxDB process (PID ${INFLUXDB_PID}) failed to start or died immediately"
    exit 1
fi

echo "Waiting for InfluxDB to be ready (polling with exponential backoff)..."
max_attempts=60
attempt=0
wait_seconds=1

while [ $attempt -lt $max_attempts ]; do
    # Check if InfluxDB responds to /ping with HTTP 204
    # curl -f fails on HTTP error codes, but 204 (No Content) is success (2xx)
    # We explicitly check for 204 status code to be sure
    http_code=$(curl -s -o /dev/null -w "%{http_code}" -m 2 http://localhost:8086/ping 2>/dev/null || echo "000")
    
    if [ "${http_code}" = "204" ]; then
        echo "✓ InfluxDB is ready after $attempt polling attempts!"
        exit 0
    fi
    
    attempt=$((attempt + 1))
    echo "Attempt $attempt/$max_attempts: InfluxDB not ready yet (HTTP ${http_code}), waiting ${wait_seconds} second(s)..."
    
    # Verify process is still running before continuing to wait
    if ! kill -0 ${INFLUXDB_PID} 2>/dev/null; then
        echo "ERROR: InfluxDB process (PID ${INFLUXDB_PID}) died unexpectedly"
        exit 1
    fi
    
    sleep $wait_seconds
    
    # Exponential backoff: increase wait time, cap at 5 seconds
    wait_seconds=$((wait_seconds + 1))
    if [ $wait_seconds -gt 5 ]; then
        wait_seconds=5
    fi
done

echo "ERROR: InfluxDB failed to become ready after $max_attempts polling attempts"
echo "Last HTTP response code: ${http_code:-unknown}"
exit 1

