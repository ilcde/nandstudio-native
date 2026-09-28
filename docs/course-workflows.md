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
| 7 | VM stack arithmetic | VM execution and added VM-to-ASM translator; segment/arithmetic CPU tests | Full debugger/diagnostic parity |
| 8 | VM program control | Branch/call/return/static translation and execution; PC breakpoints, watches, call-stack and frame views | Conditional breakpoints, OS fallback, loader edge cases |
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
`.vm`; choose Compile Jack folder in Converters & diagnostics or use `build-folder`
in the console for multi-file applications, including unsaved editor buffers.
`JackCompiler DIRECTORY` remains available through the packaged desktop CLI. Load VM reads sibling VM files and overlays open buffers. Use **More > Add supplied Jack OS** to copy missing original OS VM files
into the active application folder. Existing implementations are preserved.
Native built-in fallback remains incomplete. Saved test scripts use
the matching CPU/VM action. The in-app command console is not an external shell.

Students implement VM translators and syntax analyzers in the course; these are
not automatically tools shipped by the original desktop suite. New utilities
must be identified as additions rather than claimed as preserved functionality.

## Evidence and blockers

Reference tests include 71 differential cases, two reported-Xor cases and targeted
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

## Complete bundled Jack inventory check

`scripts/probe_bundled_jack.py` checks all 17 supplied application folders in
projects 9–11. Sixteen match Java-generated VM files byte for byte.
`10/ExpressionLessSquare` is a syntax-analysis fixture: the Java compiler rejects
its constructor returns and assignment types, while the native compiler currently
accepts them. This is a recorded compiler parity defect, not an exercise to edit
until it passes. See `evidence/bundled-jack.json` and the parity manifest.

## Independent practice demonstrations

The editable course copy includes four separate HDL/TST/CMP examples.
[View the screenshots, Eval animation and instructions](demos.md). BusSelect4
exercises sub-buses; RememberBit exercises clocked capture and hold. They are
not replacements for the unchanged assignments. All four match Java outputs.

## Jack return diagnostics

A constructor must declare its own class as the return type and return literal
`this`. A `void` routine uses `return;`; other return types require an expression.
These checks reproduce the supplied compiler's acceptance for the focused cases
in [the return-contract evidence](evidence/jack-return-contract.json). The native
compiler does not yet reproduce every legacy diagnostic, recovery rule or type
check. In particular, the syntax-analysis fixture `ExpressionLessSquare` is not
a valid compilation success case, and its rejection still differs from Java.

The GUI compiles a complete folder snapshot before writing any generated files.
If a routine contains an invalid return, the diagnostic identifies its document
and the previous VM outputs remain intact. Correct the source and build again;
a successful earlier output does not mean the invalid edit was compiled.

For additional hardware practice, create a course workspace and open
`examples/Parity3.tst` or `examples/Majority3.tst`. Run **HDL test** to check all
eight input combinations, or open the matching HDL file and edit pins manually.
Parity3 is true for an odd number of true inputs; Majority3 is true when at least
two inputs are true. These independent circuits supplement the original starters.
