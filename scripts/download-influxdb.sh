#!/usr/bin/env bash
# Download and extract InfluxDB binary
# Usage: download-influxdb.sh <version> <architecture>
# Example: download-influxdb.sh 1.8.10 darwin_amd64

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${PROJECT_ROOT}"

INFLUXDB_VERSION="${1:-1.8.10}"
ARCH="${2:-darwin_amd64}"

# Determine file extension based on architecture
if [[ "${ARCH}" == *"windows"* ]]; then
    FILE_EXT=".zip"
    EXTRACT_CMD="unzip -q"
else
    FILE_EXT=".tar.gz"
    EXTRACT_CMD="tar -xzf"
fi

INFLUXDB_ARCHIVE="influxdb-${INFLUXDB_VERSION}_${ARCH}${FILE_EXT}"
INFLUXDB_URL="https://dl.influxdata.com/influxdb/releases/${INFLUXDB_ARCHIVE}"
INFLUXDB_DIR="influxdb-${INFLUXDB_VERSION}-1"

# Determine binary path based on platform
if [[ "${ARCH}" == *"windows"* ]]; then
    BINARY_PATH="${INFLUXDB_DIR}/influxd.exe"
else
    BINARY_PATH="${INFLUXDB_DIR}/usr/bin/influxd"
fi

# Check if binary already exists
if [ -f "${BINARY_PATH}" ]; then
    echo "✓ InfluxDB binary already exists at ${BINARY_PATH}"
    chmod +x "${BINARY_PATH}" 2>/dev/null || true
    exit 0
fi

# Check if archive exists
if [ -f "${INFLUXDB_ARCHIVE}" ]; then
    echo "Archive ${INFLUXDB_ARCHIVE} already exists, extracting..."
else
    echo "Downloading InfluxDB ${INFLUXDB_VERSION} for ${ARCH}..."
    curl -L -o "${INFLUXDB_ARCHIVE}" "${INFLUXDB_URL}"
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to download InfluxDB from ${INFLUXDB_URL}"
        exit 1
    fi
fi

echo "Extracting InfluxDB..."
${EXTRACT_CMD} "${INFLUXDB_ARCHIVE}"
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to extract ${INFLUXDB_ARCHIVE}"
    exit 1
fi

echo "Verifying InfluxDB binary..."
if [ ! -f "${BINARY_PATH}" ]; then
    echo "ERROR: InfluxDB binary not found at expected path: ${BINARY_PATH}"
    echo "Extracted contents:"
    ls -la "${INFLUXDB_DIR}" || true
    exit 1
fi

ls -la "${BINARY_PATH}"
chmod +x "${BINARY_PATH}" 2>/dev/null || true
echo "✓ InfluxDB ${INFLUXDB_VERSION} downloaded and extracted successfully"

