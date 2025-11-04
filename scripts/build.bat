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
    conan install .. --build=missing --output-folder=. -s build_type=%BUILD_TYPE% -s compiler.cppstd=20
    if errorlevel 1 (
        echo ERROR: Conan install failed
        exit /b 1
    )
)

REM Configure (pass any additional arguments)
REM Find Conan toolchain file
set CONAN_TOOLCHAIN=
if exist "generators\conan_toolchain.cmake" (
    set CONAN_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=generators\conan_toolchain.cmake
) else if exist "conan_toolchain.cmake" (
    set CONAN_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
) else if exist "build\generators\conan_toolchain.cmake" (
    set CONAN_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=build\generators\conan_toolchain.cmake
) else if exist "build\build\generators\conan_toolchain.cmake" (
    set CONAN_TOOLCHAIN=-DCMAKE_TOOLCHAIN_FILE=build\build\generators\conan_toolchain.cmake
)

set CMAKE_ARGS=%*
set CMAKE_ARGS=!CMAKE_ARGS:%BUILD_TYPE%=!
if not "!CMAKE_ARGS!"=="" (
    echo Additional CMake args: !CMAKE_ARGS!
    if not "!CONAN_TOOLCHAIN!"=="" (
        cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_STANDARD=20 !CONAN_TOOLCHAIN! !CMAKE_ARGS!
    ) else (
        cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_STANDARD=20 !CMAKE_ARGS!
    )
) else (
    if not "!CONAN_TOOLCHAIN!"=="" (
        cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_STANDARD=20 !CONAN_TOOLCHAIN!
    ) else (
        cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_STANDARD=20
    )
)
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: CMake build failed
    exit /b 1
)

echo Build complete! Binaries are in %BUILD_DIR%\bin\Release

