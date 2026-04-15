#Requires -Version 5.0
param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release",

    [switch]$SkipExe,

    [string]$InnoSetupCompiler = ""
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build"
$Packaging = Join-Path $Root "packaging"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "cmake not found on PATH"
}

function Get-ProjectVersion {
    $m = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(TruckPackerWrapper VERSION ([0-9.]+)' |
        ForEach-Object { $_.Matches.Groups[1].Value } |
        Select-Object -First 1
    if (-not $m) { return "0.1.0" }
    return $m
}

function Find-InnoCompiler {
    param([string]$Explicit)
    if ($Explicit -and (Test-Path $Explicit)) { return $Explicit }
    $cmd = Get-Command "ISCC.exe" -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    foreach ($p in @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles}\Inno Setup 6\ISCC.exe",
        "${env:ProgramFiles(x86)}\Inno Setup 5\ISCC.exe"
    )) {
        if (Test-Path $p) { return $p }
    }
    return $null
}

New-Item -ItemType Directory -Force -Path $Build | Out-Null
cmake -S $Root -B $Build -DCMAKE_BUILD_TYPE=$Config
cmake --build $Build --config $Config --parallel

cmake "-DTP_BUILD_DIR=$Build" "-DTP_SOURCE_DIR=$Root" "-DTP_CONFIG=$Config" -P (Join-Path $Packaging "stage_plugins.cmake")

$VerLine = Get-ProjectVersion
$Stage = Join-Path $Packaging "_stage"
$DistRoot = Join-Path $Packaging "windows_dist"

Remove-Item $DistRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path (Join-Path $DistRoot "VST3") | Out-Null
Copy-Item -Recurse -Force (Join-Path $Stage "VST3\Truck Packer.vst3") (Join-Path $DistRoot "VST3\Truck Packer.vst3")
Copy-Item -Force (Join-Path $Stage "INSTALL.txt") (Join-Path $DistRoot "INSTALL.txt")

$Zip = Join-Path $Packaging "TruckPacker-$VerLine-Windows.zip"
if (Test-Path $Zip) { Remove-Item $Zip -Force }
Compress-Archive -Path (Join-Path $Stage "*") -DestinationPath $Zip
Write-Host "Built: $Zip"

if ($SkipExe) {
    Write-Host "Skipped EXE (Inno Setup). Install Inno Setup 6 and re-run without -SkipExe, or set -InnoSetupCompiler path to ISCC.exe"
    exit 0
}

$iscc = Find-InnoCompiler -Explicit $InnoSetupCompiler
if (-not $iscc) {
    Write-Warning "Inno Setup (ISCC.exe) not found. ZIP is ready; install Inno Setup 6 from https://jrsoftware.org/isinfo.php and re-run this script to produce a setup EXE."
    exit 0
}

Push-Location $Packaging
try {
    & $iscc "/DAppVersion=$VerLine" "TruckPacker.iss"
} finally {
    Pop-Location
}

$Exe = Join-Path $Packaging "TruckPacker-$VerLine-Windows-Setup.exe"
if (Test-Path $Exe) {
    Write-Host "Built: $Exe"
} else {
    Write-Warning "ISCC finished but EXE not found at expected name (check packaging\ for TruckPacker-*-Windows-Setup.exe)"
}
