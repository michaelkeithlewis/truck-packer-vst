#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
CONFIG="${1:-Release}"

cmake -S "$ROOT" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE="$CONFIG"
cmake --build "$BUILD" --parallel

cmake -DTP_BUILD_DIR="$BUILD" -DTP_SOURCE_DIR="$ROOT" -DTP_CONFIG="$CONFIG" -P "$ROOT/packaging/stage_plugins.cmake"

IDENT="com.truckpacker.plugins.pkg"
VER="$(grep 'TruckPackerWrapper VERSION' "$ROOT/CMakeLists.txt" | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -1)"
[[ -z "${VER}" ]] && VER="0.1.0"

PKG_OUT="${ROOT}/packaging/TruckPacker-${VER}-macOS.pkg"
rm -f "$PKG_OUT"

pkgbuild --root "${ROOT}/packaging/_stage" \
    --identifier "$IDENT" \
    --version "$VER" \
    --install-location / \
    "$PKG_OUT"

echo "Built: $PKG_OUT"
echo "Install with: sudo installer -pkg \"$PKG_OUT\" -target /"
