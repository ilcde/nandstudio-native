# Build, test and package

Only Windows x86-64 has been built and exercised locally. Other configurations
are reproducible starting points, not verified platform support. See
`platforms.json` for stage-by-stage status.

## Windows

Local Qt: `C:/Qt/6.11.1/msvc2022_64`. MSVC tools: 14.51.36231 (compiler
19.51.36246), Visual Studio 2026 18.6.2. CMake: bundled 3.30.5. The installed MSVC
is newer than the MSVC 2022 combination explicitly listed by Qt; local success
does not establish official support for this pairing. SDK/NDK/tool selections are
recorded in `toolchain-lock.json`; not all proposed versions are installed.

```powershell
./scripts/build-windows.ps1
python scripts/audit_baseline.py --verify
python tests/test_archive.py
python scripts/differential.py
./scripts/package-windows.ps1
```

Build and GUI test runtime may need permissions outside a restricted sandbox.
The GUI test uses Qt's offscreen plugin and software rendering, and writes
screenshots plus check records under `build/gui-test/`. This is different from
launching with the Windows display integration.

The portable package is an **unsigned development build**, not an installer or a
fully compatible replacement. The current test build includes QtTest solely for
the development self-test mode. `BUILD_TESTING=OFF` removes that dependency.
No signing keys are created or embedded.

## Linux / macOS

Install Qt 6.11.1 including Quick, Quick Controls, Concurrent and Test, CMake
3.30.5 and Ninja 1.12.1. Configure with the Qt kit prefix:

```sh
cmake --preset desktop -DCMAKE_PREFIX_PATH=/absolute/path/to/Qt/6.11.1/kit
cmake --build --preset desktop
ctest --preset desktop
cmake --install build --prefix "$PWD/dist/NandStudio"
```

Target Linux baseline: Ubuntu 24.04, x86-64, GCC 13.3.0, glibc 2.39. Qt's official
6.11 binary packages use Ubuntu 24.04; older glibc requires rebuilding Qt.
Target macOS: 13+, arm64 and x86-64, built with Xcode 16.2; set
`CMAKE_OSX_DEPLOYMENT_TARGET=13.0` and `CMAKE_OSX_ARCHITECTURES` appropriately.
CMake enables an app bundle. DMG/notarization and Linux distribution dependency
validation are not finished. These hosts were not available in this session.

Core-only tests do not need Qt or a display:

```sh
cmake --preset core
cmake --build --preset core
ctest --preset core
```

The `asan` preset enables AddressSanitizer/UBSan for compatible GCC/Clang hosts.
It has not been run on this Windows MSVC workstation.

## Android

The [Qt 6.11 support matrix](https://doc.qt.io/qt-6/supported-platforms.html)
specifies Android 9–16, NDK r27c (27.2.12479018), JDK 21, Gradle 9.3.1 and AGP
9.0.0. Intended ABIs are arm64-v8a and x86_64. Minimum API is 28, target 36.
Pin SDK build-tools 36.0.0. This machine has Android SDK platforms 34/35/36,
build-tools 34.0.0/35.0.0/36.1.0 and an API 36.1 emulator definition. It has no
Android Qt kit, no NDK, and no attached device at inspection time.

After installing matching host and Android Qt 6.11.1 kits and the pinned NDK:

```sh
export QT_HOST_PATH=/absolute/path/to/host/qt
export QT_ANDROID_ROOT=/absolute/path/to/android_arm64_v8a/qt
export ANDROID_SDK_ROOT=/absolute/path/to/sdk
sh scripts/build-android.sh arm64-v8a
python scripts/check_android_package.py path/to/application.apk
"$ANDROID_SDK_ROOT/build-tools/36.0.0/zipalign" -c -P 16 -v 4 path/to/application.apk
```

CI now produces ARM64 and x86-64 APK/AAB artifacts. A local development-signed
x86-64 APK installed and launched on the API 36.1 emulator; see platforms.json.
A 16 KiB link flag alone is not proof
of package compatibility; check every shipped ELF and ZIP alignment, then install
on a 16 KiB device/emulator. SAF implementation, persistent grants, lifecycle,
import/export and offline workflow tests are still blocking application work,
not merely missing build tools. Android is not declared supported by this build.


Hardware increment validation:

```powershell
python scripts/differential.py
python scripts/probe_hardware_differences.py
./scripts/package-windows.ps1
python scripts/test_windows_package.py
```

The signed-pin probe now passes 23 byte-exact Java/native cases; see
`docs/evidence/hardware-known-differences.json`. This resolves the previously
recorded narrow-negative mismatch, not every remaining HDL compatibility edge.
The package test uses the Windows Qt plugin with software rendering and a PATH
containing only Windows system directories; the current application runs 221 GUI checks plus a native
hardware batch fixture against previously captured uploaded-binary output.
Qt's deployment subprocesses may require the same host execution permission as
the compiler. No external Qt or Java runtime is needed by the resulting package.


The latest hardware package is `dist/NandStudio-hardware-windows-x86_64.zip`.
It was staged separately because the earlier package was running. To reproduce:

```powershell
./scripts/package-windows.ps1 -Destination dist/NandStudio-hardware-windows-x86_64
python scripts/test_windows_package.py dist/NandStudio-hardware-windows-x86_64
```

The package includes pinned MSVC 14.51.36231 release CRT DLLs beside the executables.
Hashes are in evidence/windows-crt.json; the original Microsoft license remains
applicable (see licenses/MSVC-runtime-notice.txt). Microsoft lists unmodified
release files under VC/redist as distributable, subject to its license terms:
https://learn.microsoft.com/en-us/visualstudio/releases/2026/redistribution.
Qt deployment warns about absent Direct3D 12 shader compiler DLLs; the software
renderer was tested, GPU/Direct3D 12 rendering was not. A clean Windows installation
has not been tested, even though app-local CRT dependencies are now included.

## Isolated source and package verification

`./scripts/build-windows.ps1 -BuildDirectory build-continuation` selects a fresh
workspace-contained build directory. `./scripts/package-windows.ps1 -BuildDirectory
build-continuation -Destination dist/NandStudio-editor-windows-x86_64` deploys into
a separate package, preserving older running packages.

`python scripts/source_package.py dist/NandStudio-source.zip` writes a deterministic
source archive with sorted names, stable permissions and 2000-01-01 timestamps.
It includes the unchanged original runtime ZIP for development reference tests,
not runtime delegation. It excludes reference clones, generated build trees,
working test output and deployed binaries. Unpack to an empty directory after
validating entries with `scripts.audit_baseline.checked_entries`; never extract
over an existing project. Install the pinned toolchain and run the build script
from that extracted directory. Exact archive reproducibility is tested on the
recorded Python/zlib host; binary reproducibility across toolchains is unproven.

Set `NAND_NATIVE_BUILD` to an absolute build directory before running
`scripts/differential.py` or `scripts/probe_hardware_differences.py` to avoid testing
an older build accidentally. The reference harness remains development-only.

The earlier Docker startup failure remains historical evidence. Linux now builds
in isolated Ubuntu 24.04 WSL (`NandStudioBuild`) and CI. All three suites pass;
the portable package passes 221 GUI checks with offscreen and X11/Xvfb plugins.
Android builds use the pinned Qt/NDK tools in CI; the local SDK installed and
launched the downloaded x86-64 package. SAF and full workflows remain unimplemented.
See `simulator-continuation.md` for current evidence and artifact locations.

## Development GitHub Releases

Every successful `native.yml` run on main automatically triggers `release.yml`.
All jobs must succeed at the same source revision. Manual retry is also available
using that run's numeric ID (an existing release will not be overwritten):

```sh
gh workflow run release.yml --repo ilcde/nandstudio-native -f build_run_id=RUN_ID
```

The workflow publishes a prerelease named `development-RUN_ID`, with Windows,
Linux, both macOS architectures, both Android ABIs, tracked source, SHA256SUMS and
provenance. It refuses missing packages, failed desktop package checks, a failed
CI run or mismatched source revision. Existing releases are never overwritten.
Android development APKs use Gradle's debug signing; release keys, macOS signing
and notarization are not configured. This workflow is prepared and locally
validated but has not yet executed remotely.
