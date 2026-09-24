# Native interface components

The Qt Quick presentation is split into shared controls and functional panes. C++
continues to own documents, search, editing transforms, parsers and execution.
No simulator data is fabricated.

`Theme.qml` defines canvas/surface/raised colors, text/accent/border states, spacing,
radii and a 44 logical-pixel minimum primary control height. Interface scaling
is 1.0–1.5. Dark and light palettes are shared by controls and panes.
`ActionButton`, `AppField` and `AppDialog` centralize focus borders, padding,
labels and bounded sizing. Dialog width is a preferred measure clamped to the
overlay; children use layout allocation rather than fixed pixel widths. Long
paths and filenames elide or scroll. Confirmation bodies scroll in a stable
preferred-height viewport; settings have a scrolling body.

`WorkspacePane`, `EditorWorkspace`, `EditorPane`, `ConsolePane`, `MachinePane`,
`HardwareInspector`, `MemoryInspector` and `ScreenView` are reusable components.
Desktop uses splitters; compact widths use Files/Editor/Machine/Console tabs.
The toolbar moves less frequent actions into menus. Search controls change rows
at narrow widths. The editor uses Qt text input and a C++ painted line gutter.
The screen releases simulated keys when focus or application activity is lost.

The uploaded Create File overflow was reproduced before replacement: a 280-wide
field extended outside a 178-wide popup. Evidence is in `evidence/dialog-before.*`.
GUI tests check the replacement at 320×640, 412×820, 820×412, 768×1024 and 1320×860
at scales 1.0 and 1.5, including a long filename. Other modal surfaces are bounded
at narrow portrait and landscape sizes. Tests reject QML binding/type/serialization
errors. This is Windows evidence, not Android device, accessibility certification
or exhaustive manual UI testing.

Remaining interface work includes parser-backed completion/navigation, full legacy
menus and inspectors, continuous execution/breakpoint controls, touch IME and
Android lifecycle tests, workspace test runs and a recovery-trash browser.
