# NandStudio development packages

Version 0.1.10 adds an installed-font picker with a preview, vector toolbar icons,
an offline tutorial (More or F1), workflow diagrams and new application demos.
The root LICENSE exposes the existing native-code GPL terms; bundled course
assets retain their separate notices. These additions do not close full parity.

Version 0.1.9 adds **Compile Jack folder** in Converters & diagnostics and
`build-folder` in the console, including unsaved dependency buffers and protected
output commits. The screenshot gate now rejects blank
app surfaces; it does not replace manual visual or physical-device testing.

Version 0.1.8 adds a native bitmap editor with drawing, transforms, undo/redo and
Jack/ASM code generation. Image import and animation export remain unimplemented.

Version 0.1.7 includes the eight unchanged compiled Jack OS files in offline
course workspaces. **More > Add supplied Jack OS** copies missing OS classes
and their notice into the active document folder without replacing local files.
Load VM explicitly afterward. This uses VM implementations, not native built-in
service fallback.

Version 0.1.6 adds BusSelect4 and RememberBit practice examples, real application
screenshots and an Eval GIF. It restores the reference VM initial stack pointer
and rejects output lists longer than 20 fields before modifying output files.
See [demonstrations](demos.md) and the recorded regression evidence.

Version 0.1.5 makes unfinished HDL dependencies visible at each Eval, records
actual input values and adds navigation to the affected local chip files.
The reported Windows Xor result matches Java when local starter chips are empty.

This version embeds all 249 unchanged course starter files (projects 0–13).
Choose **Files > Create course workspace** to create an editable offline copy
on any platform. Four separate HDL/TST/CMP examples demonstrate pin editing, truth tables,
bus wiring and clocked storage. Export local
work before uninstalling Android development builds.

Eval now detects changed, added and removed closed HDL dependencies on explicit
evaluation, through shared application code on Windows, macOS, Linux and Android.
VM-to-ASM translation, HDL validation previews, PC breakpoints and live watches
are available; see the user guide for their scope and limitations.

Version 0.1.2 adds explicit Android folder import into an editable local copy and
export into a new provider folder. Save changes the local copy; the original stays
unchanged. See `docs/android-workspaces.md`. Publication requires an emulator
import/edit/save/assemble/export/reopen test for the exact x86-64 APK. This one
provider workflow does not establish complete Android or course-wide parity.
The VM Machine pane now shows the next command, source line, frame pointers,
recent stack words and call stack from the actual execution state.
Session recovery now records active-document changes immediately and edited
buffers after a short pause; backgrounding flushes a session snapshot.

Eval now detects unsaved HDL even after switching to a non-HDL editor tab.
One-bit pin inputs have tap toggles; completed evaluations display output values.
On narrow screens, loading HDL opens the Machine pane. Script hexadecimal output
uses the legacy lowercase convention, tested with the course Seven application
and explicit OS VM files. The implementation checklist is in `docs/TODO.md`.

Version 0.1.1 fixes Android Files/More menus opening over the status bar. Menus
are anchored below the header and touch devices no longer display hover tooltips
over their actions. More displays the app version so an installed older APK can
be identified. Android release validation now taps More, Files and Open workspace,
checks popup bounds, and verifies that the folder chooser opens. This does not
claim complete Android document-provider compatibility.

This is an incomplete C++/Qt migration, not a feature-complete release of the
Nand2Tetris suite. The attached manifest identifies the exact source revision,
successful CI run, package checksums, and packaged desktop GUI test counts.
All six executable packages come from that one run. Build success does not
establish full workflow compatibility on any platform.

- Windows x86-64: extract the portable ZIP and run `bin/NandStudio.exe`.
- Linux x86-64: extract the tarball and run `NandStudio.sh`; glibc 2.39 or newer.
- macOS Apple Silicon / Intel: use the corresponding DMG. These application
  bundles are unsigned and unnotarized; release signing is not configured.
- Android ARM64 / x86-64: APKs use Android development signing, not a publisher
  release key. ARM64 targets devices; x86-64 targets suitable emulators. Native
  ELF and ZIP alignment are checked for 16 KB, but runtime verification on a
  16 KB device remains pending. Debug signing keys may differ between CI runs.

Current additions include pending-input Eval fixes, signed HDL pin compatibility,
live hardware hierarchy/connection inspection, and bounded signal history.
Corrupted menu ellipses and the hardware time separator have been corrected,
with regression checks against the actual QML text properties.
The integrated editor and native tools remain under active development.

This increment adds explicit Reload & Eval for modified HDL buffers, structured
HDL load diagnostics, and warnings for empty project-local dependencies. The
reported composite Xor is tested against the original Java engine, including
the original behavior when local student starters override built-ins. Find and
replace now opens from More or Ctrl+F, targets the active editor and supports
single-operation undo. The Android toolbar accounts for the status bar/cutout
safe area; a device layout-report entry point records its actual control bounds.
Installation, updates, troubleshooting and course-project coverage are documented
in `docs/install.md`, `docs/user-guide.md` and `docs/course-workflows.md` in the
source ZIP and desktop package's `share/nandstudio/docs`. The repository README
links each platform's package and guide. The attached `android-toolbar.json`
records the runtime check for this exact release's emulator APK.

Release blockers include broader Android provider and device/lifecycle
qualification, full native VM OS service fallback, existing Java extension binary
compatibility, complete script/diagnostic parity, and the remaining GUI operations.
See `docs/parity-manifest.json` and `docs/platforms.json` in the source archive.
Preserve existing project backups when trying development builds.

Known Android emulator visual limitation: local emulator 36.3.10 with SwiftShader
showed triangular background/button artifacts. The real-inset toolbar check passes,
but does not certify rendering or complete touch workflows. Physical ARM64 visual
testing remains pending. A software-renderer override failed Android verification
and was withdrawn. The replacement probes Vulkan availability before preferring
Vulkan, with the platform default retained when unavailable. Fresh workflow and
screenshot verification is required. See the
repository's `docs/release-2026-09-25.md` and parity register for recorded evidence.

Source ZIPs attached here contain the tracked native project. The uploaded legacy
reference archive is not in Git or this release ZIP; differential testing requires
that original archive and the development-only Java reference setup documented
in the repository. No Java reference engine is used by the application at runtime.
