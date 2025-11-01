@echo off
setlocal

cd /d %~dp0\..

echo Starting InfluxDB with docker-compose...
docker-compose up -d

echo Waiting for InfluxDB to be ready...
timeout /t 5 /nobreak >nul

echo InfluxDB started. Check status with: docker-compose ps

