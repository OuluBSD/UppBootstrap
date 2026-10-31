# ==============================================================================
# UppBootstrap: Zero-dependency U++ umk bootstrap toolchain for Windows
# Supports MSVC (cl.exe via vswhere/VsDevCmd), Clang, and MinGW
# ==============================================================================

[CmdletBinding()]
param(
    [string]$UppRoot = '',
    [ValidateSet('x86', 'x64')]
    [string]$Arch = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ScriptDir = $PSScriptRoot
$Source = Join-Path $ScriptDir 'src\main.cpp'
$BinDir = Join-Path $ScriptDir 'bin'
$Output = Join-Path $BinDir 'upp-bootstrap.exe'

if (-not (Test-Path -LiteralPath $BinDir)) {
    New-Item -ItemType Directory -Path $BinDir | Out-Null
}

function Get-VswherePath {
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe')
    )
    foreach ($cand in $candidates) {
        if ($cand -and (Test-Path -LiteralPath $cand)) { return $cand }
    }
    return $null
}

function Get-VsDevCmd {
    $vswhere = Get-VswherePath
    if ($vswhere) {
        $installPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($installPath -and (Test-Path -LiteralPath $installPath)) {
            $cmd = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
            if (Test-Path -LiteralPath $cmd) { return $cmd }
        }
    }
    return $null
}

$vsDevCmd = Get-VsDevCmd

if ($vsDevCmd) {
    Write-Host "==> UppBootstrap: Compiling runner via MSVC ($Arch)..."
    $compileCmd = 'call "' + $vsDevCmd + '" -no_logo -arch=' + $Arch + ' && cl /nologo /EHsc /std:c++17 /O2 /Fe:"' + $Output + '" "' + $Source + '"'
    & cmd.exe /d /s /c $compileCmd
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to compile upp-bootstrap.exe with exit code $LASTEXITCODE"
    }
} elseif (Get-Command 'clang++' -ErrorAction SilentlyContinue) {
    Write-Host "==> UppBootstrap: Compiling runner via Clang..."
    & clang++ -std=c++17 -O2 -o $Output $Source
} elseif (Get-Command 'g++' -ErrorAction SilentlyContinue) {
    Write-Host "==> UppBootstrap: Compiling runner via MinGW GCC..."
    & g++ -std=c++17 -O2 -o $Output $Source
} else {
    throw "No supported C++ compiler found (MSVC, Clang, or MinGW GCC)."
}

Write-Host "==> UppBootstrap: Running bootstrap toolchain..."
$extraArgs = @('--bootstrap', '--methods-dir', (Join-Path $ScriptDir 'methods'))
if ($UppRoot) {
    $extraArgs += @('--upp-root', $UppRoot)
}

& $Output @extraArgs
if ($LASTEXITCODE -ne 0) {
    throw "Bootstrap execution failed with code $LASTEXITCODE"
}
