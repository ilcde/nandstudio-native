"""Development-only Java/native differential harness. Writes only tests/work/ and docs/evidence/."""
import hashlib, json, pathlib, shutil, subprocess, sys, os
ROOT=pathlib.Path(__file__).resolve().parents[1]
WORK=ROOT/'tests/work/differential'
JAVA=pathlib.Path('C:/Program Files/Java/jdk-21/bin/java.exe') if sys.platform=='win32' else pathlib.Path('java')
BASE=ROOT/'reference/baseline/nand2tetris'
TOOLS=WORK/'reference-runtime'
SEP=';' if sys.platform=='win32' else ':'
report=[]
def run(command,cwd):
    try:
        p=subprocess.run([str(c) for c in command],cwd=cwd,capture_output=True,timeout=20)
        return dict(code=p.returncode,stdout=p.stdout.decode('utf-8',errors='replace'),stderr=p.stderr.decode('utf-8',errors='replace'))
    except subprocess.TimeoutExpired: return dict(code='timeout',stdout='',stderr='20 second limit')
def native(tool): return pathlib.Path(os.environ.get('NAND_NATIVE_BUILD',str(ROOT/'build')))/(tool+('.exe' if sys.platform=='win32' else ''))
def test(name,tool,main,files,args,outputs):
    parent=WORK/name
    dirs=[parent/'java',parent/'native']
    for d in dirs:
        d.mkdir(parents=True,exist_ok=True)
        for output in outputs:
            p=(d/output).resolve()
            if not p.is_relative_to(WORK.resolve()):raise ValueError('Output escapes disposable test workspace')
            p.unlink(missing_ok=True)
        for file,data in files.items():
            p=d/file;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data if isinstance(data,bytes) else data.encode())
    # Original launchers canonicalize inputs before changing to the tools folder.
    java_args=[str((dirs[0]/a).resolve()) for a in args]
    cp=SEP.join(str(p) for p in [TOOLS/'bin/classes', *sorted((TOOLS/'bin/lib').glob('*.jar')), TOOLS])
    java=run([JAVA,'-Djava.awt.headless=true','-cp',cp,main,*java_args],TOOLS)
    cpp=run([native(tool),*args],dirs[1])
    records=[]
    for output in outputs:
        paths=[d/output for d in dirs];data=[p.read_bytes() if p.exists() else None for p in paths]
        records.append(dict(file=output,equal=data[0] is not None and data[0]==data[1],sha256=[hashlib.sha256(b).hexdigest() if b is not None else None for b in data]))
    # stdout path banners are recorded, not used as byte-output equality evidence.
    passed=java['code']==cpp['code']==0 and all(r['equal'] for r in records)
    outcome='success-output-comparison'
    if java['code']!=0:
        passed=java==cpp and all(r['equal'] or r['sha256']==[None,None] for r in records)
        outcome='baseline-rejection-comparison'
    if tool=='TextComparer': passed=java==cpp
    row=dict(name=name,tool=tool,passed=passed,outcome=outcome,java=java,native=cpp,outputs=records)
    report.append(row);print(('PASS ' if passed else 'DIFF ')+name,flush=True)

def main():
    WORK.mkdir(parents=True,exist_ok=True)
    # The original engines write .dat preferences even in some batch paths.
    # Run from a disposable runtime copy, never the immutable baseline.
    shutil.copytree(BASE/'tools',TOOLS,dirs_exist_ok=True)
    # Exercise original test scripts against the original built-in declarations.
    # Do not replace or complete the student's HDL starter files.
    for project in ('1','2','3'):
        for path in sorted((BASE/'projects'/project).rglob('*.tst')):
            files={path.name:path.read_bytes()}
            for cmp in path.parent.glob('*.cmp'): files[cmp.name]=cmp.read_bytes()
            test('hardware-'+path.stem,'HardwareSimulator','HardwareSimulatorMain',files,[path.name],[path.stem+'.out'])
    fixture_dir=ROOT/'tests/fixtures/hardware'
    examples=ROOT/'resources/starters/examples'
    for path in sorted(examples.glob('*.tst')):
        files={p.name:p.read_bytes() for p in examples.iterdir() if p.suffix in ('.hdl','.tst','.cmp')}
        test('starter-'+path.stem,'HardwareSimulator','HardwareSimulatorMain',files,[path.name],[path.stem+'.out'])
    for path in sorted(fixture_dir.glob('*.tst')):
        files={p.name:p.read_bytes() for p in fixture_dir.iterdir() if p.suffix in ('.hdl','.asm','.hack')}
        files[path.name]=path.read_bytes()
        test('hardware-custom-'+path.stem,'HardwareSimulator','HardwareSimulatorMain',files,[path.name],[path.stem+'.out'])
    computations=['0','1','-1','D','A','M','!D','NOTD','!A','NOTA','!M','NOTM','-D','-A','-M','D+1','A+1','M+1','D-1','A-1','M-1','D+A','A+D','D+M','M+D','D-A','D-M','A-D','M-D','D&A','A&D','D&M','M&D','D|A','A|D','D|M','M|D']
    source='// assembler matrix\n'+''.join(f'{d}{c}{j}\n' for c in computations for d in ('','M=','D=','MD=','A=','AM=','AD=','AMD=') for j in ('',';JGT',';JEQ',';JGE',';JLT',';JNE',';JLE',';JMP'))
    test('assembly-matrix','Assembler','HackAssemblerMain',{'matrix.asm':source},['matrix.asm'],['matrix.hack'])
    test('assembly-legacy','Assembler','HackAssemblerMain',{'compat.asm':'(R0)\n@32768\n@-1\n@+2\nD=M[21]\n@foo\n(foo)\n@foo\nD = NOTA // spaces\n'},['compat.asm'],['compat.hack'])
    for idx,(a,b) in enumerate([('a b\r\nc','ab\nc'),('a\tb','ab'),('a\nb','a'),('a','a\nb'),('\ta\t','a'),('',''),('a\n','a\n\n')]):
        test('comparer-'+str(idx),'TextComparer','TextComparer',{'a.txt':a,'b.txt':b},['a.txt','b.txt'],[])
    cpu='@7\nAMD=1;JMP\n'
    script='load state.asm, output-file state.out, output-list A%D1.6.1 D%D1.6.1 PC%D1.6.1 RAM[7]%D1.6.1 time%D1.6.1;\nrepeat 5 { ticktock, output; }'
    test('cpu-simultaneous','CPUEmulator','CPUEmulatorMain',{'state.asm':cpu,'trace.tst':script},['trace.tst'],['state.out'])
    # Every legal ALU encoding, sequential state snapshots; no broad normalizations.
    instructions='@32767\nD=A\n@17\n'+''.join('D='+c+'\n' for c in computations)
    script='load state.asm, output-file state.out, output-list A%D1.6.1 D%D1.6.1 PC%D1.6.1;\nrepeat '+str(len(computations)+3)+' { ticktock, output; }'
    test('cpu-alu-trace','CPUEmulator','CPUEmulatorMain',{'state.asm':instructions,'trace.tst':script},['trace.tst'],['state.out'])
    fixture='class Main { function void main() { var int x; let x=2+3; if(x>1){let x=x-1;}else{let x=0;} while(x>0){let x=x-1;} return; } }'
    test('jack-control','JackCompiler','Hack.Compiler.JackCompiler',{'Main.jack':fixture},['Main.jack'],['Main.vm'])
    for relative in ['10/Square','11/Average','11/ComplexArrays','11/ConvertToBin','11/Pong','11/Seven']:
        directory=BASE/'projects'/relative
        files={p.name:p.read_bytes() for p in directory.glob('*.jack')}
        if files:test('jack-'+relative.replace('/','-'),'JackCompiler','Hack.Compiler.JackCompiler',files,['.'],[str(pathlib.Path(f).with_suffix('.vm')) for f in files])
    for path in sorted((BASE/'projects/7').rglob('*VME.tst'))+sorted((BASE/'projects/8').rglob('*VME.tst')):
        files={p.name:p.read_bytes() for p in path.parent.iterdir() if p.is_file() and p.suffix in ('.vm','.tst','.cmp')}
        outputs=[path.stem.replace('VME','')+'.out']
        test('vm-'+path.stem,'VMEmulator','VMEmulatorMain',files,[path.name],outputs)
    out=ROOT/'docs/evidence';out.mkdir(parents=True,exist_ok=True)
    (out/'differential.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(f'{sum(r["passed"] for r in report)}/{len(report)} differential cases passed')
    return 0 if all(r['passed'] for r in report) else 1
if __name__=='__main__':sys.exit(main())
