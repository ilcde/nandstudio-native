# Using this development build

Start `NandStudio.exe`. Open a folder, then a supported text file. Tabs retain
documents; a dot marks unsaved changes. Save uses Ctrl+S. Find uses Ctrl+F,
go-to-line Ctrl+G, build Ctrl+B. Ctrl+/ toggles a line comment. Tab and Shift+Tab indent/unindent selected lines using the configured width or tabs.
Return carries indentation; bracket matching ignores comments and strings. Native Qt editing supplies undo/redo, selection and clipboard operations.
Font size, wrapping, indentation, interface scale, theme and autosave are under More > Settings.
More also displays the application version. Version 0.1.1 anchors Files/More menus
below the toolbar and disables touch hover tooltips that could obstruct actions.
Version 0.1.2 adds explicit [Android workspace copies](android-workspaces.md).
For HDL, tap a one-bit input's 0/1 button or type a decimal value, then Eval.
The result line shows evaluated output values. On a narrow screen, loading HDL
opens the Machine pane. Unsaved HDL is detected even with a non-HDL tab active.
The editor gutter displays line numbers. Open find/replace through More > Find and
replace or Ctrl+F; Close hides it again. It supports case and whole-word matching;
Replace All is a single undo operation, and the bar reports missing matches.

For `.asm` or `.jack`, Build compiles the **visible buffer snapshot**, including
unsaved text, and creates a sibling `.hack` or `.vm` artifact. This is logged in
the task console. Generated files can replace prior generated outputs; dirty
open output buffers block the build. Source files are not implicitly saved.

Load CPU reads the visible `.asm` or `.hack` buffer. Step executes one instruction;
Run 10k executes a bounded batch on a worker. Stop requests cancellation. RAM is
inspectable/editable by address. Focus the screen to send simulated keys; editor
typing is separate. Load VM reads sibling VM files and overlays open buffers.
Unimplemented OS functions are reported as errors; supply project VM services
explicitly. These are development capabilities, not full emulator parity.

For a saved `.tst`, CPU test, VM test or HDL test invokes the native script runner. Every
open document must be saved first. Use copies of projects when experimenting:
test scripts specify their own output filenames.

The console directly accepts `build`, `load-cpu`, `load-vm`, `step COUNT`, `reset`,
`stop`, `test-cpu`, `test-vm`, `load-hdl`, `test-hdl`, `eval`, `tick`, `tock`, and `set-pin NAME VALUE`. It is not a shell and executes no external tool.

Recovery stores open buffers in the platform app-data directory, separately from
project files. Relaunch restores documents and reports recovered buffers. The
simulation remains stopped. External edits block Save; this build preserves the
buffer and opens a conflict dialog with the disk version. Keep editing, reload that
version, or explicitly overwrite it. A newer disk change stops the overwrite and
requires another review. Autosave skips pending conflicts; no automatic merge occurs.

On narrow windows, Files, Editor, Machine and Console become separate panes.
This responsive desktop UI does not establish Android device support.

For hardware, open an `.hdl` file and choose **Load & Eval HDL** or **Load HDL**.
Build also loads HDL. The loader captures all
sibling HDL files and overlays open buffers, then resolves absent dependencies
from the embedded built-ins. Change input pins with Return, then use Eval, Tick
and Tock. Step completes one clock cycle; Run 10k runs a cancellable batch. Pins,
internal wires and selected built-in component pins show backend state. Select
a memory component to inspect/edit an address. Open an ASM/HACK file and use
**Load current ASM/HACK into ROM** for the selected ROM32K component. Screen and
keyboard share the normal input surface. Edits never reload the running chip.
After edits, **Reload & Eval** explicitly loads the visible buffers and resets
hardware state and the clock; ordinary Eval retains the loaded state. Failed
explicit reloads retain the last valid chip and report a navigable source error.
Project-local chips take precedence over built-ins, including empty student
starters. A warning names empty dependencies; implement those course exercises
before expecting a composite chip to work with them.

HardwareSimulator SCRIPT.tst and the in-app HDL test use the same native engine.
Common hardware scripts are supported; complete compatibility is not established.
Signed narrow-pin behavior has targeted reference coverage. Consult hardware.md
and the parity register before relying on unverified workflows.

The Files menu lists recent workspaces. Create files or folders from the workspace
pane. Each file's ellipsis menu provides rename and Move to recovery trash. Modified
documents must be saved or closed before moving them. Deleted files are retained in
`.nandstudio-trash/<id>/`; the console reports the path and `original-path.txt` records
the original location. Restore using ordinary file operations; an in-app trash browser
is not implemented. Nested creation requires an existing parent folder.

Project search runs in a worker and includes unsaved open buffers. Search and Problems
in the console show located results; clicking opens the corresponding document.
Desktop splitter sizes are saved after dragging. These additions have Windows-only
evidence; URI providers and Android lifecycle restoration remain release blockers.
