@echo off
setlocal enabledelayedexpansion

set BUILD_DIR=%BUILD_DIR%
if "%BUILD_DIR%"=="" set BUILD_DIR=build

set BUILD_TYPE=%BUILD_TYPE%
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set BIN_DIR=%BUILD_DIR%\bin\%BUILD_TYPE%
set BIN_DIR_ALT=%BUILD_DIR%\bin

REM Build if not already built
if not exist "%BIN_DIR%\demo.exe" if not exist "%BIN_DIR_ALT%\demo.exe" (
    echo Building demo...
    scripts\build.bat %BUILD_TYPE%
)

REM Find and run the demo
if exist "%BIN_DIR%\demo.exe" (
    echo Running %BIN_DIR%\demo.exe...
    "%BIN_DIR%\demo.exe"
    exit /b %errorlevel%
)

if exist "%BIN_DIR_ALT%\demo.exe" (
    echo Running %BIN_DIR_ALT%\demo.exe...
    "%BIN_DIR_ALT%\demo.exe"
    exit /b %errorlevel%
)

echo ERROR: Demo executable not found. Build it first with: scripts\build.bat
exit /b 1

