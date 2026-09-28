"""Reference-test the legacy output-list column boundary without editing starters."""
import json
import shutil
import differential as d

shutil.copytree(d.BASE/'tools',d.TOOLS,dirs_exist_ok=True)
for tool,main,variable,prefix in (
    ('CPUEmulator','CPUEmulatorMain','RAM[0]',''),
    ('VMEmulator','VMEmulatorMain','RAM[0]',''),
    ('HardwareSimulator','HardwareSimulatorMain','a','load And.hdl;\n')):
    for count in (20,21):
        name=f'output-columns-{tool}-{count}'
        script=prefix+'output-file fields.out;\noutput-list '+' '.join([variable+'%D1.6.1']*count)+';\noutput;\n'
        d.test(name,tool,main,{'Fields.tst':script},['Fields.tst'],['fields.out'])
        row=d.report[-1]
        if count==21:
            # Only the exact isolated input path differs; diagnostic words,
            # line number, stream, exit code and absent output must all match.
            normalized=[]
            for side in ('java','native'):
                result=dict(row[side]);path=str((d.WORK/name/side/'Fields.tst').resolve())
                result['stderr']=result['stderr'].replace(path,'<SCRIPT>')
                normalized.append(result)
            row['normalization']='Exact isolated Fields.tst absolute path only'
            row['passed']=normalized[0]==normalized[1] and normalized[0]['code']!=0 and all(o['sha256']==[None,None] for o in row['outputs'])
            print(('PASS ' if row['passed'] else 'DIFF ')+name+' (path-normalized rejection)')
(d.ROOT/'docs/evidence/script-output-limits.json').write_text(json.dumps(d.report,indent=2)+'\n',encoding='utf-8')
raise SystemExit(not all(row['passed'] for row in d.report))
