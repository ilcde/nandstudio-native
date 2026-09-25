# Completion checklist

This is the implementation to-do list, not a claim of complete course support.
The detailed contract remains `parity-manifest.json`. Keep student starter files
unchanged; use separately identified regression fixtures for completed circuits.

## Immediate usability and release

- [x] Place Android Files/More menus below the status bar; real tap regression.
- [x] Publish 0.1.1 packages from successful CI 36154176790.
- [x] Implement explicit Android workspace import/export copies.
- [ ] Pass Android import/edit/save/build/export/reopen interaction gate.
- [x] Compare the reported composite Xor with the official web IDE (0,1 -> 1).
- [x] Add one-bit tap inputs and explicit Eval result feedback.
- [x] Fix modified HDL detection when a non-HDL document becomes active.
- [ ] Verify Eval on Android with the reported composite and publish new APKs.
- [ ] Resolve emulator rendering artifacts and verify a physical ARM64 device.
- [ ] Configure a stable externally supplied release-signing key for updates.

## Course projects 1–12

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
