@echo off
setlocal enabledelayedexpansion

set INFLUXDB_VERSION=1.8.10
set INFLUXDB_DIR=influxdb-%INFLUXDB_VERSION%-1
set INFLUXDB_ZIP=influxdb-%INFLUXDB_VERSION%_windows_amd64.zip
set INFLUXDB_URL=https://dl.influxdata.com/influxdb/releases/%INFLUXDB_ZIP%

cd /d %~dp0\..

echo Checking if InfluxDB binary already exists...
if exist "%INFLUXDB_DIR%\influxd.exe" (
    echo InfluxDB binary found.
    goto :start_influxdb
)

echo Downloading InfluxDB %INFLUXDB_VERSION%...
if exist "%INFLUXDB_ZIP%" (
    echo Archive already downloaded.
) else (
    curl -L -o "%INFLUXDB_ZIP%" "%INFLUXDB_URL%"
    if errorlevel 1 (
        echo ERROR: Failed to download InfluxDB from %INFLUXDB_URL%
        exit /b 1
    )
)

echo Extracting InfluxDB...
powershell -Command "Expand-Archive -Path '%INFLUXDB_ZIP%' -DestinationPath '.' -Force"
if errorlevel 1 (
    echo ERROR: Failed to extract InfluxDB
    exit /b 1
)

:start_influxdb
if not exist "%INFLUXDB_DIR%\influxd.exe" (
    echo ERROR: influxd.exe not found in %INFLUXDB_DIR%
    exit /b 1
)

echo Starting InfluxDB...
start /B "" "%INFLUXDB_DIR%\influxd.exe" -config influxdb.conf

REM Wait a moment for InfluxDB to start
ping 127.0.0.1 -n 4 >nul

REM Verify the process is running
tasklist /FI "IMAGENAME eq influxd.exe" 2>nul | find /I "influxd.exe" >nul
if errorlevel 1 (
    echo ERROR: InfluxDB process is not running
    exit /b 1
)

echo Waiting for InfluxDB to be ready...
REM Poll with exponential backoff: start with short waits, increase gradually
set /a max_attempts=60
set /a attempt=0
set /a wait_seconds=1
:health_check
REM Check if InfluxDB responds with HTTP 204 (No Content) which indicates readiness
powershell -Command "try { $r = Invoke-WebRequest -Uri 'http://localhost:8086/ping' -TimeoutSec 2 -UseBasicParsing -ErrorAction Stop; if ($r.StatusCode -eq 204) { exit 0 } else { exit 1 } } catch { exit 1 }" >nul 2>&1
if errorlevel 1 (
    set /a attempt+=1
    if !attempt! lss %max_attempts% (
        REM Verify process is still running before continuing
        tasklist /FI "IMAGENAME eq influxd.exe" 2>nul | find /I "influxd.exe" >nul
        if errorlevel 1 (
            echo ERROR: InfluxDB process died unexpectedly
            exit /b 1
        )
        echo Attempt !attempt!/%max_attempts%: InfluxDB not ready yet, waiting !wait_seconds! second^(s^)...
        ping 127.0.0.1 -n !wait_seconds! >nul
        REM Exponential backoff: cap at 5 seconds
        set /a wait_seconds+=1
        if !wait_seconds! gtr 5 set /a wait_seconds=5
        goto :health_check
    )
    echo ERROR: InfluxDB failed to become ready after %max_attempts% attempts
    exit /b 1
) else (
    echo ✓ InfluxDB is ready after !attempt! polling attempts!
)
