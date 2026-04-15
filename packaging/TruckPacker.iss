; Truck Packer — Windows installer (Inno Setup 6+)
; Build: install Inno Setup, run packaging/build_win_installer.ps1 (prepares windows_dist\ then runs ISCC).
; https://jrsoftware.org/isinfo.php

#define AppName "Truck Packer"
#ifndef AppVersion
  #define AppVersion "0.1.0"
#endif
#define Publisher "Truck Packer"
#define DistDir "windows_dist"

[Setup]
AppId={{B2C8F4A1-3E6D-4B7C-9F2E-1A3B5D7E9C0F}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL=https://truckpacker.com
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableDirPage=yes
DisableProgramGroupPage=yes
AllowNoIcons=yes
OutputDir=.
OutputBaseFilename=TruckPacker-{#AppVersion}-Windows-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName={#AppName}
UninstallFilesDir={autopf}\{#AppName}\uninstall
VersionInfoVersion={#AppVersion}.0
CloseApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; 64-bit VST3 system folder (same as C:\Program Files\Common Files\VST3\)
Source: "{#DistDir}\VST3\Truck Packer.vst3\*"; DestDir: "{commoncf}\VST3\Truck Packer.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DistDir}\INSTALL.txt"; DestDir: "{autopf}\{#AppName}"; Flags: ignoreversion

[Icons]
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
