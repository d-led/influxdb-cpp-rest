#!/usr/bin/env bash
set -euo pipefail

# Test the Conan Center Index recipe for influxdb-cpp-rest locally.
# Assumes the conan-center-index fork is cloned next to this repository
# (i.e. ../conan-center-index), or set CONAN_INDEX_DIR.
#
# Usage:
#   ./scripts/test-conan-recipe.sh <VERSION> [extra conan create args]
# Example:
#   ./scripts/test-conan-recipe.sh 1.0.3 -b missing

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

VERSION="${1:-}"
if [ -z "$VERSION" ]; then
    echo "Usage: $0 <VERSION> [extra conan create args]" >&2
    echo "Example: $0 1.0.3 -b missing" >&2
    exit 1
fi
shift

CONAN_INDEX_DIR="${CONAN_INDEX_DIR:-${PROJECT_ROOT}/../conan-center-index}"
RECIPE_DIR="${CONAN_INDEX_DIR}/recipes/influxdb-cpp-rest/all"

if [ ! -f "${RECIPE_DIR}/conanfile.py" ]; then
    echo "Error: recipe not found at ${RECIPE_DIR}" >&2
    echo "Clone your conan-center-index fork next to this repository or set CONAN_INDEX_DIR." >&2
    exit 1
fi

echo "Testing recipe at ${RECIPE_DIR} for version ${VERSION}..."
cd "${RECIPE_DIR}"
rm -rf test_package/build
conan create . --version="${VERSION}" "$@"
