# Xor evaluation, editor search and Android header

The supplied Xor definition is valid. Both native and uploaded Java engines
produce 0, 1, 1, 0 for inputs 00, 01, 10, 11 when its dependencies are built-ins.
With the bundled unfinished `Not.hdl`, `And.hdl`, and `Or.hdl` beside it, both
engines produce zeros. Local files intentionally override built-ins. These
student files have not been changed or completed to make tests pass.

New behavior:

- Load & Eval HDL loads the current buffer. After an HDL buffer or open dependency
  changes, the button explicitly becomes Reload & Eval and explains that state
  and clock reset. Ordinary evaluation retains the loaded state. Edits alone
  never reload the simulator. Build also accepts HDL and loads its snapshot.
- HDL parse failures report file/line/column and preserve the last valid chip.
  Empty hierarchical dependencies are named in a visible warning. An incomplete
  chip remains valid legacy input; the warning does not alter its semantics.
- Find and replace is hidden until requested through More or Ctrl+F. The editor
  binding follows delegate creation/tab changes. Next, Replace and All provide
  feedback; replacement uses C++ document edits and one undo transaction.
- Header padding includes Qt's reported safe top/left/right margins. Content
  padding remains managed by ApplicationWindow. This addresses controls beneath
  Android status bars and cutouts.

Local Windows: 2,012 core, 2,144 hardware and 244 GUI checks pass. The two Xor
Java/native cases match output bytes (`docs/evidence/xor-eval.json`). CI and
device checks for this revision must be read from its release provenance; no
full Android workflow or full course-suite parity claim is made here.

Device check entry point (development APK):

```sh
adb shell am start -W -n org.qtproject.example.NandStudio/org.qtproject.qt.android.bindings.QtActivity --ez nandstudio.layoutCheck true
adb shell run-as org.qtproject.example.NandStudio cat files/layout-report.json
```

The report records actual window insets and all four toolbar rectangles, and
checks that each fits within the safe area. Desktop equivalent:
`NandStudio --layout-report /absolute/path/report.json`.

## Course folders inspected

The workspace's actual `nand2tetris/tools` and `nand2tetris/projects` were inspected
again. Tools include HardwareSimulator, CPUEmulator, VMEmulator, Assembler,
JackCompiler, TextComparer, built-in chips/VM code and compiled OS assets.
Projects contain 0 through 13, including all requested 1 through 12.

| Projects | Workflow | Remaining parity work |
|---|---|---|
| 1-3 | HDL gates, arithmetic, sequential circuits and tests | Complete original GUI/script edge cases and extension compatibility |
| 4-6 | Machine programs, computer hardware and assembly | Complete interactive translation, debugger and visualization operations |
| 7-8 | VM execution and course-written VM translators | Full native OS fallback, debugger/call-stack UI; no bundled standalone translator assumed |
| 9-11 | Jack applications and compiler coursework | Full diagnostics/output-mode audit; syntax-analyzer output is not presumed bundled |
| 12 | Jack OS implementations and OS tests | Native built-in VM services/fallback and complete OS behavior tests |

Android SAF directory/document grants, import/export, and lifecycle workflows
remain release-blocking across these projects. See the machine-readable parity
manifest for the broader outstanding inventory. A passing Xor fixture does not
establish completion of all course tools.
