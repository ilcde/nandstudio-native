"""Check the reported Xor and project-local starter precedence against Java."""
import json, shutil, sys
import differential as d
d.WORK.mkdir(parents=True, exist_ok=True)
shutil.copytree(d.BASE/'tools', d.TOOLS, dirs_exist_ok=True)
source=(d.ROOT/'tests/fixtures/hardware/requested-xor/Xor.hdl').read_text()
commands='load Xor.hdl, output-file Xor.out, output-list a%B1.1.1 b%B1.1.1 out%B1.1.1;\n'
for a,b in [(0,0),(0,1),(1,0),(1,1),(0,1)]:
    commands+=f'set a {a}, set b {b}, eval, output;\n'
for name,extra in [('builtin-dependencies',{}),('unfinished-local-dependencies',{
        chip+'.hdl':(d.BASE/'projects/1'/f'{chip}.hdl').read_text()
        for chip in ['Not','And','Or']})]:
    d.test('requested-xor-'+name,'HardwareSimulator','HardwareSimulatorMain',
           {'Xor.hdl':source,'Xor.tst':commands,**extra},['Xor.tst'],['Xor.out'])
(d.ROOT/'docs/evidence/xor-eval.json').write_text(json.dumps(d.report,indent=2)+'\n')
sys.exit(0 if all(r['passed'] for r in d.report) else 1)
