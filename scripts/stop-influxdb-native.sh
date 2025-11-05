#!/usr/bin/env bash
# Stop native InfluxDB and optionally clean up storage
# Usage: stop-influxdb-native.sh [--clean]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

CLEAN_STORAGE=false
if [ "${1:-}" = "--clean" ]; then
    CLEAN_STORAGE=true
fi

# Find InfluxDB process
INFLUXDB_PID=$(pgrep -f "influxd.*-config.*influxdb.conf" || echo "")

if [ -z "${INFLUXDB_PID}" ]; then
    echo "No InfluxDB process found running with influxdb.conf"
    # Still try to clean if requested
    if [ "$CLEAN_STORAGE" = true ]; then
        if [ -d ".influxdb" ]; then
            echo "Cleaning up InfluxDB storage..."
            rm -rf .influxdb
            echo "✓ InfluxDB storage cleaned"
        else
            echo "No .influxdb directory found"
        fi
    fi
    exit 0
fi

echo "Stopping InfluxDB (PID: ${INFLUXDB_PID})..."

# Try graceful shutdown first
kill "${INFLUXDB_PID}" 2>/dev/null || true

# Wait for process to exit
timeout=10
elapsed=0
while [ $elapsed -lt $timeout ]; do
    if ! kill -0 "${INFLUXDB_PID}" 2>/dev/null; then
        echo "✓ InfluxDB stopped gracefully"
        break
    fi
    sleep 1
    elapsed=$((elapsed + 1))
done

# Force kill if still running
if kill -0 "${INFLUXDB_PID}" 2>/dev/null; then
    echo "Force killing InfluxDB..."
    kill -9 "${INFLUXDB_PID}" 2>/dev/null || true
    sleep 1
    echo "✓ InfluxDB force stopped"
fi

# Clean up storage if requested
if [ "$CLEAN_STORAGE" = true ]; then
    if [ -d ".influxdb" ]; then
        echo "Cleaning up InfluxDB storage..."
        rm -rf .influxdb
        echo "✓ InfluxDB storage cleaned (removed .influxdb directory)"
    else
        echo "No .influxdb directory found"
    fi
else
    echo "Storage preserved at .influxdb/"
    echo "To clean storage, run: $0 --clean"
fi

