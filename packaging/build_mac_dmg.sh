#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
CONFIG="${1:-Release}"

cmake -S "$ROOT" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG"
cmake --build "$BUILD" --parallel

cmake -DTP_BUILD_DIR="$BUILD" -DTP_SOURCE_DIR="$ROOT" -DTP_CONFIG="$CONFIG" -P "$ROOT/packaging/stage_plugins.cmake"

VER="$(grep 'TruckPackerWrapper VERSION' "$ROOT/CMakeLists.txt" | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
[[ -z "${VER}" ]] && VER="0.1.0"

DMG_SRC="${ROOT}/packaging/_dmg_source"
rm -rf "${DMG_SRC}"
mkdir -p "${DMG_SRC}"

cp -R "${ROOT}/packaging/_stage/Library/Audio/Plug-Ins/VST3/Truck Packer.vst3" "${DMG_SRC}/"
if [[ -d "${ROOT}/packaging/_stage/Library/Audio/Plug-Ins/Components/Truck Packer.component" ]]; then
    cp -R "${ROOT}/packaging/_stage/Library/Audio/Plug-Ins/Components/Truck Packer.component" "${DMG_SRC}/"
fi
cp "${ROOT}/packaging/DMG_README.txt" "${DMG_SRC}/Read Me First.txt"
cp "${ROOT}/packaging/INSTALL.txt" "${DMG_SRC}/INSTALL.txt"

DMG_OUT="${ROOT}/packaging/TruckPacker-${VER}-macOS.dmg"
rm -f "${DMG_OUT}"

hdiutil create -volname "Truck Packer ${VER}" -srcfolder "${DMG_SRC}" -ov -format UDZO "${DMG_OUT}"

echo "Built: ${DMG_OUT}"
