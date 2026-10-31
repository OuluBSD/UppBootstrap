@echo off
REM ============================================================================
REM install-methods.cmd: Install U++ build methods (.bm) to Windows config dirs
REM ============================================================================

setlocal EnableDelayedExpansion

set SCRIPT_DIR=%~dp0
set METHODS_DIR=%SCRIPT_DIR%methods

if not exist "%METHODS_DIR%" (
    echo Error: methods directory not found at "%METHODS_DIR%"
    exit /b 1
)

echo ==> Installing U++ build methods on Windows...

if defined APPDATA (
    set "THEIDE_DIR=%APPDATA%\u++\theide"
    set "UMK_DIR=%APPDATA%\u++\umk"
    if not exist "!THEIDE_DIR!" mkdir "!THEIDE_DIR!"
    if not exist "!UMK_DIR!" mkdir "!UMK_DIR!"
    copy /y "%METHODS_DIR%\*.bm" "!THEIDE_DIR!\" >nul
    copy /y "%METHODS_DIR%\*.bm" "!UMK_DIR!\" >nul
    echo ==> Installed build methods to: !THEIDE_DIR!
)

if defined USERPROFILE (
    set "UPP_DIR=%USERPROFILE%\upp"
    if not exist "!UPP_DIR!" mkdir "!UPP_DIR!"
    copy /y "%METHODS_DIR%\*.bm" "!UPP_DIR!\" >nul
    echo ==> Installed build methods to: !UPP_DIR!
)

echo ==> umk is now configured with MSVS22x64, CLANG, and GCC build methods.
exit /b 0
