#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

# Build if not already built
if [ ! -f "${BUILD_DIR}/bin/demo" ] && [ ! -f "${BUILD_DIR}/bin/${BUILD_TYPE}/demo" ] && [ ! -f "${BUILD_DIR}/${BUILD_TYPE}/bin/demo" ]; then
    echo "Building demo..."
    ./scripts/build.sh "${BUILD_TYPE}"
fi

# Find and run the demo
for demo_path in "${BUILD_DIR}/bin/demo" "${BUILD_DIR}/bin/${BUILD_TYPE}/demo" "${BUILD_DIR}/${BUILD_TYPE}/bin/demo" "${BUILD_DIR}/demo"; do
    if [ -f "${demo_path}" ] && [ -x "${demo_path}" ]; then
        echo "Running ${demo_path}..."
        "${demo_path}"
        exit 0
    fi
done

echo "ERROR: Demo executable not found. Build it first with: ./scripts/build.sh"
exit 1

