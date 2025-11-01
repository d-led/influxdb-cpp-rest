@echo off
setlocal

cd /d %~dp0\..

echo Stopping InfluxDB...
docker compose -f docker-compose.win.yml down
if errorlevel 1 (
    echo WARNING: Failed to stop InfluxDB properly
    exit /b 1
)

echo InfluxDB stopped.

