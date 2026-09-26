# Completion checklist

This is the implementation to-do list, not a claim of complete course support.
The detailed contract remains `parity-manifest.json`. Keep student starter files
unchanged; use separately identified regression fixtures for completed circuits.

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
- [ ] Verify the new closed-dependency Eval regressions across platform CI.
- [ ] Resolve emulator rendering artifacts and verify a physical ARM64 device.
- [ ] Configure a stable externally supplied release-signing key for updates.

## Advanced live UI and converters

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
arguments, while the native parser currently accepts more; track that parser gap.
