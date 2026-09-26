# NandStudio user guide

NandStudio is a development preview. It runs native tools locally on Windows,
macOS, Linux and Android. Read the [installation guide](install.md) first and
check [course coverage](course-workflows.md) before relying on an unverified
workflow. No Java runtime is required by the application.

## Create or open a workspace

Choose **Files > Create course workspace** to create a new editable local copy
of every supplied course project, folders 0–13. This works offline. The original
starter files remain unchanged and exercises remain intentionally incomplete.
The separate `examples` folder contains new EntryAlarm and SignalMismatch
HDL circuits with matching test scripts and comparison files.

Each invocation creates a separate folder and preserves previous work. Choose
**Files > Open workspace** for an existing desktop folder. On Android, select a
provider folder and confirm **Import copy**; edits then affect the local copy.
See [Android workspace copies](android-workspaces.md) for storage and grant limits.
Use **Files > Export workspace copy** to export saved work into a new folder.
Export important Android work before uninstalling: app-private copies may be lost.

Files lists recent workspaces. Use **New file** or **Folder** in the explorer to
create entries. A nested entry needs an existing parent folder. The `...` menu
provides rename and **Move to recovery trash**. Save or close modified documents
before moving them. Removed files remain in `.nandstudio-trash/<id>/` with an
`original-path.txt` record; restore them with ordinary file operations. An in-app
trash browser is not implemented.

## Edit and search

Open a file from the explorer. Tabs retain documents; a dot marks unsaved changes.
Native Qt editing provides selection, clipboard operations and undo/redo.

| Action | Shortcut or location |
|---|---|
| Save | Ctrl+S or toolbar Save |
| Save all | Files > Save all |
| Find and replace | Ctrl+F or More > Find and replace |
| Go to line | Ctrl+G |
| Build active document | Ctrl+B or toolbar Build |
| Toggle line comments | Ctrl+/ |
| Indent / unindent selection | Tab / Shift+Tab |

Return carries indentation. Bracket matching ignores comments and strings.
Find supports case and whole-word matching, reports missing matches, and groups
Replace All into one undo operation. Close hides the search bar. Project search
runs in a worker and includes unsaved open buffers; select a result to navigate.

**More > Settings** controls font size, wrapping, indentation, interface scale,
theme and optional autosave. More also displays the application version. On narrow
screens, Files, Editor, Machine and Console are separate panes. Desktop splitters
retain their sizes. The screen receives simulated keyboard input only when focused;
typing in the editor does not send keys to the simulated computer.

## Build visible source

Build uses a clearly identified snapshot of the visible buffer, including unsaved
text. It compiles ASM to a sibling `.hack`, Jack to a sibling `.vm`, translates a
single VM file to `.asm`, or loads HDL. Source files are not implicitly saved.
Generated artifacts appear in the app. Builds may replace earlier generated files;
dirty open output buffers block the operation. Read the console for the exact action.

For directory Jack compilation, the desktop command is `JackCompiler DIRECTORY`.
The in-app task console invokes native services; it is not an external shell.

## Hardware Simulator and Eval

1. Open an HDL file and choose Build or Load HDL.
2. In Machine, type input values or tap a one-bit input's 0/1 button.
3. Press Eval and read the result above the pins. The independent EntryAlarm
   example produces `alarm=1` for `enabled=1`, `door=1`, `window=0`.
4. Use Tick and Tock for sequential circuits according to their specification.

Eval uses the loaded circuit. After edits, Reload & Eval loads the visible HDL
snapshot, retains same-chip inputs and resets internal clock/state. Explicit Eval
also detects changed, added or removed closed HDL dependencies in the chip folder.
Open documents remain authoritative. Ordinary combinational Eval does not advance
the clock. Editing alone does not reset the simulation.

A parser failure retains the last valid chip and shows an error instead of a stale
success message. Explicit Load clears the old Eval result. Stop a running batch
before evaluating again. Select components to inspect their pins and memory.

Project-local HDL takes precedence over built-ins, including incomplete student
Not/And/Or files. A warning names empty dependencies; the app does not substitute
assignment solutions. Windows, macOS, Linux and Android share this implementation.
See [Eval troubleshooting](eval-troubleshooting.md) and [hardware coverage](hardware.md).

## CPU and VM execution

Load CPU reads the visible ASM or Hack buffer. Step executes one instruction;
Run 10k executes a bounded worker batch. Stop requests cancellation. Inspect or
edit RAM by address. Load VM reads sibling VM files and overlays open buffers.
The VM view exposes the current command, stack, frame pointers and call stack.
Supply required project OS VM implementations: complete native fallback is pending.

The Live debugger supports separate CPU and VM PC breakpoints. Run stops before
executing a breakpoint; Step bypasses it for one instruction. Watches show live
values or explicit errors. For an unchanged open ASM source loaded into the CPU,
**Go to current ASM instruction** navigates to the decoded instruction. Editing
or closing that source disables stale mappings. Conditional/data-change and HDL
breakpoints, and debugger persistence, remain incomplete.

## Run test scripts

Open a saved `.tst` and choose CPU test, VM test or HDL test. Every open document
must be saved first. Scripts run against saved files, write their declared `.out`
files and use their `.cmp` references. Do not change expected comparison files to
make a failing test pass. Interactive and command-line tests share the native engine;
complete script compatibility is still being audited.

The console accepts `build`, `load-cpu`, `load-vm`, `step COUNT`, `reset`, `stop`,
`test-cpu`, `test-vm`, `load-hdl`, `test-hdl`, `eval`, `tick`, `tock`, and
`set-pin NAME VALUE`, plus the converter commands below.

## Converters and live diagnostics

Open **More > Converters & diagnostics**. The word converter accepts decimal,
binary or hexadecimal values from -32768 to 65535 and shows the same 16 bits
as signed, unsigned, binary and hexadecimal numbers.

**Check & preview** converts ASM to Hack, Jack to VM, or a VM folder to ASM;
it validates Hack and HDL. VM/HDL dependencies overlay visible editor buffers.
Preview writes no files and does not reset a running simulator. Optional Live
checks run after 600 ms of idle editing, even after closing the panel. Reopen the
panel to disable them. Editing invalidates stale results. Diagnostics appear in
Problems and provide file/line/column navigation. Preview input is limited to
1 MiB; on-disk folder dependencies are limited to 4 MiB. Request a new preview
after external file changes.

For a VM application, open a VM file and choose **Translate VM folder to ASM file**.
Enable bootstrap to initialize SP=256 and call Sys.init; disable it for course tests
that initialize state themselves. Every called function needs an implementation
in the folder. Build on a VM tab translates only that file. Desktop commands:

```sh
nand VMTranslator DIRECTORY --bootstrap
nand VMTranslator FILE.vm
```

Output is `DIRECTORY/DIRECTORY.asm` or `FILE.asm`. The in-app console provides
`preview [--bootstrap]` and `translate-vm [--bootstrap]` for the active VM folder,
including on Android. The translator is a new utility, not a migrated legacy tool.

## Recovery and conflicting saves

Recovery journals open buffers separately from project files. Relaunch restores
documents, reports recovered buffers and leaves simulation stopped. Edited text
is journaled after 250 ms idle; active-document changes and backgrounding also
save session state. This does not guarantee recovery of every interrupted keystroke.

External changes block Save and open a conflict dialog. Keep editing, reload the
reviewed disk version, or explicitly overwrite it. If the disk changes again,
overwrite stops for another review. Optional document autosave is separate from
recovery, skips known conflicts and uses normal save checks. No automatic merge
occurs. UTF-8 BOM, dominant newline style and final-newline state are preserved;
mixed-newline editing remains a documented limitation.

The [parity register](parity-manifest.json), [platform evidence](platforms.json)
and [implementation checklist](TODO.md) distinguish implemented features from
unverified workflows and known differences.
