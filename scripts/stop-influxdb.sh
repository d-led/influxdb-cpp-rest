#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

echo "Stopping InfluxDB..."
# Try docker compose (V2) first, fallback to docker-compose (standalone)
if command -v docker > /dev/null 2>&1 && docker compose version > /dev/null 2>&1; then
    docker compose down
elif command -v docker-compose > /dev/null 2>&1; then
    docker-compose down
else
    echo "Error: Neither 'docker compose' nor 'docker-compose' is available"
    exit 1
fi

echo "InfluxDB stopped."

