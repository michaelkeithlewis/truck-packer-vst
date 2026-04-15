# Truck Packer (JUCE)

VST3 / AU instrument that embeds [app.truckpacker.com](https://app.truckpacker.com) and can loop optional “inspirational music” from a local file.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Installers — GitHub **Releases**, not **Packages**

- **Packages** on GitHub is for registry formats (npm, Docker, NuGet, etc.). DMG / EXE / ZIP plug-in builds do **not** appear there.
- Publish installers under [**Releases**](https://github.com/michaelkeithlewis/truck-packer-vst/releases): create a release, attach `TruckPacker-*-macOS.dmg`, `.pkg`, and `TruckPacker-*-Windows-Setup.exe` (or `.zip`) from `packaging/` after running the scripts in `packaging/INSTALL.txt`.

## Inspirational music file

Put `videoplayback.mp4`, `.wav`, or `.mp3` in **Downloads**, **Desktop**, **Documents**, **Music**, or **Movies**, or set env **`TRUCK_PACKER_LOOP`** to the full path. See `packaging/INSTALL.txt` for installer paths and platform notes.
