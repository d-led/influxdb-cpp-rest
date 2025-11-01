#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

echo "Starting InfluxDB with docker compose..."
# Try docker compose (V2) first, fallback to docker-compose (standalone)
if command -v docker > /dev/null 2>&1 && docker compose version > /dev/null 2>&1; then
    docker compose up -d
elif command -v docker-compose > /dev/null 2>&1; then
    docker-compose up -d
else
    echo "Error: Neither 'docker compose' nor 'docker-compose' is available"
    exit 1
fi

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

# Determine which compose command to use for the warning
if command -v docker > /dev/null 2>&1 && docker compose version > /dev/null 2>&1; then
    COMPOSE_CMD="docker compose"
elif command -v docker-compose > /dev/null 2>&1; then
    COMPOSE_CMD="docker-compose"
else
    COMPOSE_CMD="docker compose"
fi
echo "Warning: InfluxDB may not be fully ready yet. Check with: ${COMPOSE_CMD} ps"

