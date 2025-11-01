@echo off
setlocal

cd /d %~dp0\..

echo Stopping InfluxDB...

REM Kill influxd.exe process
taskkill /F /IM influxd.exe >nul 2>&1

REM Wait a bit for the process to fully terminate
timeout /t 2 /nobreak >nul

echo InfluxDB stopped.
