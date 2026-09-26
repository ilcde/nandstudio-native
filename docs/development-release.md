# NandStudio development packages

Version 0.1.2 adds explicit Android folder import into an editable local copy and
export into a new provider folder. Save changes the local copy; the original stays
unchanged. See `docs/android-workspaces.md`. Publication requires an emulator
import/edit/save/assemble/export/reopen test for the exact x86-64 APK. This one
provider workflow does not establish complete Android or course-wide parity.
The VM Machine pane now shows the next command, source line, frame pointers,
recent stack words and call stack from the actual execution state.

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

Release blockers include Android Storage Access Framework access and lifecycle
workflows, full native VM OS service fallback, existing Java extension binary
compatibility, complete script/diagnostic parity, and the remaining GUI operations.
See `docs/parity-manifest.json` and `docs/platforms.json` in the source archive.
Preserve existing project backups when trying development builds.

Known Android emulator visual limitation: local emulator 36.3.10 with SwiftShader
showed triangular background/button artifacts. The real-inset toolbar check passes,
but does not certify rendering or complete touch workflows. Physical ARM64 visual
testing remains pending. No speculative rendering override is shipped. See the
repository's `docs/release-2026-09-25.md` and parity register for recorded evidence.

Source ZIPs attached here contain the tracked native project. The uploaded legacy
reference archive is not in Git or this release ZIP; differential testing requires
that original archive and the development-only Java reference setup documented
in the repository. No Java reference engine is used by the application at runtime.
