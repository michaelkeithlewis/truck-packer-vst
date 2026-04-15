# Truck Packer (JUCE)

VST3 / AU instrument that embeds [app.truckpacker.com](https://app.truckpacker.com) and can loop optional “inspirational music” from a local file.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Automated releases (CI)

Push a **version tag** matching `v*` (after bumping `project(... VERSION ...)` in `CMakeLists.txt` so filenames match):

```bash
git tag v0.1.1
git push origin v0.1.1
```

GitHub Actions (`.github/workflows/release.yml`) builds **macOS DMG + PKG** and **Windows ZIP + Inno Setup EXE**, then uploads everything to a **Release** for that tag.

### Where is the Windows `.exe`?

- It is **not committed to Git** (installers are build artifacts; `.gitignore` excludes `*-Windows-Setup.exe`). In the repo you only have **`packaging/TruckPacker.iss`** (Inno Setup script) and **`packaging/build_win_installer.ps1`**.
- The **downloaded installer** is **`TruckPacker-<version>-Windows-Setup.exe`**, attached under [**Releases → Assets**](https://github.com/michaelkeithlewis/truck-packer-vst/releases) for a tag whose **Release** workflow completed successfully (alongside the `.zip`).
- Release **v0.1.0** was published **before** CI ran, so its assets are **macOS-only**. Push a **new tag** (e.g. `v0.1.1`) after `main` includes `.github/workflows/release.yml` so Actions can produce the Windows EXE and ZIP.

## Installers — GitHub **Releases**, not **Packages**

- **Packages** on GitHub is for registry formats (npm, Docker, NuGet, etc.). DMG / EXE / ZIP plug-in builds do **not** appear there.
- Publish installers under [**Releases**](https://github.com/michaelkeithlewis/truck-packer-vst/releases): create a release, attach `TruckPacker-*-macOS.dmg`, `.pkg`, and `TruckPacker-*-Windows-Setup.exe` (or `.zip`) from `packaging/` after running the scripts in `packaging/INSTALL.txt`.

## Inspirational music

The **MP4 in `resources/videoplayback.mp4`** is compiled into the binary (JUCE `juce_add_binary_data`). To change the loop, replace that file and rebuild. You can still override at runtime with **`TRUCK_PACKER_LOOP`** or loose `videoplayback.*` files in common folders (see `packaging/INSTALL.txt`).
