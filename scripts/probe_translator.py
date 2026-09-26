"""Run generated ASM with unchanged course CPU scripts in Java and native engines.
Development only. All generated files stay under tests/work; starters are read-only.
"""
import json
import pathlib
import re
import shutil
import subprocess
import differential as d

def main():
    d.WORK.mkdir(parents=True,exist_ok=True)
    shutil.copytree(d.BASE/'tools',d.TOOLS,dirs_exist_ok=True)
    for project in ('7','8'):
        for script in sorted((d.BASE/'projects'/project).rglob('*.tst')):
            if script.stem.endswith('VME'): continue
            fixture=d.ROOT/'tests/work/translator'/script.stem
            fixture.mkdir(parents=True,exist_ok=True)
            sources={p.name:p.read_bytes() for p in script.parent.glob('*.vm')}
            if not sources: continue
            for name,data in sources.items(): (fixture/name).write_bytes(data)
            bootstrap=any(re.search(rb'^\s*function\s+Sys\.init\s',data,re.M) for data in sources.values())
            command=[str(d.native('nand')),'VMTranslator',str(fixture)]
            if bootstrap: command.append('--bootstrap')
            subprocess.run(command,check=True,timeout=30,capture_output=True)
            files={p.name:p.read_bytes() for p in script.parent.iterdir() if p.suffix in ('.tst','.cmp')}
            files[script.stem+'.asm']=(fixture/(script.stem+'.asm')).read_bytes()
            d.test('translated-'+script.stem,'CPUEmulator','CPUEmulatorMain',files,[script.name],[script.stem+'.out'])
    output=d.ROOT/'docs/evidence/translator-course.json'
    output.write_text(json.dumps(d.report,indent=2),encoding='utf-8')
    print(f'{sum(row["passed"] for row in d.report)}/{len(d.report)} translated course cases passed')
    return 0 if d.report and all(row['passed'] for row in d.report) else 1

if __name__=='__main__': raise SystemExit(main())
