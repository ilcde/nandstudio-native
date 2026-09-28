# Completion checklist

This is the implementation to-do list, not a claim of complete course support.
The detailed contract remains `parity-manifest.json`. Keep student starter files
unchanged; use separately identified regression fixtures for completed circuits.

## Demonstrations and starter resources

- [x] Embed all 249 unchanged original project files for offline editable copies.
- [x] Add independent EntryAlarm, SignalMismatch, BusSelect4 and RememberBit fixtures.
- [x] Compare all four example outputs against Java without modifying course files.
- [x] Capture actual application screenshots and a four-state Eval GIF; document reproduction.
- [x] Match the 20-field output-list boundary and VM initial stack pointer on Windows.
- [x] Pass six-target CI and development publication gates through 0.1.9 (36399208565); full platform qualification remains open.

## Immediate usability and release

- [x] Place Android Files/More menus below the status bar; real tap regression.
- [x] Publish 0.1.1 packages from successful CI 36154176790.
- [x] Implement explicit Android workspace import/export copies.
- [x] Pass Android import/edit/save/build/export/reopen interaction gate (CI 36208034521).
- [x] Compare the reported composite Xor with the official web IDE (0,1 -> 1).
- [x] Add one-bit tap inputs and explicit Eval result feedback.
- [x] Fix modified HDL detection when a non-HDL document becomes active.
- [x] Show live VM instruction, frame pointers, stack words and call stack.
- [x] Verify Eval on Android with the reported composite (CI 36208034521).
- [x] Publish and verify 0.1.2 packages from successful CI 36208034521.
- [x] Preserve input values on same-chip Eval reloads without pin arguments; clear stale success after parser failure or explicit Load (266 Windows GUI checks).
- [x] Verify follow-up Eval CI and publish from successful run 36209394882 (publication run 36209869977).
- [x] Shared desktop Eval tests passed on Windows, Linux, macOS ARM64 and Intel in CI 36211395573; Android Xor interaction also passed.
- [x] Implement Eval detection of changed, added and removed closed HDL dependency files; open buffers remain authoritative.
- [x] Verify shared closed-dependency Eval regressions in desktop CI 36221941023; Android Xor tap rerun passed with the same APK.
- [ ] Resolve emulator rendering artifacts and verify a physical ARM64 device.
- [x] Verify automatic Vulkan selection and readable menu/course captures on the API 36 emulator (36399208565).
- [ ] Synchronize screenshot capture with presented frames; the 0.1.9 Xor capture preceded the final state despite passing backend assertions.
- [ ] Configure a stable externally supplied release-signing key for updates.

## Advanced live UI and converters

- [x] Add a shared native bitmap editor with Jack/ASM generation, transforms and undo/redo; Windows tests.
- [ ] Complete [web IDE bitmap and converter parity](web-ide-parity.md), including image import, export modes, animation and keyboard access.
- [ ] Qualify bitmap drawing, clipboard and responsive layout on every packaged platform.

- [x] Native 16-bit decimal/binary/hex converter with signed and unsigned views.
- [x] Read-only ASM-to-Hack and Jack-to-VM previews from unsaved editor snapshots.
- [x] Optional debounced parser diagnostics of the active editor buffer.
- [x] Discard stale preview output, replace previous preview errors and navigate to diagnostics.
- [ ] Exercise converter UI and live diagnostics on Android devices and all desktop platforms.
- [x] Implement HDL/VM folder-aware preview validation without resetting simulation (Windows tests; platform qualification remains above).
- [x] Implement VM-to-ASM translation as a new utility; 11 unchanged project 7/8 CPU scripts pass in Java and native CPU engines.
- [x] Add native CPU/VM PC breakpoints, single-step bypass and live expression watches (Windows tests).
- [x] Add CPU instruction decoding, source navigation and configurable watches; edited-source mapping is explicitly invalidated (Windows tests).
- [ ] Add full breakpoint controls, richer waveform inspection and parser-backed completion.
- [ ] Persist debugger configuration; implement conditional/data-change breakpoints and HDL breakpoint parity.

## Course projects 1–12 coverage

- [x] Shared Jack folder build from GUI and console, including unsaved dependencies,
  compile-before-write, output conflict protection and cancellation (Windows tests).

Each row requires native implementation, reachable UI/console actions, reference
comparison, and exercised desktop/Android workflows before it can be checked.

- [ ] 1: All Boolean gates, hierarchy, pin editing, Eval, complete HDL/script errors.
- [ ] 2: Bus wiring, arithmetic circuits, ALU controls and comparison workflows.
- [ ] 3: Sequential timing, memories, tick/tock/reset, all component inspectors.
- [ ] 4: ASM/CPU programs, screen/keyboard, breakpoints and execution controls.
- [ ] 5: Composite CPU/Computer, ROM/RAM loading, visualizations and clock traces.
- [ ] 6: Full assembler syntax/diagnostics and incremental interactive translation.
- [ ] 7: VM segments/arithmetic and debugging; preserve VM translator coursework.
- [ ] 8: VM branches/calls/recursion/statics, call-stack inspection and OS selection.
- [ ] 9: Complete Jack application edit/compile/run, drawing and keyboard workflows.
- [ ] 10: Parser coverage and diagnostics; classify XML analyzer as an addition if absent in baseline.
- [ ] 11: Compiler byte parity across programs and negative cases; directory builds.
- [ ] 12: All native OS services, allocation/string/drawing/text/input/time/error behavior,
  user VM precedence and confirmed fallback. Explicit OS VM execution is distinct
  from built-in service compatibility.

## Shared release blockers

- [ ] All legacy test-script commands, debugger operations and exact formatting.
- [ ] Legacy Java extension binary strategy and tested native extension interface.
- [ ] Android provider errors, grants, interrupted transfer recovery, IME and lifecycle.
- [ ] Complete editor navigation/completion using parsed project symbols.
- [ ] Original CLI no-argument launch behavior, diagnostics and platform path cases.
- [ ] Offline OS/resource licensing, full Qt third-party notices and source obligations.
- [ ] Install/launch/workflow tests on all declared platforms; Android 16 KB runtime.
- [ ] Full baseline differential coverage, negative parser tests, fuzz/resource tests.

## Reference notes

The [official web IDE](https://nand2tetris.github.io/web-ide/chip/) was exercised
with the user's composite Xor: pin b toggled from 0 to 1, Eval enabled, Eval
changed out from 0 to 1 and updated internal pins. Its presentation informs the
tap controls; the uploaded desktop suite still governs legacy syntax and lookup.
Native local starter dependencies are not silently replaced by solved built-ins.

`scripts/probe_course_os.py` executes the original Seven project compiled by the
native compiler with all eight explicit original OS VM files. It compares RAM
and rendered glyph words against Java after two million VM steps. This exposed
uppercase hexadecimal script output; the baseline uses lowercase. Output bytes
must be fixed, not normalized in the harness.

The OS probe now also passes the original ArrayTest, MathTest, MemoryTest and
MemoryDiag scripts with their unchanged comparison files. StringTest, OutputTest
and ScreenTest compare all 8,192 screen words in 16-column records after two
million VM steps. All eight cases pass with explicit bundled OS VM files. This
does not establish native built-in service fallback or interactive keyboard parity.
The probe also found that legacy output-list declarations accept at most 20
arguments. Native preflight now matches this boundary; six reference cases pass.

## Offline starters and documentation

- [x] Embed all 249 unchanged original project files; preserve per-file hashes.
- [x] Add an explicit, non-overwriting editable course-copy action.
- [x] Add two independent HDL/TST/CMP demonstrations; Java/native output bytes match.
- [x] Exercise course-copy UI and EntryAlarm Eval on Android; verify packaged desktop resources in CI 36225896175.
- [x] Review maintained guides for stale instructions, spelling/encoding problems and local links; add a mechanical UTF-8/link check. Historical evidence remains labelled and preserved.
- [x] Add a real application screenshot and accessible demo caption; animated demos remain optional.

## Newly confirmed compatibility defect

- [ ] Match Java semantic rejection and output-file behavior for the untouched
  project 10 ExpressionLessSquare fixture. The full bundled Jack audit is 16/17,
  with the mismatch retained in `evidence/bundled-jack.json`.

- [x] Reproduce the Windows Xor/empty-dependency result in Java and native engines.
- [x] Repeat dependency warnings at every Eval and add direct navigation to unfinished chips.
- [x] Qualify the 0.1.5 warning/navigation UI in CI 36231390783; physical-device coverage remains open.

## Offline OS integration

- [x] Bundle the eight original OS VM files with hashes and retained attribution.
- [x] Add an explicit missing-files-only installation action and destination confirmation.
- [x] Verify bytes, existing student-file preservation, cancellation and repeated installation locally.
- [x] Pass supplied-OS desktop checks on all four desktop targets (36399208565).
- [ ] Exercise the supplied-OS installation action on Android devices.
- [ ] Implement and verify native built-in service selection and fallback; copied VM assets do not close that gap.

## Android picker return investigation

- [ ] Resolve or characterize the picker-return stall in CI 36389621457 attempt 1.
  Safe-area controls passed, but the import confirmation did not appear after the
  tree grant. Qt logged wrong-thread QObject parenting during accessibility
  activation. This is evidence, not a proven root cause. Retain the failed attempt;
  rerun the unchanged APK without disabling accessibility or skipping assertions.

## Guided interface

- [x] Replace free-text font entry with installed-font choices and a live preview.
- [x] Add vector toolbar icons with retained accessible names.
- [x] Bundle an offline tutorial for supported workflows, accessible from More and F1.
- [x] Put the existing GPL text in root LICENSE and distinguish asset licenses in README.
- [ ] Qualify the new tutorial and font picker on all packaged targets and Android devices.

## Return compatibility and additional examples

- [x] Match 14 constructor/void/non-void return acceptance and generated-byte cases.
- [ ] Match complete compiler diagnostics, error recovery, output retention and CLI results.
- [x] Add independent Parity3 and Majority3 HDL/TST/CMP examples; eight-case truth tables match Java.
- [ ] Exercise these additions on all packaged platforms and Android devices.
