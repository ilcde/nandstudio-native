"""Exercise a bundled course application with explicit original OS VM files.

Development-only; never edits starter projects or substitutes native OS fallback.
"""
import json
import os
import pathlib
import shutil
import differential as harness

root=harness.ROOT
os.environ.setdefault('NAND_NATIVE_BUILD',str(root/'build-continuation'))
source=root/'reference/baseline/nand2tetris/projects/11/Seven/Main.jack'
work=root/'tests/work/course-os-compilation'
work.mkdir(parents=True,exist_ok=True)
(work/'Main.jack').write_bytes(source.read_bytes())
compiled=harness.run([harness.native('JackCompiler'),'Main.jack'],work)
if compiled['code']!=0: raise RuntimeError(compiled)
shutil.copytree(harness.BASE/'tools',harness.TOOLS,dirs_exist_ok=True)
files={p.name:p.read_bytes() for p in (harness.BASE/'tools/OS').glob('*.vm')}
files['Main.vm']=(work/'Main.vm').read_bytes()
# Capture screen words covering all 11 rows of the first output glyph plus VM
# registers after initialization, execution, and the user-supplied Sys.halt loop.
variables=[f'RAM[{address}]%D1.6.1' for address in range(5)]
variables += [f'RAM[{16384+32*row}]%X1.4.1' for row in range(12)]
files['Course.tst']='load, output-file Course.out, output-list '+' '.join(variables)+';\nrepeat 2000000 { vmstep; } output;\n'
harness.test('course-Seven-explicit-OS','VMEmulator','VMEmulatorMain',files,['Course.tst'],['Course.out'])
for name in ('ArrayTest','MathTest','MemoryTest','MemoryTest/MemoryDiag','StringTest','OutputTest','ScreenTest'):
    directory=harness.BASE/'projects/12'/name
    compilation=work/name;compilation.mkdir(parents=True,exist_ok=True)
    for source in directory.glob('*.jack'):
        (compilation/source.name).write_bytes(source.read_bytes())
    result=harness.run([harness.native('JackCompiler'),'.'],compilation)
    if result['code']!=0: raise RuntimeError(result)
    inputs={p.name:p.read_bytes() for p in (harness.BASE/'tools/OS').glob('*.vm')}
    inputs.update({p.name:p.read_bytes() for p in compilation.glob('*.vm')})
    scripts=list(directory.glob('*.tst'))
    if scripts:
        inputs.update({p.name:p.read_bytes() for p in directory.iterdir() if p.suffix in ('.tst','.cmp')})
        script=scripts[0].name;output=scripts[0].stem+'.out'
    else:
        # Compare the whole screen, including unchanged pixels, after a fixed
        # instruction count. No image/whitespace normalization is permitted.
        script='ScreenProbe.tst';output='ScreenProbe.out'
        content='load, output-file '+output+';\nrepeat 2000000 { vmstep; }\n'
        for start in range(16384,24576,16):
            columns=[f'RAM[{address}]%X1.4.1' for address in range(start,start+16)]
            content+='output-list '+' '.join(columns)+'; output;\n'
        inputs[script]=content
    harness.test('course-'+name.replace('/','-')+'-explicit-OS','VMEmulator','VMEmulatorMain',inputs,[script],[output])
destination=root/'docs/evidence/course-os.json'
destination.write_text(json.dumps(harness.report,indent=2)+'\n',encoding='utf-8')
raise SystemExit(0 if all(case['passed'] for case in harness.report) else 1)
