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
- **rxcpp/4.1.1** - Reactive Extensions for C++
- **catch2/3.11.0** - Testing framework (test-only)

## Publishing to Conan Center

The package is published to [Conan Center](https://conan.io/center) via pull requests to the [conan-center-index](https://github.com/conan-io/conan-center-index) repository. The recipe lives at `recipes/influxdb-cpp-rest/` and follows the "all" recipe convention: a single recipe in `all/` serves every version, and `config.yml` maps versions to that folder.

### One-time setup

1. Fork https://github.com/conan-io/conan-center-index
2. Clone your fork and add the upstream remote:
   ```bash
   git remote add upstream https://github.com/conan-io/conan-center-index.git
   ```

### Publishing a new version

1. **Create and push a Git tag** (e.g. `v1.0.4`):
   ```bash
   ./scripts/tag-version.sh patch  # or major / minor / rc / release
   ./scripts/push-latest-version.sh
   ```

   Pushing a `v*` tag also creates a GitHub release automatically (`.github/workflows/release.yml`).

2. **Open the recipe in your conan-center-index fork**:
   ```bash
   cd ../conan-center-index
   git checkout master
   git pull upstream master
   git checkout -b recipes/influxdb-cpp-rest/1.0.4
   ```

3. **Add the new version to two files** under `recipes/influxdb-cpp-rest/`:
   - `config.yml` — map `"1.0.4"` to `folder: all`
   - `all/conandata.yml` — add the source URL and SHA256 of the tag tarball

   Compute the checksum with:
   ```bash
   curl -sL https://github.com/d-led/influxdb-cpp-rest/archive/refs/tags/v1.0.4.tar.gz | shasum -a 256
   ```

4. **Test the recipe locally**:
   ```bash
   cd recipes/influxdb-cpp-rest/all
   conan create . --version=1.0.4
   ```

5. **Commit, push and open a PR**:
   ```bash
   git add recipes/influxdb-cpp-rest
   git commit -m "influxdb-cpp-rest: add version 1.0.4"
   git push origin recipes/influxdb-cpp-rest/1.0.4
   gh pr create --repo conan-io/conan-center-index --title "Add influxdb-cpp-rest/1.0.4"
   ```

6. **Address review comments and CI failures** until the maintainers merge it.

### Recipe conventions

- No `version` field in `conanfile.py` — the version comes from `config.yml` / `conandata.yml`.
- `source()` downloads the tag tarball; its SHA256 lives in `conandata.yml`.
- No `exports_sources` — the source is downloaded, not exported.
- The license file is copied into `licenses/` in `package()`.

### Conan Recipe Checklist

- [ ] Recipe follows Conan Center conventions
- [ ] All dependencies are declared in `requirements()`
- [ ] Test package included
- [ ] License file packaged
- [ ] Compatible with major compilers (GCC, Clang, MSVC)
- [ ] Works on Linux, macOS, Windows

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

