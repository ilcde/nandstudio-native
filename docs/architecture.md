# Architecture and decisions

`nand_core` is a C++20 library without Qt, GUI initialization, JRE, Python or
network dependencies. Assembler/CPU/comparison are in `src/core.cpp`; Jack and VM
services have separate translation units. `src/script.cpp` shares the CPU/VM/HDL
engines with interactive execution. Named desktop executables use `src/cli.cpp`.
Qt Quick defines presentation; C++ owns documents, files, compilation and state.

Word values use uint16_t with explicit wrap and a portable signed interpretation.
CPU and hardware semantics must remain separate: the legacy CPU emulator's jump
ordering must not leak into an HDL CPU implementation. The independent hardware graph is described in hardware.md.

Build actions capture the visible document as immutable UTF-8 input. Test scripts
use disk files and refuse to start while any open document is modified. Edits do
not reload simulations. Loads parse before replacing a valid CPU/VM/HDL instance. Hardware construction and clock batches also run off the GUI thread.
Compilation and bounded execution run via QtConcurrent. The GUI consumes result
snapshots, so paint frequency does not determine the number of executed steps.

Documents preserve UTF-8 BOM, dominant newline style and final-newline state;
no-op saves do not rewrite files. Other encodings are refused. Mixed-newline edit
preservation remains incomplete. QSaveFile commits atomically; pre-save byte
comparison catches external edits, but the small check/commit race still needs a
stronger storage transaction. Recovery saves buffers separately every five
seconds; it never autosaves onto external project files. Full conflict-resolution
UI and configurable autosave remain incomplete.

The current file services are local-path based. An Android document-provider
storage adapter is **not implemented**. It must replace path assumptions with
workspace-relative identifiers, URI grants and explicit import/export capabilities
before Android can be called supported. QML has no alternate simulator or parser.

The reference tree is development-only. Java runs only in differential testing.
No Java executable, simulator JAR, Python runtime or browser is needed by native
executables. Legacy extension compatibility has no implementation yet; it cannot
be declared solved by offering a different native API.

Resource limits: 32 MiB core file reads, 1 million tokens, Jack parser nesting
256, script execution budget and cancellation, bounded instruction batches,
32768-word CPU memories. Limits and malformed-input diagnostics are additions
that need compatibility review for valid large programs.

## Continued document and interface implementation

`src/app/storage.*` defines provider capabilities, read/write/list/child/create/rename
and workspace-relative resolution. The implemented filesystem provider uses atomic
QSaveFile writes, rejects traversal and reserves existing destinations. Android
content URI providers are still absent; this abstraction does not establish SAF
compatibility. Document saves compare disk bytes with the opened version. Conflict
resolution checks the reviewed version again before reload or overwrite. A race
between the last comparison and commit is still possible without filesystem CAS;
no claim of interprocess transactions is made.

Project enumeration and search run in QtConcurrent workers using captured paths
and buffer snapshots. Results are published to the GUI on completion; task errors
carry originating paths. Execution stays separate from edit events. Autosave skips
pending conflicts. Recent folders and preferences use QSettings with native value
conversion; recovery continues to store buffers separately from user files.

`EditorServices` applies text transforms using QTextCursor edit blocks; QML supplies
presentation, selection and commands. `LineNumberGutter` paints text-block locations.
The shared QML controls and panes are described in design-system.md.
