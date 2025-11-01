#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

echo "Starting InfluxDB with docker compose..."
docker compose up -d

echo "Waiting for InfluxDB to be ready..."
timeout=30
elapsed=0
while [ $elapsed -lt $timeout ]; do
    if curl -s http://localhost:8086/ping > /dev/null 2>&1 || wget --spider -q http://localhost:8086/ping 2>/dev/null; then
        echo "InfluxDB is ready!"
        exit 0
    fi
    sleep 1
    elapsed=$((elapsed + 1))
done

echo "Warning: InfluxDB may not be fully ready yet. Check with: docker compose ps"

