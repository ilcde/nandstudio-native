"""Install a development APK and verify actual Android toolbar safe-area bounds."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import time

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('apk',type=pathlib.Path)
parser.add_argument('report',type=pathlib.Path)
parser.add_argument('--adb',default='adb')
args=parser.parse_args()
package='org.qtproject.example.NandStudio'
def adb(*command,check=True):
    return subprocess.run([args.adb,*command],capture_output=True,text=True,encoding='utf-8',errors='replace',check=check,timeout=90)
adb('install','-r',str(args.apk))
adb('shell','am','force-stop',package)
# Delete only this tool's previous report, never app documents/settings.
adb('shell','run-as',package,'rm','-f','files/layout-report.json')
args.report.parent.mkdir(parents=True,exist_ok=True)
adb('logcat','-c')
launch=adb('shell','am','start','-W','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.layoutCheck','true')
args.report.with_suffix('.launch.txt').write_text(launch.stdout+launch.stderr,encoding='utf-8')
report=None
for _ in range(45):
    result=adb('exec-out','run-as',package,'cat','files/layout-report.json',check=False)
    if result.returncode==0:
        try: report=json.loads(result.stdout)
        except json.JSONDecodeError: pass
        if report is not None: break
    time.sleep(1)
if report is None:
    # Retain the complete bounded emulator log buffer: startup failures can be
    # displaced by unrelated system messages within the last 1,000 lines.
    args.report.with_suffix('.log').write_text(adb('logcat','-d').stdout,encoding='utf-8')
    args.report.with_suffix('.files.txt').write_text(adb('shell','run-as',package,'find','files','-maxdepth','2','-type','f',check=False).stdout,encoding='utf-8')
    raise RuntimeError('Application did not produce its Android layout report')
report.update(apk_sha256=hashlib.sha256(args.apk.read_bytes()).hexdigest(),
    abi=adb('shell','getprop','ro.product.cpu.abi').stdout.strip(),
    android_api=adb('shell','getprop','ro.build.version.sdk').stdout.strip(),
    page_size=adb('shell','getconf','PAGE_SIZE').stdout.strip(),full_workflow_verified=False)
args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
if not report.get('passed') or report.get('safe_top',0)<=0:
    raise RuntimeError('Toolbar failed safe-area check or the emulator did not exercise a status-bar inset')
