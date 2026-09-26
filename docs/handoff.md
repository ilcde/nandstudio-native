# Development handoff — incomplete migration

> Historical checkpoint. For current instructions and status, read the
> [user guide](user-guide.md), [checklist](TODO.md), and [parity register](feature-parity.md).


The working application and native services have been implemented directly in
this workspace. The result is a tested development increment, not the complete
cross-platform migration requested.

## Artifacts

* `dist/NandStudio-hardware-windows-x86_64.zip`: unsigned portable Windows development package.
* `dist/NandStudio-hardware-windows-x86_64/bin/NandStudio.exe`: application.
* Same `bin/`: Assembler, CPUEmulator, VMEmulator, JackCompiler, TextComparer,
  HardwareSimulator and unified `nand` executables. HardwareSimulator now executes
  the native HDL/script subset; see hardware.md for outstanding compatibility gaps.
* `src/`, `CMakeLists.txt`, `CMakePresets.json`, scripts and CI YAML: implementation
  and build configuration. CI jobs are authored but have not run on hosted agents.

## Verified in this environment

* Windows x86-64 compilation: MSVC 19.51.36246, Qt 6.11.1, CMake 3.30.5,
  Ninja 1.12.1. Local compiler is newer than Qt's explicitly supported MSVC 2022.
* Core: 2,012 existing checks and 2,127 hardware checks pass, including clock
  timing, wrapping, ALU boundaries, graph checks and malformed partial input.
* GUI: 26 checks pass under both offscreen and Windows platform plugins, covering
  editing, undo/redo, save semantics, conflicts, assembly, CPU execution, HDL loading/pins/tick/tock/reload and layouts.
* Differential harness: 65 cases match the uploaded Java binaries. This includes
  63 output/comparison cases and matching rejections of the unchanged NestedCall
  fixture and writing Keyboard from a script. Those are not successful execution
  tests. A separate probe reproduces the unresolved narrow-negative HDL mismatch.
* Archive safety: four test methods pass, including traversal, duplicate paths,
  symlinks and valid paths. Baseline integrity verification passes for all 881
  entries; original extracted files are byte-identical to the uploaded archive.
* Portable package: launched and exercised with system Qt and Java absent from
  PATH. Runtime dependencies resolve from the package. Software rendering was
  used; GPU backend coverage is not established.

The main suites pass. The separate known-difference probe still fails for
negative values on narrow HDL pins, as recorded in hardware-known-differences.json.
Coverage remains limited; passing the other tests does not establish full parity.

## Release blockers

Remaining HDL numerical/timing/GUI parity; native OS services; Java extension binary compatibility;
complete script/CLI/diagnostic behavior; remaining editor and GUI operations;
Android SAF, lifecycle and device workflow; Android Qt/NDK installation and
16 KiB package testing; macOS/Linux builds and packages; complete licensing and
dependency-lock audit. No Android APK/AAB exists. No platform meets the full
requested completion criteria.

The complete conservative register is `parity-manifest.json` (227 feature entries
and 150 GUI discovery rows). Consult `progress.md` for continuation priorities,
`baseline.md` for provenance and `evidence/` for machine-readable test results.


## Editor continuation — 24 September 2026

Implemented and packaged the editor/workspace/UI increment. See continuation-report.md
for changed files, exact test results, artifact paths and remaining release blockers.
Use build-continuation for the current Windows build. The original build directory
contains older binaries; set NAND_NATIVE_BUILD when running differential tests.
No Android/macOS/Linux completion is claimed.
