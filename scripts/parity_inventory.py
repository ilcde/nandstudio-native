"""Generate the conservative parity register from baseline assets and audited source."""
import json,pathlib,re
ROOT=pathlib.Path(__file__).resolve().parents[1]
UP=ROOT/'reference/upstream'
features=[]
platforms=['windows-x86_64','macos-arm64','macos-x86_64','linux-x86_64','android-arm64-v8a','android-x86_64']
def add(id,evidence,implementation=None,tests=None,notes='',kind='legacy'):
    features.append(dict(id=id,kind=kind,original_evidence=evidence,native_implementation=implementation,
        regression_tests=tests or [],platform_coverage={p:('limited-local-tests' if p=='windows-x86_64' and tests else 'not-verified') for p in platforms},
        status='partial' if implementation else 'not-implemented',release_blocker=True,notes=notes))
add('assembler.grammar','CompilersPackageSource/Hack/Assembler/*.java; uploaded Compilers.jar','src/core.cpp',['assembly-matrix','assembly-legacy'],'2,368 instructions including aliases. Invalid input diagnostics and unusual tokenizer combinations not exhaustively matched.')
add('assembler.symbols','HackAssembler.generateSymbolTable/compileLine','src/core.cpp',['assembly-legacy'],'Legacy duplicate labels and out-of-short-range numeric text becoming symbols preserved in tested cases.')
add('assembler.interactive','HackAssemblerGUI / HackTranslatorGUI','src/app/studio.cpp',['gui_workflow'],'Build snapshot and generated artifact available. Incremental translation, selection mapping and comparison UI missing.')
add('cpu.instructions','SimulatorsPackageSource/Hack/CPUEmulator/CPU.java; uploaded Simulators.jar','src/core.cpp',['cpu-simultaneous','cpu-alu-trace','core_tests'],'Legacy jump uses NEW A; memory destination uses OLD A. Exhaustive invalid machine encodings and diagnostics pending.')
add('cpu.reset','CPU.initProgram','src/core.cpp',['core_tests'],'Keeps data RAM and clears screen. Full reset/reload interactions not exhausted.')
for name in ['screen','keyboard','registers','memory-inspection','step','bounded-run','stop']:
    add('cpu.gui.'+name,'CPUEmulatorComponent and HackGUI sources','src/app/Main.qml; src/app/studio.cpp',['gui_workflow'] if name in ['step','memory-inspection'] else [],'Incomplete interaction coverage; continuous run, breakpoints, display modes and keyboard edge cases remain.')
for name in ['breakpoints','animation','number-formats','ROM-editing','instruction-list','execution-speed']:
    add('cpu.gui.'+name,'SimulatorsGUIPackageSource/SimulatorsGUI/CPUEmulatorComponent.java; HackController')
for name in ['commands','segments','branches','calls','returns','recursion','static-scope']:
    add('vm.'+name,'SimulatorsPackageSource/Hack/VMEmulator; uploaded VME fixtures','src/vm.cpp',['scripts/differential.py'],'Valid bundled cases tested. Segment bounds, invalid-program diagnostics and bootstrap differences remain; see evidence.')
add('vm.load-order','VMProgram.java','src/vm.cpp',['vm-NestedCallVME'],'Native sorts filenames. Baseline directory enumeration order not fully characterized. Indented label rejection is preserved in the observed NestedCall case; broader symbol-pass edge cases remain.')
for name in ['native-service-fallback','confirmation','local-class-precedence','call-stack-view','segment-views','breakpoints','string-script-variables']:
    add('vm.'+name,'VMProgram.java; VMEmulator.java; VMEmulatorComponent.java')
for name in ['tokens','expressions','scopes','arrays','strings','objects','calls','control-flow','VM-output']:
    add('jack.'+name,'Uploaded Compilers.jar Hack.Compiler classes; reference source archive does NOT contain Jack compiler source','src/jack.cpp',['scripts/differential.py'],'Generated output tested on bundled programs; complete semantic checking, cross-class validation, warnings and error recovery not ported.')
add('jack.invocation','tools/JackCompiler.bat; tools/JackCompiler.sh','src/cli.cpp',[],'Single file, directory and cwd supported. CLI wording/exit behavior and extension validation incomplete.')
add('text-comparer','MainClassesSource/TextComparer.java; uploaded TextComparer.class javap','src/core.cpp; src/cli.cpp',['comparer-0','comparer-1','comparer-2','comparer-3','comparer-4','comparer-5','comparer-6'],'ASCII-space removal, edge trimming, line counts, missing lines and return status tested. Legacy default-charset differences remain.')
for name in ['load','set','repeat','while','output-file','compare-to','output-list','output','echo','clear-echo']:
    add('script.'+name,'HackPackageSource/Hack/Controller/Script.java; HackController.java','src/script.cpp',['scripts/differential.py'] if name not in ['while','echo','clear-echo'] else [],'CPU/VM/HDL subset. Breakpoints and full punctuation/stop semantics are pending. Partial output persistence on script errors has a Keyboard rejection regression.')
for name in ['breakpoint','clear-breakpoints','stop-terminator','single-step-terminator','GUI-script-control','full-diagnostics']:
    add('script.'+name,'Hack/Controller sources; HardwareSimulator.java')
for name in ['grammar','hierarchy','buses','sub-buses','constants','built-in-lookup','dependency-precedence','cycle-detection','clock-tick','clock-tock','eval','reload','pin-inspection','component-inspection','ROM-load','test-runner']:
    add('hardware.'+name,'SimulatorsPackageSource/Hack/Gates; HardwareSimulator; uploaded HDL fixtures','src/hdl.cpp; src/app/studio.cpp; src/app/Main.qml',['hardware_tests','scripts/differential.py','gui_workflow'],'Implemented subset; see docs/hardware.md for known numerical, hierarchy, diagnostics and GUI gaps. Windows-only evidence.')
for path in sorted((ROOT/'reference/baseline/nand2tetris/tools/builtInChips').glob('*.hdl')):
    add('chip.'+path.stem,str(path.relative_to(ROOT)),'src/hdl.cpp; resources/hdl/'+path.name,['hardware_tests','scripts/differential.py'],'Native behavior implemented; all 35 declarations load/eval in unit tests. Differential coverage varies per chip; complete original visual controls and edge cases remain.')
add('hardware.narrow-negative-values','HardwareSimulator.isLegalWidth and Node signed-short storage; uploaded Not binary','src/hdl.cpp',['scripts/probe_hardware_differences.py'],'CONFIRMED MISMATCH: set in -1 on Not returns legacy in=-1/out=2; native in=1/out=0. Release blocked. No normalization permitted.')
add('hardware.nonstandard-clock-declarations','BuiltInGateClass.isClocked and CompositeGate.clockUp/clockDown','src/hdl.cpp',[],'Unverified: flattened scheduling differs structurally from per-composite clock traversal for custom CLOCKED outputs, missing declarations and order-sensitive circuits. Requires targeted differential coverage.')
add('script.HDL-commands','HardwareSimulator.java and bundled .tst files','src/script.cpp',['scripts/differential.py'],'load/set/eval/tick/tock, ROM command, time strings and common formatting verified in 36 cases; full diagnostics/commands pending.')
for path in sorted((UP/'BuiltInVMCodeSource').glob('*.java')):
    if path.stem=='JackOSClass':continue
    for method in re.findall(r'public\s+static\s+(?:void|short|boolean|char|int)\s+(\w+)\s*\(',path.read_text(errors='replace')):
        add('os.'+path.stem+'.'+method,str(path.relative_to(ROOT)),notes='Compiled VM assets preserved unchanged. Native equivalent and fallback not implemented.')
for name in ['Java-chip-binaries','Java-chip-GUI','Java-VM-binaries','callbacks-to-user-VM','native-extension-interface']:
    add('extensions.'+name,'reference/upstream/README.md Chip API and VMCode API',notes='RELEASE BLOCKER: no legacy binary bridge or native plugin loader. A C++ migration guide would not establish binary compatibility.')
editor_done=['folder-explorer','tabs','open','create','save','dirty-state','undo-redo','clipboard','highlighting','font-scaling','wrap','find-replace','go-to-line','comment-toggle','project-search','diagnostic-navigation','recovery','session-restore','external-change-detection','snapshot-build','explicit-reload','task-console','themes','responsive-panes']
for name in editor_done:add('editor.'+name,'User-requested addition','src/app/studio.cpp; src/app/Main.qml',['gui_workflow'] if name in ['open','save','dirty-state','undo-redo','snapshot-build','themes','responsive-panes','external-change-detection'] else [],'Implemented subset; not all edge cases tested. See user-guide.md.',kind='addition')
for name in ['rename','safe-delete','recent-workspaces','line-number-gutter','indent-assistance','bracket-match','configurable-indent','configurable-autosave','conflict-resolution-UI','layout-restoration']:
    add('editor.'+name,'User-requested addition','src/app/editor_services.cpp; src/app/storage.cpp; src/app/studio.cpp; src/app/*.qml',['gui_workflow'] if name not in ['configurable-autosave','layout-restoration'] else [],'Implemented desktop workflow; responsive Windows geometry checked. Android and other platforms remain unverified. Trash restoration currently uses ordinary file operations; no in-app trash browser.',kind='addition')
for name in ['parser-completion','symbol-navigation','workspace-test-run']:
    add('editor.'+name,'User-requested addition',kind='addition')
for name in ['SAF-document-URIs','directory-tree-grants','grant-recovery','local-import-export','IME','lifecycle','offline-assets','16KB-package-verification','device-workflow']:
    add('android.'+name,'User requirement; Qt 6.11 and Android platform contracts',notes='No Android Qt kit or NDK installed. SAF and lifecycle implementation missing, independently of toolchain blocker.')
for tool in ['HardwareSimulator','CPUEmulator','VMEmulator','Assembler','JackCompiler','TextComparer']:
    add('launcher.'+tool,'Uploaded tools/'+tool+'.bat and .sh','src/cli.cpp',[],'Named executable exists. No-argument GUI dispatch and exact help/diagnostic/exit compatibility remain incomplete. HardwareSimulator now runs saved HDL scripts through the native core.')
for name in ['windows-package','macos-bundle','linux-package','android-APK','android-AAB','CI','licenses']:
    add('delivery.'+name,'User requirement',notes='See build.md and platforms.json for actual configured/compiled/packaged/launched/tested states.')
# Source-derived GUI strings retain discovery evidence without claiming a complete manual audit.
gui=[]
for package in ['HackGUIPackageSource','SimulatorsGUIPackageSource']:
 for p in (UP/package).rglob('*.java'):
  for line_no,line in enumerate(p.read_text(errors='replace').splitlines(),1):
   if re.search(r'setText\(|setToolTipText\(|new JMenuItem\(|new JMenu\(',line):gui.append(dict(source=str(p.relative_to(ROOT)),line=line_no,text=line.strip(),status='not-mapped'))
manifest=dict(schema_version=1,release_status='BLOCKED_INCOMPLETE',audit_status='initial_inventory_not_exhaustive',features=features,gui_discovery=gui)
(ROOT/'docs/parity-manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
print(len(features),'feature entries;',len(gui),'GUI evidence rows; release blocked')
