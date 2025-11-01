@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=%BUILD_DIR%
if "%BUILD_DIR%"=="" set BUILD_DIR=build

set BUILD_TYPE=%BUILD_TYPE%
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BIN_DIR=%BUILD_DIR%\bin\%BUILD_TYPE%
set BIN_DIR_ALT=%BUILD_DIR%\bin

echo Running tests...
echo Searching in: %BIN_DIR% and %BIN_DIR_ALT%

REM Add build directories to PATH so DLLs can be found
REM influx-c-rest.dll is needed by test-influx-c-rest and test-influxdb-cpp-auth
set PATH=%BIN_DIR%;%BIN_DIR_ALT%;%BUILD_DIR%;%BUILD_DIR%\Release;%BUILD_DIR%\bin\Release;%PATH%

REM Test executables
set TEST_FILES=test-influxdb-cpp-rest test-influx-c-rest test-influxdb-cpp-auth
set FOUND_TESTS=0
set FAILED_TESTS=0

for %%t in (%TEST_FILES%) do (
    set TEST_FOUND=0
    
    REM Try Release subdirectory first
    if exist "%BIN_DIR%\%%t.exe" (
        echo Running %%t from %BIN_DIR%...
        "%BIN_DIR%\%%t.exe" -d yes
        if !errorlevel! equ 0 (
            echo [OK] %%t passed
        ) else (
            echo [FAIL] %%t failed
            set /a FAILED_TESTS+=1
        )
        set TEST_FOUND=1
        set /a FOUND_TESTS+=1
    ) else (
        REM Try bin directory
        if exist "%BIN_DIR_ALT%\%%t.exe" (
            echo Running %%t from %BIN_DIR_ALT%...
            "%BIN_DIR_ALT%\%%t.exe" -d yes
            if !errorlevel! equ 0 (
                echo [OK] %%t passed
            ) else (
                echo [FAIL] %%t failed
                set /a FAILED_TESTS+=1
            )
            set TEST_FOUND=1
            set /a FOUND_TESTS+=1
        ) else (
            echo Warning: %%t.exe not found
        )
    )
)

echo.
if %FOUND_TESTS% equ 0 (
    echo No tests found! Make sure you've built the project first.
    exit /b 1
) else if %FAILED_TESTS% equ 0 (
    echo All tests passed!
    exit /b 0
) else (
    echo %FAILED_TESTS% test^(s^) failed.
    exit /b 1
)

