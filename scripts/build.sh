#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"

echo "Building with CMake (${BUILD_TYPE})..."

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Install dependencies with Conan if not already done
if [ ! -f "conan_toolchain.cmake" ] && [ ! -f "generators/conan_toolchain.cmake" ]; then
    echo "Installing dependencies with Conan..."
    conan install .. --build=missing --output-folder=. -s build_type="${BUILD_TYPE}" -s compiler.cppstd=20
    
    # Verify toolchain was created
    if [ -f "generators/conan_toolchain.cmake" ]; then
        echo "✓ Conan toolchain found at: generators/conan_toolchain.cmake"
    elif [ -f "conan_toolchain.cmake" ]; then
        echo "✓ Conan toolchain found at: conan_toolchain.cmake"
    else
        echo "⚠ Warning: Conan toolchain not found where expected"
        echo "Searching for conan_toolchain.cmake:"
        find . -name "conan_toolchain.cmake" 2>/dev/null | head -5
    fi
fi

# Configure
cmake .. -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DCMAKE_CXX_STANDARD=20

# Build
cmake --build . --config "${BUILD_TYPE}"

echo "Build complete! Binaries are in ${BUILD_DIR}/bin"

