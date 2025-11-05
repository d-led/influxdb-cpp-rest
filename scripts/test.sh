#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

# Build before testing to ensure we're using the latest code
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${PROJECT_ROOT}"

echo "Building project (${BUILD_TYPE})..."
# First configure (if needed)
"${SCRIPT_DIR}/build.sh" >/dev/null 2>&1 || true

# Then build the test targets - this will show compilation errors
echo "Compiling test targets..."
cd "${BUILD_DIR}"
if ! make test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth -j4 2>&1; then
    echo "" >&2
    echo "Build failed! Fix compilation errors above before running tests." >&2
    exit 1
fi
cd "${PROJECT_ROOT}"

# Determine possible binary directories
# Prefer BUILD_TYPE-specific directories first (they contain the most recent builds)
BIN_DIRS=(
    "${BUILD_DIR}/bin/${BUILD_TYPE}"
    "${BUILD_DIR}/${BUILD_TYPE}/bin"
    "${BUILD_DIR}/bin"
    "${BUILD_DIR}"
)

# Set library path
export LD_LIBRARY_PATH="${BUILD_DIR}:${LD_LIBRARY_PATH:-}"
export DYLD_LIBRARY_PATH="${BUILD_DIR}:${DYLD_LIBRARY_PATH:-}"

# Also add build directory to PATH for shared libraries
export PATH="${BUILD_DIR}:${PATH}"

echo "Running tests..."
echo "Searching in: ${BIN_DIRS[*]}"

# Test executables
TEST_FILES=(
    "test-influxdb-cpp-rest"
    "test-influx-c-rest"
    "test-influxdb-cpp-auth"
)

FAILED_TESTS=0
FOUND_TESTS=0

for test in "${TEST_FILES[@]}"; do
    TEST_FOUND=0
    
    # Search in all possible locations
    for bin_dir in "${BIN_DIRS[@]}"; do
        if [ -f "${bin_dir}/${test}" ] && [ -x "${bin_dir}/${test}" ]; then
            echo "Running ${test} from ${bin_dir}..."
            if "${bin_dir}/${test}" -d yes; then
                echo "✓ ${test} passed"
            else
                echo "✗ ${test} failed"
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
            TEST_FOUND=1
            FOUND_TESTS=$((FOUND_TESTS + 1))
            break
        fi
    done
    
    if [ $TEST_FOUND -eq 0 ]; then
        echo "Warning: ${test} not found (searched in: ${BIN_DIRS[*]})"
    fi
done

echo ""
if [ $FOUND_TESTS -eq 0 ]; then
    echo "No tests found! Make sure you've built the project first."
    exit 1
elif [ $FAILED_TESTS -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "${FAILED_TESTS} test(s) failed."
    exit 1
fi

