@echo off
setlocal enabledelayedexpansion

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BUILD_DIR=%BUILD_DIR%
if "%BUILD_DIR%"=="" set BUILD_DIR=build

echo Building with CMake (%BUILD_TYPE%)...

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM Install dependencies with Conan if not already done
if not exist "conan_toolchain.cmake" if not exist "generators\conan_toolchain.cmake" (
    echo Installing dependencies with Conan...
    conan install .. --build=missing --output-folder=. -s build_type=%BUILD_TYPE%
)

REM Configure
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_STANDARD=20

REM Build
cmake --build . --config %BUILD_TYPE%

echo Build complete! Binaries are in %BUILD_DIR%\bin\Release

