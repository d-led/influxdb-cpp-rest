#!/usr/bin/env bash
set -euo pipefail

# Script to publish a new version to Conan Center Index
# Usage: ./scripts/publish-conan-version.sh [VERSION] [--dry-run] [--skip-pr]
#
# Example: ./scripts/publish-conan-version.sh 1.0.0

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
VERSION="${1:-}"

if [ -z "${VERSION}" ]; then
    echo "Error: Version required"
    echo "Usage: $0 [VERSION] [--dry-run] [--skip-pr]"
    exit 1
fi

# Remove 'v' prefix if present
VERSION="${VERSION#v}"

# Validate version format (semver)
if [[ ! "${VERSION}" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)(-rc[0-9]+)?$ ]]; then
    echo "Error: Invalid version format. Expected MAJOR.MINOR.PATCH or MAJOR.MINOR.PATCH-rcN"
    exit 1
fi

DRY_RUN=false
SKIP_PR=false
for arg in "$@"; do
    case $arg in
        --dry-run)
            DRY_RUN=true
            ;;
        --skip-pr)
            SKIP_PR=true
            ;;
    esac
done

# Configuration (can be overridden by environment variables)
GITHUB_REPO="${GITHUB_REPO:-d-led/influxdb-cpp-rest}"
CONAN_INDEX_FORK="${CONAN_INDEX_FORK:-d-led/conan-center-index}"
CONAN_INDEX_UPSTREAM="${CONAN_INDEX_UPSTREAM:-conan-io/conan-center-index}"
CONAN_INDEX_DIR="${CONAN_INDEX_DIR:-${PROJECT_ROOT}/../conan-center-index}"
RECIPE_DIR="recipes/influxdb-cpp-rest/${VERSION}"
BRANCH_NAME="recipes/influxdb-cpp-rest/${VERSION}"

echo "Publishing influxdb-cpp-rest/${VERSION} to Conan Center"
echo "=================================================="

# Check if gh CLI is installed
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is required. Install from https://cli.github.com/"
    exit 1
fi

# Check if git is configured
if ! git config user.name > /dev/null 2>&1; then
    echo "Error: Git user.name not configured"
    exit 1
fi

# Setup or update conan-center-index repository
if [ ! -d "${CONAN_INDEX_DIR}" ]; then
    echo "Cloning conan-center-index fork..."
    if [ "${DRY_RUN}" = true ]; then
        echo "[DRY-RUN] Would clone https://github.com/${CONAN_INDEX_FORK}.git to ${CONAN_INDEX_DIR}"
    else
        # Use GITHUB_TOKEN if available (for CI), otherwise use default authentication
        if [ -n "${GITHUB_TOKEN:-}" ]; then
            git clone "https://${GITHUB_TOKEN}@github.com/${CONAN_INDEX_FORK}.git" "${CONAN_INDEX_DIR}"
        else
            git clone "https://github.com/${CONAN_INDEX_FORK}.git" "${CONAN_INDEX_DIR}"
        fi
        cd "${CONAN_INDEX_DIR}"
        git remote add upstream "https://github.com/${CONAN_INDEX_UPSTREAM}.git" 2>/dev/null || true
    fi
else
    echo "Updating conan-center-index repository..."
    cd "${CONAN_INDEX_DIR}"
    if [ "${DRY_RUN}" != true ]; then
        git fetch upstream master
        git checkout master 2>/dev/null || git checkout -b master
        git merge upstream/master || true
    fi
fi

cd "${CONAN_INDEX_DIR}"

# Check if version already exists
if [ -d "${RECIPE_DIR}" ]; then
    echo "Warning: Recipe for version ${VERSION} already exists!"
    read -p "Overwrite? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Check if tag exists
TAG="v${VERSION}"
if ! git ls-remote --tags "https://github.com/${GITHUB_REPO}.git" | grep -q "refs/tags/${TAG}"; then
    echo "Warning: Tag ${TAG} not found in ${GITHUB_REPO}"
    echo "Creating the tag first is recommended."
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Calculate SHA256 of source tarball
echo "Calculating SHA256 checksum..."
SOURCE_URL="https://github.com/${GITHUB_REPO}/archive/refs/tags/${TAG}.tar.gz"
if [ "${DRY_RUN}" = true ]; then
    SHA256="[would calculate from ${SOURCE_URL}]"
    echo "[DRY-RUN] SHA256: ${SHA256}"
else
    SHA256=$(curl -sL "${SOURCE_URL}" | shasum -a 256 | awk '{print $1}')
    echo "SHA256: ${SHA256}"
fi

# Create recipe directory
mkdir -p "${RECIPE_DIR}"

# Generate conanfile.py for conan-center-index
cat > "${RECIPE_DIR}/conanfile.py" <<EOF
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import get
import os

required_conan_version = ">=1.54.0"


class InfluxdbCppRestConan(ConanFile):
    name = "influxdb-cpp-rest"
    description = "A C++ client library for InfluxDB using C++ REST SDK"
    topics = ("influxdb", "cpprest", "http", "client")
    license = "MPL-2.0"
    homepage = "https://github.com/${GITHUB_REPO}"
    url = "https://github.com/conan-io/conan-center-index"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options["cpprestsdk"].shared = True

    def requirements(self):
        self.requires("cpprestsdk/2.10.19")
        self.requires("rxcpp/4.1.1")

    def build_requirements(self):
        # Only for tests - not linked to the library
        self.test_requires("catch2/3.11.0")

    def layout(self):
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        # Disable tests and demo for packaging
        tc.variables["BUILD_TESTING"] = False
        tc.variables["BUILD_DEMO"] = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "influxdb-cpp-rest")
        self.cpp_info.set_property("cmake_target_name", "influxdb-cpp-rest::influxdb-cpp-rest")
        
        # Libraries to link
        self.cpp_info.libs = ["influxdb-cpp-rest"]
        
        # Include directories
        self.cpp_info.includedirs = ["include"]
        
        # System dependencies (if any)
        if self.settings.os == "Linux":
            self.cpp_info.system_libs = ["pthread"]
EOF

# Generate conandata.yml
cat > "${RECIPE_DIR}/conandata.yml" <<EOF
sources:
  "${VERSION}":
    url: "${SOURCE_URL}"
    sha256: "${SHA256}"
EOF

echo "Created recipe files in ${RECIPE_DIR}/"

if [ "${DRY_RUN}" = true ]; then
    echo "[DRY-RUN] Would commit and push changes"
    echo "[DRY-RUN] Would create PR to ${CONAN_INDEX_UPSTREAM}"
    exit 0
fi

# Create branch and commit
echo "Creating branch and committing..."
git checkout -b "${BRANCH_NAME}" 2>/dev/null || git checkout "${BRANCH_NAME}"
git add "${RECIPE_DIR}/"
git commit -m "Add influxdb-cpp-rest/${VERSION}

- C++20 client library for InfluxDB
- Includes C++ static library and C wrapper shared library
- Supports InfluxDB 1.x
- Requires cpprestsdk and rxcpp dependencies"

# Push branch
echo "Pushing branch..."
if [ -n "${GITHUB_TOKEN:-}" ]; then
    # Use token for CI environment
    git remote set-url origin "https://${GITHUB_TOKEN}@github.com/${CONAN_INDEX_FORK}.git"
fi
git push -u origin "${BRANCH_NAME}" || git push origin "${BRANCH_NAME}"

# Create pull request using GitHub REST API (works without gh CLI)
if [ "${SKIP_PR}" != true ]; then
    echo "Creating pull request..."
    
    if [ -n "${GITHUB_TOKEN:-}" ]; then
        # Use GitHub REST API to create PR
        PR_BODY=$(cat <<EOF
This PR adds the influxdb-cpp-rest package version ${VERSION} to Conan Center.

## Package Details
- **Name**: influxdb-cpp-rest
- **Version**: ${VERSION}
- **License**: MPL-2.0
- **Homepage**: https://github.com/${GITHUB_REPO}

## Features
- C++20 client library for InfluxDB
- Includes both a C++ static library (influxdb-cpp-rest) and a C wrapper shared library (influx-c-rest)
- Supports InfluxDB 1.x (tested with v1.8+)
- Synchronous and asynchronous insertion APIs
- Query API support
- Basic authentication support

## Dependencies
- cpprestsdk/2.10.19
- rxcpp/4.1.1
- catch2/3.11.0 (test requirement only)

## Compiler Requirements
- C++20 or later
- GCC 11+, Clang 14+, MSVC 2019+

Automatically created by GitHub Actions workflow on tag push.
EOF
)
        
        # Extract owner and repo from CONAN_INDEX_UPSTREAM (format: owner/repo)
        # Format PR body as JSON (escape newlines and quotes)
        ESCAPED_BODY=$(echo "${PR_BODY}" | sed ':a;N;$!ba;s/\n/\\n/g' | sed 's/"/\\"/g')
        
        # Extract fork owner from CONAN_INDEX_FORK (format: owner/repo)
        FORK_OWNER="${CONAN_INDEX_FORK%%/*}"
        
        PR_RESPONSE=$(curl -s -w "\n%{http_code}" \
            -X POST \
            -H "Accept: application/vnd.github.v3+json" \
            -H "Authorization: token ${GITHUB_TOKEN}" \
            "https://api.github.com/repos/${CONAN_INDEX_UPSTREAM}/pulls" \
            -d "{
                \"title\": \"Add influxdb-cpp-rest/${VERSION}\",
                \"body\": \"${ESCAPED_BODY}\",
                \"head\": \"${FORK_OWNER}:${BRANCH_NAME}\",
                \"base\": \"master\"
            }")
        
        HTTP_CODE=$(echo "${PR_RESPONSE}" | tail -1)
        PR_JSON=$(echo "${PR_RESPONSE}" | head -n -1)
        
        if [ "${HTTP_CODE}" = "201" ] || [ "${HTTP_CODE}" = "200" ]; then
            PR_URL=$(echo "${PR_JSON}" | grep -o '"html_url":[^,]*' | cut -d'"' -f4 || echo "")
            echo ""
            echo "✅ Pull request created: ${PR_URL}"
            echo ""
            echo "Next steps:"
            echo "  1. Monitor the PR for CI results"
            echo "  2. Address any review comments or CI failures"
            echo "  3. Once merged, the package will be available on Conan Center"
        else
            echo "Warning: Failed to create PR (HTTP ${HTTP_CODE})"
            echo "Response: ${PR_JSON}"
            echo "Branch pushed to: ${CONAN_INDEX_FORK}:${BRANCH_NAME}"
            echo "Create PR manually at: https://github.com/${CONAN_INDEX_UPSTREAM}/compare/master...${CONAN_INDEX_FORK#*/}:${BRANCH_NAME}"
            exit 1
        fi
    else
        echo "✅ Branch pushed. Create PR manually:"
        echo "  https://github.com/${CONAN_INDEX_UPSTREAM}/compare/master...${CONAN_INDEX_FORK#*/}:${BRANCH_NAME}"
    fi
else
    echo "✅ Branch pushed. Create PR manually:"
    echo "  https://github.com/${CONAN_INDEX_UPSTREAM}/compare/master...${CONAN_INDEX_FORK#*/}:${BRANCH_NAME}"
fi

