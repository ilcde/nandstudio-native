# Editor and reproducible-build continuation — 24 September 2026

This is an implemented development increment. The complete migration remains blocked.

## Changes

- Audited `nand2teris_tool.zip` safely: 6,647 entries, 452,146,617 expanded bytes; source matched the starting workspace. SHA-256: `2c9ffa6a17576e9464f792f4955fda95b27edea2e3997ea7e351219af28c6a34`. No extraction over user work.
- Reproduced Create File overflow (280-wide field inside 178-wide popup), then replaced the monolithic interface with shared themed controls, bounded dialogs, responsive toolbars/search and separate workspace/editor/console/machine/inspector panes.
- Added C++ line-number painting, multiline indent/unindent, bracket matching, auto-indent and single-undo replace-all. Added settings, recent folders and persisted splitter sizes.
- Added filesystem provider abstraction; workspace-safe creation, rename following open documents and recoverable deletion. Added disk-conflict preview/reload/overwrite with a second version check. Autosave is configurable and skips pending conflicts.
- Moved project enumeration/search onto workers; search sees captured unsaved buffers and produces clickable file/line/column results. Build diagnostics keep their originating document when the active tab changes.
- Added deterministic source packaging and selectable build directories. CMake does not rewrite the generated HDL header when its contents are unchanged. Packages deploy into a new directory, preserving older running copies.

Principal implementation files: `src/app/editor_services.*`, `src/app/storage.*`,
`src/app/studio.*`, `src/app/Main.qml`, the new QML controls/panes/dialogs,
`tests/gui_checks.cpp`, `CMakeLists.txt`, and `scripts/{build-windows.ps1,
package-windows.ps1,source_package.py,differential.py}`. Design, user guide,
architecture, build instructions and machine-readable parity/platform registers
are updated. No original project, expected comparison output or Java engine was modified.

## Executed verification

| Check | Result |
|---|---|
| Windows MSVC/Qt configure and compile | Passed in `build-continuation` |
| CTest core / hardware / GUI suites | 3/3 passed; 2,012 core, 2,127 hardware and 159 GUI checks |
| GUI scope | Editing, undo/redo, save encoding/newlines, CPU/HDL controls, conflict paths, create/rename/trash, unsaved-buffer search, originating diagnostics, preferences and bounded modals |
| Create File geometry | Five window sizes, scales 1.0/1.5, long filenames; passed |
| Runtime QML checks | No binding-loop, type, reference or settings-serialization errors in final test run |
| Packaged Windows launch | Passed with Windows Qt plugin/software renderer; system Qt and Java absent from PATH; all 159 GUI checks passed |
| Packaged hardware batch | Devices.out byte-identical to uploaded Java reference |
| Differential suite | 65/65 matched: 63 outputs/comparisons and two expected rejections |
| Separate known-difference probe | Still fails: narrow HDL input -1 is masked natively; explicitly release-blocking |
| Archive rejection tests | 4/4 passed |
| Baseline integrity | 881 files, seven JARs, zero extraction differences |
| Deterministic source archive | Two byte-identical archives; safely extracted intermediate snapshot configured, compiled and passed 3/3 suites |

Final source reproduction records are written beside the delivered source ZIP in
`dist/source-reproduction.json`; the intermediate source verification remains in
`docs/evidence/source-package.json`. Build warnings include unavailable optional
Vulkan headers and pre-existing integer-conversion warnings; neither was concealed.
This is not a clean-machine or hardware-GPU certification.

## Artifacts

- `dist/NandStudio-editor-windows-x86_64/bin/NandStudio.exe`
- `dist/NandStudio-editor-windows-x86_64.zip` (unsigned portable Windows package)
- `dist/NandStudio-source.zip` (native source, tests, docs, notices and original reference ZIP)
- `dist/source-reproduction.json` and `dist/artifact-sha256.json`
- `docs/evidence/gui-continuation.json`, `gui-packaged-windows.json`, `packaged-launch.json`, `differential.json`, `hardware-known-differences.json`, `continuation-archive.json`, before/after screenshots

## Remaining blockers

Only Windows x86-64 was compiled and exercised. Linux verification could not start:
Docker Desktop failed initializing its Inference-manager socket, and no Linux
engine was available. No Docker reset or existing distribution changes were made.
There is no Git repository/remote here, so CI was not dispatched or source published.
No macOS host was exercised. Android lacks its Qt kit, pinned NDK and SDK command-line
tools; SAF providers/grants/import-export, lifecycle/IME and device testing remain
unimplemented. No APK/AAB or 16 KB package evidence was produced.

Native OS services and VM fallback/precedence, Java extension-binary compatibility,
full test-script/launcher/GUI behavior, the narrow-negative HDL mismatch and unusual
clock scheduling remain release blockers. Parser completion/navigation, comprehensive
parser diagnostics, continuous execution/breakpoints and workspace test runs remain.
New file-provider code implements local paths only. Trash restore has no in-app
browser. Autosave timing, splitter restoration and the full accessibility/IME surface
need broader automated coverage. Concurrent save compare/commit is not an interprocess
transaction. Long-file resource/cancellation behavior needs further work.

The parity register remains conservative: 227 features, 135 partial and 92 not
implemented; no feature or platform is certified complete. The original 150 GUI
source-evidence rows still require exhaustive operation mapping.
