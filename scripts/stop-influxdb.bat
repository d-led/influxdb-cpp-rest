@echo off
setlocal

cd /d %~dp0\..

echo Stopping InfluxDB...
docker-compose down

echo InfluxDB stopped.

