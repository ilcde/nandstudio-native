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





## Editor continuation â€” 24 September 2026



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
