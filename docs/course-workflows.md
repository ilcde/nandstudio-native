# Course projects 1–12

This is a tool-coverage guide, not completed coursework. Numbers follow the
bundled project folders; course week schedules can differ. Exercises and reference
comparisons remain unchanged. Full original tool parity is not yet achieved.
The project inventory follows the [official course](https://www.nand2tetris.org/course).

| Project | Course work | Current workflow | Remaining work |
|---|---|---|---|
| 1 | Boolean logic | HDL load/eval, pins, saved tests | Complete parser/script/GUI edge cases |
| 2 | Boolean arithmetic | Composition, buses and arithmetic tests | Full ALU visualizations and compatibility |
| 3 | Sequential logic | Tick/tock, register/RAM state and tests | Unusual hierarchical timing and debugger controls |
| 4 | Machine language | Assembly, CPU step/run, RAM/screen/keyboard | Full breakpoint, animation and ROM editor parity |
| 5 | Computer architecture | Hierarchical HDL, built-in memories/ROM | All chip visualizations and interactions |
| 6 | Assembler | Native ASM-to-Hack and named CLI | Incremental translation and full diagnostics |
| 7 | VM stack arithmetic | Native VM commands and VME tests | All debugger/diagnostic operations |
| 8 | VM program control | Branch/call/return/static execution; live call-stack, frame-pointer and instruction views | Full debugger controls, OS fallback, loader edge cases |
| 9 | High-level language | Jack editing/compilation and VM with OS files | Complete application/device workflows |
| 10 | Syntax analysis | Jack editing/compilation | Standalone XML analyzer is not claimed as a bundled legacy utility |
| 11 | Compiler | File/directory Jack compilation; byte comparisons | Complete semantic/diagnostic/output-mode audit |
| 12 | Operating system | Preserved compiled VM assets; local VM services | Native built-ins, selection/fallback and full OS tests |

## HDL workflow

Open a folder, implement the assigned chip and its required earlier chips, then
choose Load & Eval HDL. Edit pins and Eval. After source changes, Reload & Eval
loads the visible buffers and resets state. Sequential chips use Tick/Tock.
Run a saved `.tst` with HDL test and inspect the `.out`/comparison result.

The reported composite Xor matches the original Java engine: working dependencies
produce `0, 1, 1, 0` for `00, 01, 10, 11`. Empty local Not/And/Or starters produce
zero in both engines. Local files override built-ins; the app warns instead of
substituting answers. Regression fixtures are separate from student exercises.

## Assembly, VM and Jack workflow

Build visible `.asm` to generate `.hack`, then Load CPU. Build `.jack` to generate
`.vm`; use `JackCompiler DIRECTORY` for directory compilation through the packaged
desktop CLI. Load VM reads sibling VM files and overlays open buffers. Supply
required OS VM files until native fallback is complete. Saved test scripts use
the matching CPU/VM action. The in-app command console is not an external shell.

Students implement VM translators and syntax analyzers in the course; these are
not automatically tools shipped by the original desktop suite. New utilities
must be identified as additions rather than claimed as preserved functionality.

## Evidence and blockers

Reference tests include 65 differential cases, two reported-Xor cases and targeted
hardware probes. They do not exhaust all programs or UI operations. Read the
release manifest, linked CI run, `evidence/` and `parity-manifest.json` for scope.

`scripts/probe_course_os.py` additionally passes eight native-versus-Java cases
with explicit original OS VM files: Seven, Array, Math, Memory, MemoryDiag,
String, Output and Screen. The latter three compare every screen word after a
fixed execution budget; the four original scripted OS tests retain their `.cmp`
files. Native built-in OS fallback and interactive keyboard tests remain pending.

Android shares the C++ engines. Version 0.1.2 adds explicit workspace copies;
broader SAF provider and lifecycle coverage still blocks a fully verified
open/edit/save/test/run/export workflow. Legacy Java extension
binaries and other original capabilities also remain release blockers. Course-wide
completion requires implemented, reachable features and platform workflow tests.
