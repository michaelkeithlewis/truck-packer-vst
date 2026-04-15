#Requires -Version 5.0
param(
    [ValidateSet("Release", "Debug")]
    [string]$Config = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "cmake not found on PATH"
}

New-Item -ItemType Directory -Force -Path $Build | Out-Null
cmake -S $Root -B $Build -DCMAKE_BUILD_TYPE=$Config
cmake --build $Build --config $Config --parallel

cmake "-DTP_BUILD_DIR=$Build" "-DTP_SOURCE_DIR=$Root" "-DTP_CONFIG=$Config" -P (Join-Path $Root "packaging\stage_plugins.cmake")

$Stage = Join-Path $Root "packaging\_stage"
$VerLine = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(TruckPackerWrapper VERSION ([0-9.]+)' | ForEach-Object { $_.Matches.Groups[1].Value }
if (-not $VerLine) { $VerLine = "0.1.0" }

$Zip = Join-Path $Root "packaging\TruckPacker-$VerLine-Windows.zip"
if (Test-Path $Zip) { Remove-Item $Zip -Force }

Compress-Archive -Path (Join-Path $Stage "*") -DestinationPath $Zip
Write-Host "Built: $Zip"
Write-Host "Unzip and copy VST3\Truck Packer.vst3 to C:\Program Files\Common Files\VST3\"
