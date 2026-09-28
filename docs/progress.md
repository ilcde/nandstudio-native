# Progress / continuation record



Work performed on 24 September 2026 directly in the supplied directory. No

existing Git history was available; no commit, remote, PR or publication created.



Completed increment:



* Inventoried 881 ZIP entries, JAR members, original launchers and assets; safely

  extracted reference and downloaded official source. Recorded exact hashes and

  source revision. Starter/project files unchanged.

* Implemented portable native assembler, CPU emulator, VM interpreter, Jack

  compiler, TextComparer and a CPU/VM script subset. No runtime Java delegation.

* Added Qt Quick application, C++ document model, syntax highlighting, multiple

  documents, safe saves with external-change detection, recovery, snapshot

  builds, CPU/VM controls and task console. Background tasks return state snapshots.

* Built and tested on local Windows/MSVC/Qt. Core test includes 2,012 checks;

  GUI test initially included 19 checks; the hardware increment below expands it. See actual evidence files for the latest run.

* Differential harness covers assembler matrix, legacy aliases, text comparison,

  CPU state traces, several bundled Jack programs and VM project 7/8 tests.

  Latest run has 29/29 matches: 28 output/comparison cases and one reproduced

  legacy rejection of NestedCall's indented label. No expected outputs were

  modified to obtain those matches.

* Produced an unsigned Windows portable ZIP and exercised its Windows platform

  plugin with system Qt and Java removed from PATH. The same 19 GUI checks pass

  against packaged binaries. No Android, macOS or Linux artifact was produced.



Reference isolation correction: initial Java batch runs changed CPU/VM `.dat`

preferences in the reference runtime. These two files were restored exactly

from the uploaded archive, and the harness now runs a disposable runtime copy.

`evidence/reference-runtime-isolation.json` records the repair. Baseline verification

subsequently passed. The user's original extraction was never modified.



Highest-priority remaining implementation:



1. Complete the VM loader's remaining symbol-pass edge cases and segment/frame

   restrictions, script grammar/formatting/error effects and compiler diagnostics.

2. Close the confirmed narrow-negative HDL mismatch, audit nonstandard clock

   declarations and hierarchical scheduling, then complete hardware UI/script parity.

3. Port all native OS services and preserve VM/local/built-in lookup and confirmation.

4. Resolve Java binary extension compatibility independently of ordinary engines.

5. Implement URI storage/SAF, grants, import/export, Android lifecycle and offline

   resources; install pinned Android toolchain and test ARM64/x86_64 packages.

6. Complete editor conveniences, file operations, diagnostics/symbol navigation,

   full emulator inspectors, breakpoints, layouts, accessibility and mobile IME.

7. Finish launchers/help/exit behavior, license audit, reproducible dependency

   locks, desktop packages, CI execution and real multi-platform workflows.



Do not infer completion from executable names, visible panels, CMake branches or

CI YAML. The manifest retains release blockers for all incomplete operations.





## Hardware continuation — 24 September 2026



Implemented native HDL parsing/loading, hierarchical bus connectivity, dependency

precedence and cycle checks; all 35 bundled built-in declarations embedded for

offline execution; sequential clock state, RAM/ROM, screen and keyboard. Added

HardwareSimulator batch scripts and the same services to Qt controls/console.

The GUI loads immutable folder snapshots including visible buffers on a worker,

shows pins/component pins, edits memory, and supports ROM loading and hardware

scripts. Failed reloads retain the last valid circuit. No student files changed.



Verification: 2,127 dedicated hardware checks plus 2,012 existing core checks;

26 GUI interaction checks; 65 differential matches including 36 hardware cases

(29 original scripts and seven independent fixtures). Two differential cases are

matching rejections (VM NestedCall and read-only Keyboard), not successful runs.

A separate probe reproduces a known failure for negative values on narrow pins;

its evidence remains failed and release-blocking. See hardware.md for exact

scope, source evidence, limits and unresolved clock/GUI/diagnostic behavior.



The Special `clk` node's spelling and initial level, positional built-in binding,

ROM command syntax, read-only Keyboard, signed binary/hex input and partial output

persistence on script errors were corrected against source/binary evidence.

Resources are original declarations, not replacement course implementations.



The refreshed portable Windows package passed all 26 GUI checks using the Windows

plugin and software rendering, with system Qt and Java removed from PATH. Its

HardwareSimulator executable also produced byte-identical Devices.out against

the uploaded baseline. The initial sandboxed deployment could not spawn qtpaths;

the authorized host-tool deployment succeeded. Baseline verification again found

881 unchanged files. Android/macOS/Linux states are unchanged and unverified.



Final package directory uses `NandStudio-hardware-windows-x86_64` because the prior

packaged app was open and locked its DLLs; that user session was left running.

The new package also includes pinned app-local MSVC release CRT DLLs, with hashes

and Microsoft attribution. Clean-machine/GPU deployment remains unverified.





## Editor continuation — 24 September 2026



Implemented and packaged the editor/workspace/UI increment. See continuation-report.md

for changed files, exact test results, artifact paths and remaining release blockers.

Use build-continuation for the current Windows build. The original build directory

contains older binaries; set NAND_NATIVE_BUILD when running differential tests.

No Android/macOS/Linux completion is claimed.



## Simulator continuation, 24 September 2026

See simulator-continuation.md for the Eval fix, signed HDL nodes, hierarchy,
signal history, real GitHub CI runs and the new isolated Linux build host.
The former narrow-negative mismatch is fixed. Full migration remains blocked.

## 2026-09-26: offline course bundle and release evidence

Added all 249 supplied project files, byte-identical to the baseline, including
folders 0–13. Qt embeds them on every GUI target. Files > Create course workspace
creates a unique editable copy through a worker; tests hash every copied file
and reject destination collisions. Two new independent demonstrations pass
Java/native byte comparisons. Four local Windows CTest suites pass, and the
differential suite now passes 67/67 cases. Original baseline: 881 unchanged files.

Eval CI 36221941023 passed the desktop and Android rerun checks. Publication
initially picked the older of two same-name Android artifacts. The publisher
now resolves the latest artifact by creation time and still requires the exact
APK hash and full interaction results. The first failure remains in evidence.

Refreshed stale Android/recovery instructions, marked historical reports, and
added an authentic Qt application screenshot capture path. New course-copy
Android interaction and cross-platform package checks are pending CI.

## 2026-09-28: demonstrations and script boundaries

Added independent BusSelect4 and RememberBit HDL/TST/CMP examples. The native
Qt capture runner produces four evaluated EntryAlarm states, a desktop screenshot
and a narrow-window screenshot. The GIF uses those frames without retouching.
All 249 original starter files remain unchanged. Documentation distinguishes
responsive desktop captures from actual Android verification.

Native output-list preflight now rejects more than 20 fields before output file
side effects. Six Java/native boundary comparisons pass, with only the exact
isolated script path normalized in rejection diagnostics. The VM starts SP at
256 as observed in Java and confirmed by CPU.boot(). Four CTest suites and
69 differential cases pass locally. Cross-platform 0.1.6 CI is pending; full
course support, OS fallback and other parity blockers remain incomplete.

## 2026-09-28: explicit offline Jack OS assets

Added the eight original tools/OS VM files, per-file hashes and their retained
attribution to the embedded course resources. More > Add supplied Jack OS
confirms the active document folder and adds only missing files, preserving
existing student implementations and the running simulation. The worker reports
partial failures and cancellation; users reload explicitly. Native service
fallback remains unfinished. Eight explicit-OS Java/native comparisons passed.

CI 36388335655 passed Linux, both macOS targets, both Android builds, Android
workspace interaction and sanitizers, but failed the Windows course-copy GUI
assertion. Publication was blocked. The prior three-second simulator wait was
used for hundreds of file writes; the follow-up test waits for bounded copy
completion and retains output/busy-state evidence to distinguish errors from
timeouts. The next revision must pass this gate before release.

The 0.1.7 run 36389621457 passed all six package builds, desktop deployed GUI
checks and sanitizers. Its first Android interaction attempt failed returning
from DocumentsUI to the import prompt. The toolbar safe-area checks passed.
The log contains Qt accessibility-related cross-thread object warnings; the
root cause remains unconfirmed. Publication was blocked and the same APK is
being rechecked in a failed-job-only rerun. The parity register retains the
intermittent lifecycle issue even if a later attempt succeeds.

Attempt 2 of the unchanged 0.1.7 Android APK failed earlier, waiting for the
More menu, before the folder picker. This does not confirm accessibility as the
root cause. Both attempts are retained; release publication remains blocked.
The Android harness now captures the latest Qt scene, foreground activity, window
and power dumps, and a screenshot on every interaction timeout, including failures
before workspace selection. The diagnostic-only change passes Python syntax
validation; its new capture path has not yet been exercised on an emulator.

## 2026-09-28: native bitmap authoring (0.1.8 candidate)

Inspected the official web IDE hardware, converter, compiler and bitmap surfaces.
Added More > Bitmap editor using a portable C++ pixel model and Qt Quick painted
canvas: drawing, resize, shift, flip, invert, square rotation, 32-operation undo
and redo, selectable Jack/ASM output and clipboard copy. The generated full-canvas
ASM is executed in core tests to verify screen addresses and LSB-first words;
Jack high-bit constants compile without out-of-range literals. GUI checks use a
real Qt mouse click to draw and verify generated code and undo/redo.

Windows Release build and all four CTest suites passed. All 26 maintained
Markdown files passed encoding/local-link checks. Other platform qualification
is pending CI. The web comparison, bitmap instructions and explicit missing
features are in `web-ide-parity.md`; the parity manifest and TODO remain open.
Android picker/menu stalls still block publication; no release is asserted here.

## 2026-09-28: multi-file Jack application workflow

Added Compile Jack folder in Converters & diagnostics and `build-folder` in the
native task console. Compilation runs off the GUI thread with open-buffer
snapshots. All sources compile before any output commits; the GUI-thread commit
checks dirty buffers again, including edits made during the worker operation.
External changes, cancellation, resource bounds and partial I/O errors are
reported explicitly. Original project and source files are not rewritten.

Windows Release: four CTest suites passed, including 350 GUI checks. New tests
cover unsaved dependencies, malformed source with unchanged output, concurrent
output edits, cancellation, the shared GUI action, and a multi-file application
compiled and executed through the supplied OS to a result of 42. Documentation
checks passed for 26 maintained Markdown files. Other targets need this commit's CI.

For preceding commit db07f13, Android API 36 x86-64 CI 36395135372 passed actual
toolbar taps, Downloads import/edit/save/assemble/export/reopen, composite Xor
Eval, and an offline course workspace/example. APK SHA-256:
`ac8084cd0e57b3e958ab810ec23ad11bc845ba96c1cbd0c7822655e5b58bf292`.
The emulator page size was 4096 bytes; this is not a 16 KB runtime test. Earlier
intermittent picker/menu failures remain unexplained and are not closed merely
because this run passed.

## Android rendering follow-up (0.1.9 candidate)

Local API 36 testing of the released 0.1.8 APK reproduced a black captured
surface, despite successful import/edit/build/export/reopen and Xor/course Eval
assertions. CI screenshots also show triangular rendering corruption. Therefore
the prior test success is not complete visual qualification. Added a Pillow
12.0.0 development-only image check rejecting blank app surfaces (system bars
excluded), with three unit tests and confirmation that the actual black capture
fails. This cannot detect every rendering defect; manual image review remains
required. Selected Qt Quick's software renderer on Android for the 0.1.9
candidate; device verification is pending. Simulation/compiler code is unchanged.
The UI has no ShaderEffect or particle dependencies. Qt documents the raster
renderer and its limitations in the [software adaptation guide](https://doc.qt.io/qt-6/qtquick-visualcanvas-adaptations-software.html).


CI 36397320058 confirmed the software renderer initialized, but its More-menu
capture was blank. The new gate correctly failed and blocked publication. The
override was withdrawn rather than shipped as a fix. The local emulator is now
retesting the unchanged 0.1.8 APK with modern SwiftShader to isolate the rendering
backend. Blank failures now retain Android logs and the last Qt state snapshot.

The unchanged 0.1.8 APK still showed triangle corruption with modern emulator
SwiftShader. A controlled launch using Qt's `extraenvvars` with
`QSG_RHI_BACKEND=vulkan` and `QSG_INFO=1` initialized QRhi Vulkan and produced a
clean workspace capture on the same emulator. The revised candidate probes
QVulkanInstance and a physical device before selecting Vulkan; devices without
one retain the platform default. The full controlled workflow and shipped
selection still require verification. This does not prove physical ARM64 parity.

## 0.1.9 verification and publication

CI 36399208565 and publication 36400223377 succeeded at b3485f4. Windows,
Linux, macOS ARM64/Intel and Android ARM64/x86-64 packages are published in
`development-36399208565`, together with source, manifest and checksums. Desktop
checks include four CTest suites and 350 GUI assertions. Sanitizers passed.
Android API 36 selected Vulkan automatically (graphics_api=5), passed the
workspace import/edit/save/assemble/export/reopen, safe-area taps and Xor/course
Eval checks. Manual inspection found clean menu/course images and visible
EntryAlarm alarm=1. The Xor image retained the earlier loaded frame despite the
backend assertion, so capture synchronization remains an explicit test gap.

The controlled unchanged-APK Vulkan test also passed locally. Real comparison
screenshots and reproduction instructions are in `android-rendering.md`. No
physical ARM64 or 16 KB runtime qualification is claimed. Baseline verification
reported 881 files, seven JARs and no differences. All 12 course rows and full
web IDE parity remain incomplete; see TODO.md and parity-manifest.json.

## Guided interface (0.1.10 candidate)

Added a system-font dropdown with preview and persisted selection, locally drawn
vector toolbar icons, and an 11-topic offline tutorial accessible through More
or F1. Compact Save/Build buttons retain accessible names. Windows checks now
include font selection/reload, topic navigation/content and portrait/landscape
dialog bounds: 377 GUI assertions and four CTest suites pass.

Added a workflow guide with Mermaid diagrams and test-driven screenshots/GIF.
Root LICENSE contains the existing GPL-3.0 text; README explicitly distinguishes
GPL-3.0-or-later native code from educational assets and third-party licenses.
No relicensing of original materials is intended. Full course/web parity and
physical-device qualification remain open. Cross-platform candidate CI pending.

The first candidate (dbcaea8, CI 36401617480) exposed a Linux 320-pixel/150%
scale dialog-button overflow. Replaced the icon row's extra spacing with explicit
content sizing; text-only buttons retain their original intrinsic width. The
correction a9bbd90 passes four Windows and Linux WSL CTest suites with 377 GUI
assertions. The pre-correction Windows portable package also passed its normal
interactive runner; hidden-window and missing-offscreen-plugin attempts failed
for test-environment reasons and are not platform passes.

Baseline verification found one external extracted-file difference in
`nand2tetris/projects/1/Xor.hdl`; no repair or overwrite was performed. The
original ZIP hash and immutable reference bytes remain unchanged.

0.1.10 published successfully: native CI 36402288659 and publication 36403339646
at a9bbd90. All four desktop packaged GUI jobs, both Android builds, sanitizers
and the Android interaction gate passed. The full-resolution Android Xor image
shows all toolbar icons and out=1. A suspected missing-icon issue in a resized
preview was disproved by the original image and pixel inspection. Speculative
local renderer changes were backed up and withdrawn; no 0.1.11 was published.
Tutorial/font-specific Android interactions and full course parity remain open.

## Return validation and practice circuits (0.1.11 candidate)

The shared Jack compiler now rejects missing non-void returns, value-bearing
void returns, incorrect constructor declaration types and constructor returns
other than literal `this`. Fourteen focused Java/native acceptance and output
contracts pass. Broader return-type restrictions are deliberately not inferred:
the supplied compiler accepts a boolean expression in an integer-returning routine.
Diagnostic envelopes and recovery remain different and are retained in evidence.

Added independent Parity3 and Majority3 circuits with exhaustive eight-row
truth tables, scripts and comparison files. Both outputs match Java byte for
byte. They are embedded alongside the original, unchanged 249 project files
and eight supplied OS files; student exercise implementations are not replaced.

Windows: all four CTest suites pass, including 383 GUI assertions. Unsaved
semantic errors in a Jack folder build preserve every existing VM output and
identify the failing source. The main differential suite passes 71/71 cases.
The broader bundled Jack audit remains 16/17: ExpressionLessSquare diagnostic,
recovery and output behavior still differs. This is an open compatibility
failure, not a completed project-10/11 parity claim. Cross-platform CI pending.

0.1.11 published at 167e940: CI 36405309943 and publication 36406261361 passed.
All four desktop packages pass 383 GUI assertions; both Android ABIs package,
and the API 36 workspace/backend Eval gate passes. Manual review confirms the
EntryAlarm image visibly shows alarm=1 with a safe toolbar. The Xor image again
captures stale initial values despite backend out=1: this remains an open
frame-synchronization/visual-evidence issue, not proof of correct displayed Xor.
The main repository guides and platform record include this limitation.
