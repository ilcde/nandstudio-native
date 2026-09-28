"""Compare return acceptance and emitted bytes with the uploaded Java compiler.

Diagnostic envelopes, recovery and exit-code parity are recorded separately;
passing this focused contract does not certify full compiler parity.
"""
import hashlib
import json
import pathlib
import shutil
import sys
import differential as d

cases = {
    'constructor-this': 'constructor Probe new() { return this; }',
    'constructor-alias': 'constructor Probe new() { var Probe x; let x=this; return x; }',
    'constructor-parentheses': 'constructor Probe new() { return (this); }',
    'constructor-null': 'constructor Probe new() { return null; }',
    'constructor-empty': 'constructor Probe new() { return; }',
    'constructor-arithmetic': 'constructor Probe new() { return this+0; }',
    'constructor-wrong-type': 'constructor int new() { return this; }',
    'void-empty': 'function void f() { return; }',
    'void-value': 'function void f() { return 1; }',
    'int-empty': 'function int f() { return; }',
    'int-value': 'function int f() { return 123; }',
    'int-boolean-reference-accepts': 'function int f() { return true; }',
    'object-empty': 'method Probe f() { return; }',
    'object-null': 'method Probe f() { return null; }',
}

def main():
    shutil.copytree(d.BASE/'tools', d.TOOLS, dirs_exist_ok=True)
    cp=d.SEP.join(str(p) for p in [d.TOOLS/'bin/classes', *sorted((d.TOOLS/'bin/lib').glob('*.jar')), d.TOOLS])
    records=[]
    for name, body in cases.items():
        source='// Independent native compatibility fixture, GPL-3.0-or-later.\nclass Probe {\n    '+body+'\n}\n'
        folders=[d.WORK/'jack-return-contract'/name/engine for engine in ('java','native')]
        for folder in folders:
            folder.mkdir(parents=True,exist_ok=True)
            (folder/'Probe.jack').write_text(source,encoding='utf-8')
            (folder/'Probe.vm').unlink(missing_ok=True)
        java=d.run([d.JAVA,'-cp',cp,'Hack.Compiler.JackCompiler',str((folders[0]/'Probe.jack').resolve())],d.TOOLS)
        native=d.run([d.native('JackCompiler'),'Probe.jack'],folders[1])
        outputs=[(folder/'Probe.vm').read_bytes() if (folder/'Probe.vm').exists() else None for folder in folders]
        accepted=java['code']==0
        passed=(native['code']==0)==accepted and outputs[0]==outputs[1] and (outputs[0] is not None if accepted else outputs[0] is None)
        if java['code']=='timeout' or native['code']=='timeout': passed=False
        records.append(dict(name=name,source=source,return_contract_passed=passed,
            full_cli_result_equal=java==native,java=java,native=native,
            output_sha256=[hashlib.sha256(b).hexdigest() if b is not None else None for b in outputs]))
        print(('PASS ' if passed else 'FAIL ')+name)
    output=d.ROOT/'docs/evidence/jack-return-contract.json'
    output.write_text(json.dumps(records,indent=2)+'\n',encoding='utf-8')
    print(f'{sum(r["return_contract_passed"] for r in records)}/{len(records)} acceptance/output contracts; raw CLI differences retained')
    return 0 if all(r['return_contract_passed'] for r in records) else 1

if __name__=='__main__': sys.exit(main())
