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

**Using docker-compose directly:**

```bash
docker-compose up -d
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
docker-compose down
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

### Prerequisites

1. Conan 2.0+
2. Conan account at https://conan.io/
3. GitHub repository (public)

### Publishing Steps

1. **Update version in conanfile.py**

   Update the `version` field in `conanfile.py`:
   ```python
   version = "1.0.0"  # or your new version
   ```

2. **Create Conan recipe (if not exists)**

   The `conanfile.py` should export all necessary files and define requirements.

3. **Test the recipe locally**

   ```bash
   conan create . --version 1.0.0
   ```

   Test with different configurations:
   ```bash
   conan create . --version 1.0.0 -s compiler=gcc -s compiler.version=11
   conan create . --version 1.0.0 -s compiler=clang -s compiler.version=14
   ```

4. **Export recipe**

   ```bash
   conan export . --version 1.0.0
   ```

5. **Upload to Conan Center**

   First, create a pull request to Conan Center Index:
   
   - Fork https://github.com/conan-io/conan-center-index
   - Create branch: `recipes/influxdb-cpp-rest/x.y.z` (where x.y.z is version)
   - Copy your `conanfile.py` to `recipes/influxdb-cpp-rest/x.y.z/conanfile.py`
   - Add recipe files to the index repository
   - Open a pull request

   The Conan Center team will review and merge your recipe.

6. **Alternative: Upload to your own remote**

   If you want to publish to your own Conan remote:
   
   ```bash
   # Add your remote
   conan remote add myremote https://your-conan-server.com
   
   # Upload
   conan upload influxdb-cpp-rest/1.0.0 -r myremote --all
   ```

### Conan Recipe Checklist

- [ ] Recipe follows Conan Center conventions
- [ ] All dependencies are declared in `requirements()`
- [ ] Test package included (optional but recommended)
- [ ] License file included
- [ ] Source code properly exported
- [ ] Compatible with major compilers (GCC, Clang, MSVC)
- [ ] Works on Linux, macOS, Windows

### Conan Center Requirements

- Recipe must be in conan-center-index repository
- Must pass automated CI checks
- All dependencies must also be in Conan Center
- License must be compatible with Conan Center
- Source code must be publicly accessible

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

