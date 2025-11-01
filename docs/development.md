# Development Guide

## Prerequisites

- CMake 3.20 or later
- C++20 compatible compiler (GCC 11+, Clang 14+, MSVC 2019+)
- Conan 2.0+
- Python 3.x (for Conan)
- Docker (for running InfluxDB tests locally)

## Building the Library

### 1. Install Conan

```bash
pip install conan
```

### 2. Build the Library

**Using scripts (recommended):**

```bash
# Linux/macOS
./scripts/build.sh Release    # or Debug

# Windows
scripts\build.bat Release      # or Debug
```

**Manual build:**

```bash
mkdir build && cd build
conan install .. --build=missing --output-folder=.
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
cmake --build . --config Release
```

This installs dependencies:
- cpprestsdk (HTTP client library)
- catch2 (testing framework)

## Project Structure

- `src/influxdb-cpp-rest/` - Core C++ library (static library)
- `src/influx-c-rest/` - C wrapper API (shared library)
- `src/demo/` - Example application
- `src/test/` - C++ unit tests
- `src/test-shared/` - C API unit tests
- `src/auth_test/` - Authentication tests

## Running Tests

### Start InfluxDB

**Using scripts (recommended):**

```bash
# Linux/macOS
./scripts/start-influxdb.sh

# Windows
scripts\start-influxdb.bat
```

**Using docker compose directly:**

```bash
docker compose up -d              # Linux/macOS
docker compose -f docker-compose.win.yml up -d   # Windows
```

This starts InfluxDB with:
- Database: `testdb`
- Admin user: `admin` / `admin123`
- Test user: `testuser` / `testpass`
- Port: `8086`

### Run Tests

**Using scripts:**

```bash
# Linux/macOS
./scripts/test.sh

# Windows
scripts\test.bat
```

**Manual test execution:**

```bash
cd build
# Linux/macOS
export LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH
./bin/test-influxdb-cpp-rest -d yes
./bin/test-influx-c-rest -d yes
./bin/test-influxdb-cpp-auth -d yes
```

Windows:
```cmd
set PATH=%CD%\bin\Release;%PATH%
bin\Release\test-influxdb-cpp-rest.exe -d yes
bin\Release\test-influx-c-rest.exe -d yes
bin\Release\test-influxdb-cpp-auth.exe -d yes
```

### Stop InfluxDB

```bash
# Linux/macOS
./scripts/stop-influxdb.sh

# Windows
scripts\stop-influxdb.bat
```

Or directly:
```bash
docker compose down              # Linux/macOS
docker compose -f docker-compose.win.yml down   # Windows
```

## Build Options

- `BUILD_TESTING` (default: ON) - Build test executables
- `BUILD_DEMO` (default: ON) - Build demo application
- `USE_CONAN` (default: ON) - Use Conan for dependency management

Example:
```bash
cmake .. -DBUILD_TESTING=OFF -DBUILD_DEMO=OFF
```

## Code Style

- Follow existing code style
- C++20 standard
- Use modern C++ features (smart pointers, ranges, concepts where applicable)

## Dependencies

### Managed via Conan

- **cpprestsdk/2.10.19** - HTTP client library
- **catch2/3.5.3** - Testing framework
- **fmt/11.1.4** - Formatting library (C++20 compatible)

### Bundled (header-only or source)

- **fmt** - Formatting library (in `deps/fmt/`)
- **RxCpp** - Reactive Extensions (in `deps/rxcpp/`, header-only)

These bundled dependencies may be migrated to Conan in the future.

## Publishing to Conan Center

The package is published to [Conan Center](https://conan.io/center) via pull requests to the [conan-center-index](https://github.com/conan-io/conan-center-index) repository. The recipe is maintained at `recipes/influxdb-cpp-rest/` in that repository.

### Initial Setup (One-time)

1. Fork https://github.com/conan-io/conan-center-index
2. Clone your fork locally
3. Set upstream remote: `git remote add upstream https://github.com/conan-io/conan-center-index.git`
4. **Create a GitHub Personal Access Token (PAT)** with `repo` scope:
   - Go to https://github.com/settings/tokens
   - Generate a new token (classic) with `repo` scope
   - Add it as a repository secret named `CONAN_INDEX_PAT` in your influxdb-cpp-rest repository
     - Go to Settings → Secrets and variables → Actions → New repository secret
     - Name: `CONAN_INDEX_PAT`
     - Value: your PAT token

   **Note:** The workflow will use `GITHUB_TOKEN` if `CONAN_INDEX_PAT` is not set, but `GITHUB_TOKEN` cannot push to forks. A PAT is required for automated publishing.

### Publishing a New Version

Use the automated script (recommended):

```bash
./scripts/publish-conan-version.sh 1.0.0
```

Or manually:

1. **Create a GitHub release/tag** for the version (if not already exists)
2. **Navigate to conan-center-index repository**:
   ```bash
   cd ../conan-center-index  # or wherever you cloned it
   git checkout master
   git pull upstream master
   ```
3. **Create version branch**:
   ```bash
   git checkout -b recipes/influxdb-cpp-rest/X.Y.Z
   ```
4. **Create recipe directory and files**:
   ```bash
   mkdir -p recipes/influxdb-cpp-rest/X.Y.Z
   # Copy conanfile.py template and adapt for conan-center-index format
   # Create conandata.yml with source URL and SHA256
   ```
5. **Commit and push**:
   ```bash
   git add recipes/influxdb-cpp-rest/X.Y.Z/
   git commit -m "Add influxdb-cpp-rest/X.Y.Z"
   git push origin recipes/influxdb-cpp-rest/X.Y.Z
   ```
6. **Create pull request**:
   ```bash
   gh pr create --repo conan-io/conan-center-index --title "Add influxdb-cpp-rest/X.Y.Z"
   ```

See [Publishing New Versions](#publishing-new-versions) section below for details.

## Publishing New Versions

When releasing a new version of influxdb-cpp-rest:

1. **Create and push a Git tag** (e.g., `v1.0.0`):
   ```bash
   ./scripts/tag-version.sh patch  # or major/minor
   git push --tags
   ```

2. **Automatic publishing**: Pushing a tag starting with `v` (e.g., `v1.0.0`) automatically triggers the GitHub Actions workflow that:
   - Calculates the SHA256 hash of the source tarball
   - Creates the recipe files in the correct format
   - Commits and pushes to your conan-center-index fork
   - Creates a pull request to conan-center-index

3. **Monitor the PR**: The Conan Center CI will automatically test the recipe. Address any review comments or CI failures.

### Manual Publishing (Alternative)

If you need to publish manually or the automated workflow fails:

```bash
./scripts/publish-conan-version.sh 1.0.0
```

This script will:
- Clone/update your conan-center-index fork if needed
- Calculate the SHA256 hash of the source tarball
- Create the recipe files in the correct format
- Commit and push the changes
- Create a pull request to conan-center-index

### Recipe Format for Conan Center

The recipe in conan-center-index differs from the local `conanfile.py`:

- **No `version` field** - version comes from directory path
- **`source()` method** - downloads from GitHub release/tag
- **`conandata.yml`** - contains source URL and SHA256 checksum
- **No `exports_sources`** - source is downloaded, not exported

The automation script handles these differences automatically.

### Conan Recipe Checklist

- [ ] Recipe follows Conan Center conventions
- [ ] All dependencies are declared in `requirements()`
- [ ] Test package included (optional but recommended)
- [ ] License file included
- [ ] Source code properly exported
- [ ] Compatible with major compilers (GCC, Clang, MSVC)
- [ ] Works on Linux, macOS, Windows

### Testing Locally

Before publishing, test the recipe locally:

```bash
# In your conan-center-index fork
cd recipes/influxdb-cpp-rest/1.0.0
conan create . influxdb-cpp-rest/1.0.0@
```

### Conan Center Requirements

- Recipe must be in conan-center-index repository
- Must pass automated CI checks
- All dependencies must also be in Conan Center
- License must be compatible with Conan Center
- Source code must be publicly accessible
- GitHub tag/release must exist for the version

### Versioning

Follow semantic versioning (semver):
- MAJOR.MINOR.PATCH (e.g., 1.2.3)
- Release candidates: MAJOR.MINOR.PATCH-rcN (e.g., 1.2.3-rc1)

**Using the version tagging script:**

```bash
# Preview changes without applying (dry-run)
./scripts/tag-version.sh major --dry-run

# Bump major version (e.g., 1.2.3 -> 2.0.0)
./scripts/tag-version.sh major

# Bump minor version (e.g., 1.2.3 -> 1.3.0)
./scripts/tag-version.sh minor

# Bump patch version (e.g., 1.2.3 -> 1.2.4)
./scripts/tag-version.sh patch

# Create/bump release candidate (e.g., 1.2.3 -> 1.2.3-rc1 or 1.2.3-rc1 -> 1.2.3-rc2)
./scripts/tag-version.sh rc

# Bump and create RC in one command (e.g., 1.2.3 -> 2.0.0-rc1)
./scripts/tag-version.sh major rc
./scripts/tag-version.sh minor rc
./scripts/tag-version.sh patch rc

# Release (remove -rc suffix, e.g., 1.2.3-rc2 -> 1.2.3)
./scripts/tag-version.sh release
```

The script:
- Updates version in `conanfile.py` and `CMakeLists.txt`
- Creates a git tag `v{version}`
- Uses current version from conanfile.py or latest git tag
- Supports `--dry-run` flag to preview changes without applying them

**Pushing version tags:**

```bash
# Preview what would be pushed (dry-run)
./scripts/push-latest-version.sh --dry-run

# Push latest version tag to origin
./scripts/push-latest-version.sh

# Push to specific remote
./scripts/push-latest-version.sh upstream

# Dry-run with specific remote
./scripts/push-latest-version.sh upstream --dry-run
```

