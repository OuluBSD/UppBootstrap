@echo off
REM ============================================================================
REM UppBootstrap: Windows CMD Batch Entrypoint
REM ============================================================================

setlocal EnableDelayedExpansion

if exist "C:\Program Files\7-Zip" set "PATH=%PATH%;C:\Program Files\7-Zip"

set SCRIPT_DIR=%~dp0
set BIN_DIR=%SCRIPT_DIR%bin
set SOURCE=%SCRIPT_DIR%src\main.cpp
set OUTPUT=%BIN_DIR%\upp-bootstrap.exe

if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"

where cl.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo ==> UppBootstrap: Compiling with MSVC cl.exe...
    cl /nologo /EHsc /std:c++17 /O2 /Fe:"%OUTPUT%" "%SOURCE%"
    goto run
)

where clang++ >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo ==> UppBootstrap: Compiling with clang++...
    clang++ -std=c++17 -O2 -o "%OUTPUT%" "%SOURCE%"
    goto run
)

where g++ >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo ==> UppBootstrap: Compiling with g++...
    g++ -std=c++17 -O2 -o "%OUTPUT%" "%SOURCE%"
    goto run
)

echo Error: Neither cl.exe, clang++, nor g++ found in PATH.
echo Launch this script from a Visual Studio Developer Command Prompt or install MinGW.
exit /b 1

:run
echo ==> UppBootstrap: Executing bootstrap...
"%OUTPUT%" --bootstrap --methods-dir "%SCRIPT_DIR%methods" %*
exit /b %ERRORLEVEL%
