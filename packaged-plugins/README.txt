After a successful build, CMake copies fresh plug-ins here (same as under build/.../Release):

  Truck Packer.vst3
  Truck Packer.component

Your DAW still loads from:
  ~/Library/Audio/Plug-Ins/VST3/
  ~/Library/Audio/Plug-Ins/Components/

when COPY_PLUGIN_AFTER_BUILD is on. This folder is only a convenient duplicate next to the source tree.

Installers: packaging/build_mac_dmg.sh (macOS .dmg), packaging/build_mac_pkg.sh (.pkg), packaging/build_win_installer.ps1 (.exe via Inno Setup + .zip). See packaging/INSTALL.txt.
