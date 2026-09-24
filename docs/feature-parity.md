# Feature parity â€” release blocked

This is a working native development increment, **not a completed migration**.
No tool or platform is certified to have full parity. The machine-readable
[`parity-manifest.json`](parity-manifest.json) is deliberately conservative: every
legacy feature remains release-blocking until implementation, access paths and
coverage are complete. The initial inventory is not claimed exhaustive.

| Area | Native implementation | Evidence and outstanding work |
|---|---|---|
| Assembler | C++ grammar, symbols, compact addressing, binary output; GUI build | Matrix differential tests; incremental interactive controls and exact invalid-input behavior pending |
| CPU Emulator | C++ state, ALU, RAM/ROM, reset, steps; Qt screen and inspector | Instruction trace tests; full controls, numeric modes, breakpoints and keyboard tests pending |
| VM Emulator | All command categories, segments, branches, calls/returns, statics | Bundled differential cases; load ordering, bounds, bootstrap/fallback, call-stack UI and diagnostics incomplete |
| Jack Compiler | Tokenizer, recursive parser, symbol scopes, VM generation | Byte equality for tested bundled programs; type checking, warnings, cross-file validation and negative diagnostics incomplete |
| TextComparer | Native comparison and CLI | Seven differential cases; non-ASCII/default-encoding compatibility incomplete |
| Test scripts | CPU/VM/HDL load/set/step/loops/output/compare subset | Bundled cases; 36 hardware differential cases; breakpoints, stop semantics and full grammar pending |
| Hardware Simulator | C++ HDL parser, hierarchical net graph, 35 embedded declarations, built-ins, clocks, pins/components/memory and ROM controls | 29 bundled + 7 independent reference cases; 23 passing narrow-negative probes and remaining scheduling/GUI gaps in hardware.md |
| Jack OS | Original VM assets preserved | Native service implementations, fallback confirmation/precedence and bundled offline assets pending |
| Extensions | Source APIs audited initially | Java binary compatibility, native interface, callbacks and GUI extensions unresolved |
| Editor | Native Qt Quick editor and C++ documents/services | Multi-document editing, save conflicts, recovery, snapshot builds; remaining conveniences and edge cases in manifest |
| Android | Shared core compiles in CMake design; responsive QML exists | No Android build produced. SAF, grants, lifecycle, import/export and device workflow missing |

Actual results are in `evidence/differential.json`, `build/gui-test/checks.json`,
and `platforms.json`. Phone-width rendering on Windows is **not Android testing**.

Known differences include native diagnostics/exit behavior, CLI no-argument GUI
dispatch, legacy segment limits, VM missing-return
checks, incomplete script string variables and unverified unusual HDL scheduling. Partial output persistence on script errors now has a rejection regression.
No broad whitespace normalization is used to hide generated-output mismatches.
No project starter files or expected comparison outputs were changed.

`scripts/parity_inventory.py` records individual bundled chips and native OS
service requirements and extracts original GUI strings with source locations.
Those GUI evidence rows still require manual operation mapping and regression
coverage. New editor conveniences are marked `addition`, not preserved legacy
features. A standalone VM translator or Jack XML analyzer is not represented as
a migrated feature.

The editor continuation adds shared responsive controls, C++ editing helpers and
filesystem workspace/conflict services. See continuation-report.md and
evidence/gui-continuation.json for 159 Windows checks. These additions remain
partial in the manifest because cross-platform and edge-case coverage is incomplete.
