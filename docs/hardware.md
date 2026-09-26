# Native hardware increment — remaining parity blockers

`src/hdl.cpp` provides the first native hardware workflow. It does not invoke
Java. A C++ resolver supplies project-local HDL text; absent names fall back to
35 declarations embedded at CMake configuration time from `resources/hdl`.
Those files are unchanged copies of the uploaded built-in declarations, not
solutions substituted into student projects. Original notices are retained.

The parser constructs a hierarchical circuit, a flattened dependency graph of bus
bits, and directed short-valued pin nodes with slice adapters. It checks pin widths/directions, driven internal
wires, conflicting outputs, sub-bus ranges, recursive dependencies and
combinational cycles. Internal wires cannot be subscripted. Local definitions
take precedence over embedded declarations. Built-in pin roles use declaration
order, matching Java's indexed inputs/outputs. Malformed loads build separately
and cannot destroy the previous circuit. Copies own independent memory/state.

Implemented built-ins include gates, mux/demux, adders, ALU, Bit/DFF/Register,
ARegister/DRegister/PC, every bundled RAM size, Screen, ROM32K and Keyboard.
Tick samples sequential state; tock publishes it. RAM writes occur during tick
and reads respond to address changes. PC priority is reset, load, increment.
The special wire is named `clk`, initially 1, becoming 0 on tick and 1 on tock.
Hardware time is a string with `+` during the high phase. Scripts cannot write
Keyboard; focused GUI key events update it. ROM commands use `ROM32K load FILE`.

## Access and evidence

| Legacy operation | Native access | Evidence |
|---|---|---|
| Load/reload HDL | Load HDL button; `load-hdl`; script `load` | GUI reload preservation, local dependency and positional built-in fixtures |
| Edit input pins, evaluate | Input rows, atomic commit on Eval or Return; `set-pin`, `eval` | GUI pin editing; project 1/2 reference outputs |
| Tick/tock | Separate buttons and commands; Step/Run clock batches | GUI tick/tock; project 3 and independent DFF/feedback/RAM timing traces |
| Inspect pins and components | Root pins/internal wires; component picker and pins | C++ state snapshots, unit tests; full visual behavior not yet tested |
| Edit memory | Component address/value controls; script `set RAM8[3] ...` | Independent memory timing reference fixture |
| Load ROM | Selected ROM button consumes visible ASM/HACK buffer; script command | Combined Devices reference fixture |
| Screen/keyboard | Shared 512×256 view, focused keys and touch buttons | Screen memory/ROM reference fixture; read-only keyboard rejection; native key unit test |
| Run test script | HardwareSimulator executable, HDL test button, `test-hdl` | 36 hardware reference cases (one expected rejection) |

The differential harness also retains all 29 earlier assembler/CPU/VM/compiler/
comparison cases. Its 67 matching cases are not an exhaustive parity claim.
Original project 1–3 tests run in isolated directories against built-ins; the
student HDL stubs remain unchanged. Independent circuits are under
`tests/fixtures/hardware`. Byte comparison includes whitespace and CRLF.

## Confirmed difference and unverified behavior

* The former narrow-negative mismatch is fixed. `set in -1` on Not now retains input -1/output 2. Whole-pin short storage and explicit slice adapters pass 23 byte-exact uploaded-Java probes in `evidence/hardware-known-differences.json`. These probes supplement, rather than replace, the existing 65 cases.
* Flattening has only limited hierarchical timing coverage. Nonstandard CLOCKED
  declarations, clocked outputs, absent CLOCKED declarations, fanout involving
  several sequential hierarchy levels, repeated input connections and unusual
  built-in signatures need further differential tests. They are not certified.
* Full legacy parser diagnostics, pin declaration grammar corner cases,
  default-directory behavior and native extension loading are unfinished.
  Negative indices are handled safely; this is not evidence of matching legacy
  failures. Built-in register and memory GUI edit validation is not fully mapped.
* All original display modes, animation/speed controls, breakpoints, script
  stepping, stop punctuation, ALU expression visualization, screen interaction,
  keyboard mappings and hierarchy navigation are not yet fully implemented or
  covered. Component selection currently exposes primitive built-ins.
* HDL load diagnostics navigate the editor to dependency source locations and
  failed loads preserve the last valid chip. Script file I/O remains local-path based; Android
  SAF, lifecycle and on-device hardware testing are still blocked.

Bounds: 32 MiB source, one million tokens, dependency depth 128, two million
bit nodes, 100,000 primitive devices, 16 million memory words. Cancellation is
checked between dependency instantiations and clock cycles. These bounds are
explicit safety limits whose valid-large-project compatibility needs review.

Primary source evidence: `Hack/Gates/{CompositeGateClass,CompositeGate,Gate}.java`,
`Hack/HardwareSimulator/HardwareSimulator.java`, `BuiltInChipsSource/*.java`, and
`Hack/Utilities/Conversions.java` in the recorded upstream tree. Observed uploaded
binary behavior takes precedence over those sources.

## Explicit action history

The inspector includes a Clock cycle action and bounded history for the last 256
explicit Load/Eval/Tick/Tock/Cycle actions. Select a root or internal signal and
decimal, hexadecimal or binary formatting. The plot distinguishes zero/nonzero;
rows preserve exact values and clock phase. This addition does not implement the
remaining schematic, multi-signal, component-signal or batch tracing requirements.
