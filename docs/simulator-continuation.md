# Simulator and platform verification increment - 24 September 2026

The migration remains incomplete. This report supersedes the earlier editor-only
continuation report for the changes described here.

Implemented and tested:

- Eval commits all pending input fields atomically; invalid values preserve the
  prior state. A failing regression reproduced the original native GUI defect.
  Mouse/keyboard regression now covers all 21 requested combinational chips.
- HDL whole pins retain signed 16-bit values; explicit sub-buses mask and merge.
  Narrow negative values, adders, ALU controls and mux/demux selection now match
  23 uploaded-Java differential probes byte for byte. All 65 earlier cases pass.
- Explicit Eval/Tick/Tock/Cycle snapshots feed a bounded 256-event signal history.
  Decimal, hex and binary values and clock phases are real backend snapshots.
- Composite hierarchy and directed wire snapshots feed a zoomable connection
  diagram, pin values and source/target slice list. It is an initial educational
  view, not full legacy visualization or animation parity.
- Created and uploaded the Git repository after explicit approval. Continued
  public publication only after separate user approval of public visibility.
  Actual Windows/Linux/macOS/Android CI runs now produce logs and expose failures.
- Installed isolated Ubuntu 24.04 WSL host NandStudioBuild and pinned Qt 6.11.1,
  CMake 3.30.5, Ninja 1.13.2 and aqtinstall 3.3.0. Linux builds no longer depend on
  Docker. GCC observed is Ubuntu GCC 13.3.0; full distro dependency locking remains.

| Area | Implementation | Build/test evidence | Remaining issue |
|---|---|---|---|
| Hardware Eval | Atomic pending-input commit | 21-chip GUI matrix; clock unchanged | Broader original interaction inventory incomplete |
| HDL negative pins | Whole-pin short nodes and slice adapters | 23/23 byte-exact Java probes | Nonstandard clock declarations and other grammar edges |
| Hardware hierarchy/wires | C++ snapshots, live diagram and connection list | Composite Register GUI and core hierarchy tests | Constant display, richer routing, animation, breakpoints |
| Waveform/history | Bounded action snapshots, selectable signal and radix | Timing/values/clear-history tests | Multi-signal, component pins, batch cycles, export |
| CPU | Existing native engine/inspector retained | Existing core and differential cases | Full interactive controls/breakpoints/display parity |
| VM | Existing native engine retained | Existing core and differential cases | Native OS fallback/services and complete call-stack UI |
| Editor | Existing native workspace/editor retained | GUI edits, saves, undo/redo, conflict/recovery tests | Parser-backed completion/navigation and Android storage |
| Scripts/CLI | Existing native services and launchers retained | 65 differential cases | Full command/diagnostic/launcher parity |
| Android | Shared core compiled for ARM64 and x86-64 | Both CI packages and alignment checks pass; x86-64 APK installed and launched locally | SAF, lifecycle, all device workflows and 16 KiB runtime testing |
| Extensions | No new compatibility claim | None | Existing Java extension binaries remain release blocker |

Latest local Windows source checks: 2,012 core, 2,139 hardware, 221 GUI; all three
CTest suites pass. Linux build-tree suites also pass with the hierarchy changes.
The current hierarchy Linux package passed all 221 checks with both offscreen and
X11/Xvfb plugins. An earlier package also launched visibly in WSLg.
These are partial workflow tests, not
platform certification. No starter or reference file was changed: the audit
reconfirmed 881 files, seven JARs and zero extraction differences.

Useful paths:

- Current Windows build: `build-continuation`
- New unsigned package: `dist/NandStudio-circuit-windows-x86_64.zip`
- Linux development bundle: `dist/NandStudio-linux-x86_64.tar.xz`
- Android emulator development APK: `dist/NandStudio-android-x86_64-development.apk`
- Detailed current platform stages: `docs/platforms.json`
- Local build/runtime logs: `tests/work/`
- Durable regression evidence: `docs/evidence/`
- CI: https://github.com/ilcde/nandstudio-native/actions

Linux build tools live inside the isolated distribution in `/opt/nandstudio-tools`,
Qt in `/opt/nandstudio-qt/6.11.1/gcc_64`, and build output in `/opt/nandstudio-build`.
Invoke `wsl -d NandStudioBuild -u root --exec /opt/nandstudio-tools/bin/cmake --build
/opt/nandstudio-build --parallel 4`, then the corresponding `ctest --test-dir
/opt/nandstudio-build --output-on-failure`. All package scripts use the shared
native implementation; Java remains development-only reference testing.

Android x86-64 installation and cold activity launch succeeded on the existing
API 36.1 emulator. Logcat confirms the native application and QML loaded, and the
process remained alive. The downloaded CI Release APK was unsigned; this local
copy uses the workstation's existing Android debug key. Signature and ZIP
alignment checks passed, as did alignment of all 88 native ELF libraries. The
emulator reports 4096-byte pages: this is not a 16 KiB runtime test. No complete
Android edit/save/test/export workflow has been verified.

GitHub run 35992557632 produced ARM64 and x86-64 APK/AAB artifacts, passed Linux,
Apple Silicon macOS packaged tests/DMG generation, and sanitizers. Intel macOS
passed in run 35991900348; the later run's final Intel status has not been read.
The Windows job failed while aqt extracted Qt; native 7-Zip extraction is prepared
but has not been verified in CI. Local Windows builds and package tests pass.

The user requested all-platform GitHub Release assets. A dispatchable development
release workflow now requires all six packages from one successful main-branch
CI run, checks desktop packaged-test evidence, and emits checksums/provenance.
Five local release-staging tests pass, including missing-platform and wrong-source
rejection. Windows CI packaging and Android debug APK generation are prepared but
unrun. No GitHub Release has been published: automatic approval review blocked
the next GitHub status request because workspace review credits were exhausted.
Restoring those credits is required to push, exercise the new workflow, and publish.
A fresh local Windows deployment attempt also failed inside the sandbox when
`windeployqt` could not launch `qtpaths6.exe` (pipe error). The incomplete directory
`dist/NandStudio-development-windows-x86_64` is not a release artifact; use the
previously tested circuit ZIP. Retrying deployment with host execution permission
requires the same currently unavailable approval service.


## Text fix and release continuation

GitHub approval access is restored. The user-reported Settings suffix was literal
mojibake in QML, also present in both Open menu entries and the hardware time
separator. These four strings now use Unicode escapes; valid Unicode elsewhere
is preserved. Four live QML property regressions pass (225 GUI checks total),
alongside the unchanged core and hardware suites. UTF-8 editor configuration
was added. No user document bytes were converted.

The latest preceding CI run also passed Intel macOS; Windows extraction was its
only failed job. The new release workflow automatically follows successful main
CI and publishes a development prerelease with all six platform packages. All
functional gaps remain explicitly documented; this is not full migration parity.
