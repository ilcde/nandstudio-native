# NandStudio

[![Native build checks](https://github.com/ilcde/nandstudio-native/actions/workflows/native.yml/badge.svg)](https://github.com/ilcde/nandstudio-native/actions/workflows/native.yml)

A native C++20 / Qt Quick workspace for Nand2Tetris: edit HDL, assembly, VM, Jack
and test scripts, inspect hardware, and run native tools locally. No JRE, Python
or web service is required by the application.

**Development preview.** The migration is incomplete. Packages exist for Windows,
macOS, Linux and Android; full course compatibility and Android project workflows
remain unfinished. Read the [parity register](docs/feature-parity.md) and
[platform evidence](docs/platforms.json) before relying on a workflow.

## Download

Open [GitHub Releases](https://github.com/ilcde/nandstudio-native/releases), choose
a development release, and expand **Assets**. Each includes source, checksums
and a manifest linking to the exact build.

| Device | Package | Instructions |
|---|---|---|
| Windows x86-64 | `NandStudio-windows-x86_64.zip` | [Windows](docs/install.md#windows) |
| Mac with Apple Silicon | `NandStudio-macos-arm64.dmg` | [macOS](docs/install.md#macos) |
| Mac with Intel processor | `NandStudio-macos-x86_64.dmg` | [macOS](docs/install.md#macos) |
| Linux x86-64 | `NandStudio-linux-x86_64.tar.xz` | [Linux](docs/install.md#linux) |
| Android ARM64 device | `NandStudio-android-arm64-v8a-development.apk` | [Android limitations](docs/install.md#android) |
| Android x86-64 emulator | `NandStudio-android-x86_64-development.apk` | [Android](docs/install.md#android) |

Desktop packages are unsigned; Android packages use development signing.
No store publication, Apple notarization or production signing is claimed.

## Get started

1. Extract/install the package and launch NandStudio.
2. On desktop, choose **Files > Open workspace** and select a copy of a project.
3. Open a source file. **More > Find and replace** (Ctrl+F) opens search.
4. **Build** compiles the visible ASM/Jack buffer or loads HDL. After HDL edits,
   **Reload & Eval** explicitly replaces the loaded chip and resets its state.
5. Open a saved `.tst`, choose CPU, VM or HDL test, and inspect console results.

Project-local HDL overrides built-ins. Empty student dependencies remain empty
and can keep outputs at zero; the app warns instead of completing exercises.
Android directory-provider access, import/export and lifecycle workflows are
unfinished. Installing an APK does not yet provide the complete desktop workflow.

## Documentation

- [Installation, upgrades and troubleshooting](docs/install.md)
- [Editor, simulators and console](docs/user-guide.md)
- [Course projects 1–12: coverage and remaining work](docs/course-workflows.md)
- [Build, test and package](docs/build.md)
- [Architecture](docs/architecture.md) and [baseline provenance](docs/baseline.md)
- [Feature parity](docs/feature-parity.md) and [machine-readable status](docs/parity-manifest.json)
- [Contribution and issue-reporting guide](CONTRIBUTING.md)
- [Licenses and attribution](NOTICE.md)

## Development

```sh
cmake --preset core
cmake --build --preset core
ctest --preset core
```

The core uses C++20 and CMake; GUI builds use Qt 6.11.1. Platform tools are listed
in `toolchain-lock.json`. Python and Java are development-only audit/reference-test
dependencies. Public source ZIPs contain the tracked native project and bundled
resources, not the original uploaded reference ZIP. Obtain the official desktop
distribution separately for reference tests; see the baseline guide.

Release automation requires all platform jobs, packaged desktop GUI tests,
sanitizers and the Android toolbar runtime gate to pass. Passing these checks
establishes only their tested scope, not complete parity.
