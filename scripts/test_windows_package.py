"""Run packaged native GUI and HDL batch checks without a system Qt/JRE PATH."""
import hashlib, json, os, pathlib, shutil, subprocess, sys

root=pathlib.Path(__file__).resolve().parents[1]
package=root/(sys.argv[1] if len(sys.argv)>1 else 'dist/NandStudio-windows-x86_64')
work=root/'tests/work/packaged-windows'
work.mkdir(parents=True,exist_ok=True)
env=os.environ.copy()
env['PATH']=env.get('SystemRoot','C:/Windows')+'/System32;'+env.get('SystemRoot','C:/Windows')
for name in ('QT_PLUGIN_PATH','QML_IMPORT_PATH','QML2_IMPORT_PATH'):
    env.pop(name,None)
env.update(QT_QPA_PLATFORM='windows',QT_QUICK_BACKEND='software',NAND_TEST_STATE_DIR=str(work/'state'))
exe=package/'bin/NandStudio.exe'
p=subprocess.run([str(exe),'--self-test',str(work)],cwd=package,env=env,capture_output=True,timeout=45)
record=dict(executable=str(exe),sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),qt_platform='windows',render_backend='software',exit_code=p.returncode,stdout=p.stdout.decode(errors='replace'),stderr=p.stderr.decode(errors='replace'),system_qt_on_path=False,java_on_path=False)
checks=json.loads((work/'checks.json').read_text())
record['gui_checks']=len(checks)
record['gui_checks_passed']=all(c['passed'] for c in checks)
crt=json.loads((root/'docs/evidence/windows-crt.json').read_text(encoding='utf-8-sig'))
record['app_local_crt_matches_pinned_source']=all((package/'bin'/f['file']).exists() and hashlib.sha256((package/'bin'/f['file']).read_bytes()).hexdigest().upper()==f['sha256'] for f in crt)
# Exercise the packaged headless hardware engine, including embedded declarations.
fixture=root/'tests/fixtures/hardware'
for name in ('Devices.hdl','Devices.asm','Devices.tst'):
    shutil.copyfile(fixture/name,work/name)
batch=subprocess.run([str(package/'bin/HardwareSimulator.exe'),str(work/'Devices.tst')],cwd=package,env=env,capture_output=True,timeout=15)
expected=root/'tests/work/differential/hardware-custom-Devices/java/Devices.out'
record['hardware_batch']=dict(exit_code=batch.returncode,output_matches_reference=(work/'Devices.out').read_bytes()==expected.read_bytes(),stdout=batch.stdout.decode(errors='replace'),stderr=batch.stderr.decode(errors='replace'))
out=root/'docs/evidence'
(out/'packaged-launch.json').write_text(json.dumps(record,indent=2),encoding='utf-8')
shutil.copyfile(work/'checks.json',out/'gui-packaged-windows.json')
print(json.dumps(record,indent=2))
sys.exit(0 if p.returncode==0 and record['gui_checks_passed'] and record['app_local_crt_matches_pinned_source'] and batch.returncode==0 and record['hardware_batch']['output_matches_reference'] else 1)
