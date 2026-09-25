# Installation and troubleshooting

Download from [GitHub Releases](https://github.com/ilcde/nandstudio-native/releases).
Choose a release and keep its `manifest.json`, `SHA256SUMS` and matching package
together. Do not mix assets from different versions. Qt and native engines are
bundled; users do not need Java, Python, a compiler or a network connection to run
them. These are development previews with [known gaps](parity-manifest.json).

## Windows

Target: Windows 10/11 x86-64. Recorded tests use Windows 11 locally and Windows
Server 2022 in CI. There is no native Windows ARM64 package.

1. Extract `NandStudio-windows-x86_64.zip` into a writable user folder.
2. Run `bin\NandStudio.exe`. Keep DLLs, plugins, QML and resources together;
   copying only the EXE does not work.
3. Choose Files > Open workspace and select a copy of your project.

No administrator installation is required. The package is unsigned. Check the
source and checksum before deciding to run it. If graphics initialization fails:

```powershell
$env:QT_QUICK_BACKEND = 'software'
& .\bin\NandStudio.exe
```

Native command tools are in `bin`, for example:

```powershell
& .\bin\HardwareSimulator.exe 'C:\Course work\01\Xor.tst'
& .\bin\Assembler.exe 'C:\Course work\04\Mult.asm'
& .\bin\JackCompiler.exe 'C:\Course work\11\Square'
```

## macOS

Target: macOS 13 or newer. Choose **arm64** for Apple Silicon and **x86_64** for
Intel processors. Both are separately built and GUI-tested in CI.

1. Open the corresponding DMG.
2. Copy `NandStudio.app` into a writable Applications folder, such as
   `~/Applications`. Keep accompanying command tools/resources together if using
   them separately.
3. Open the copied app. It is unsigned and unnotarized. If macOS blocks it, review
   its source/checksum and use macOS's per-app approval procedure if you trust it.
   Do not disable system-wide security controls.

The GUI executable is `NandStudio.app/Contents/MacOS/NandStudio`. Named command
tools are in the package's `bin` directory. Clean-device installation and all
course workflows have not been certified.

## Linux

Package target: x86-64, glibc 2.39 or newer (Ubuntu 24.04 baseline). This is a
tarball, not an AppImage, Flatpak or package-manager installer.

```sh
mkdir nandstudio
tar -xJf NandStudio-linux-x86_64.tar.xz -C nandstudio
cd nandstudio
./NandStudio.sh
```

Use the launcher to locate bundled Qt libraries/plugins. The host still supplies
glibc, graphics drivers and system X11 libraries. CI installs
`libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0 libxkbcommon-x11-0` on Ubuntu 24.04;
see `scripts/package-linux.py` for the bundled dependency selection. Headless
native tools in `bin` do not require a display server. Older runtime baselines
need an appropriate rebuild, including compatible Qt libraries.

## Android

Target: Android 9/API 28 through Android 16/API 36; ARM64 devices and x86-64
emulators. No ARM32 or x86 build is supplied. Physical ARM64 device workflows
remain unverified.

1. Download `NandStudio-android-arm64-v8a-development.apk` on a compatible device.
2. Open it with Android's package installer. If needed, allow installation for
   that specific file/download app and review the prompt.
3. Open NandStudio. Narrow screens use Files, Editor, Machine and Console panes;
   More contains settings and find/replace.

For an x86-64 emulator:

```sh
adb install -r NandStudio-android-x86_64-development.apk
adb shell am start -n org.qtproject.example.NandStudio/org.qtproject.qt.android.bindings.QtActivity
```

Use the ARM64 APK for ARM64 devices. APKs use debug signing; CI keys can differ
between releases. If an update fails because of a signature mismatch, do not
uninstall before backing up important data. Uninstallation normally erases
app-private documents/settings. A stable production update channel is not configured.

**Unfinished workflows:** Storage Access Framework directory grants, provider-backed
editing, import/export and lifecycle/process recreation. Installing the APK does
not establish a complete course-project workflow. Do not assume a document URI is
a filesystem folder. ELF/ZIP alignment is checked for 16 KB, but 16 KB runtime
testing remains pending. Release layout reports record the emulator/API/page size
actually exercised; they verify toolbar geometry, not full device parity.

## Checksums and upgrades

Compare a package hash with its entry in `SHA256SUMS`:

```powershell
Get-FileHash .\NandStudio-windows-x86_64.zip -Algorithm SHA256
```

On Linux use `sha256sum package-name`; on macOS use `shasum -a 256 package-name`.
A checksum establishes matching bytes, not publisher signing or full correctness.

Keep coursework outside the application package. Close the old app and extract
the new package separately. Preserve project backups. Do not overwrite expected
`.cmp` files to make a failing test pass.

## Troubleshooting

- **Xor output stays zero:** inspect inputs and the named local dependencies.
  Empty course starters override built-ins, just as in the original tools.
- **HDL edits appear ignored:** choose Reload & Eval to load visible buffers and
  reset hardware state. Invalid source leaves the last valid simulation intact.
- **Search is hidden:** choose More > Find and replace or Ctrl+F.
- **Old source is tested:** save documents before running a test script. Builds
  and HDL loading explicitly use visible-buffer snapshots; read console messages.
- **Unknown VM OS function:** supply the course's required compiled OS VM files
  in the project until native built-in fallback is implemented.
- **Save conflict:** review the disk version in the conflict dialog. Autosave
  does not silently overwrite external changes.

Report other problems using [the issue guide](../CONTRIBUTING.md).
