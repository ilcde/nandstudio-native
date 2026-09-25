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

# Exercise the actual touch route that the geometry-only check missed.
def read_menu():
    data=adb('exec-out','run-as',package,'cat','files/menu-report.json',check=False)
    try: return json.loads(data.stdout) if data.returncode==0 else None
    except json.JSONDecodeError: return None

def wait_menu(predicate):
    for _ in range(60):
        data=read_menu()
        if data and predicate(data): return data
        time.sleep(0.5)
    args.report.with_suffix('.interaction.log').write_text(adb('logcat','-d').stdout,encoding='utf-8')
    raise RuntimeError('Android menu interaction did not reach the required state')

def named(data,group,name):
    return next((item for item in data[group] if item['name']==name),None)

def tap(data,group,name):
    item=named(data,group,name)
    if not item or not item['inside_safe_area']: raise RuntimeError(name+' is outside the safe area')
    scale=data['device_pixel_ratio']
    adb('shell','input','tap',str(round((item['x']+item['width']/2)*scale)),str(round((item['y']+item['height']/2)*scale)))

def screenshot(name):
    image=subprocess.run([args.adb,'exec-out','screencap','-p'],capture_output=True,check=True,timeout=30)
    args.report.with_name(name+'.png').write_bytes(image.stdout)

adb('shell','am','force-stop',package)
adb('shell','run-as',package,'rm','-f','files/menu-report.json')
adb('shell','am','start','-W','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.interactionCheck','true')
initial=wait_menu(lambda data:data.get('passed') and data.get('safe_top',0)>0)
tap(initial,'controls','moreButton')
more=wait_menu(lambda data:named(data,'menu_items','settingsMenuItem'))
if not all(item['inside_safe_area'] for item in more['menu_items']): raise RuntimeError('More menu intersects system UI')
screenshot('more-menu')
adb('shell','input','keyevent','4')
wait_menu(lambda data:not data['menu_items'])
tap(initial,'controls','filesButton')
files=wait_menu(lambda data:named(data,'menu_items','openWorkspaceMenuItem'))
if not all(item['inside_safe_area'] for item in files['menu_items']): raise RuntimeError('Files menu intersects system UI')
screenshot('files-menu')
tap(files,'menu_items','openWorkspaceMenuItem')
folder=wait_menu(lambda data:data.get('folder_dialog_visible'))
screenshot('workspace-chooser')
report['interaction']={'passed':True,'files_menu':files['menu_items'],'more_menu':more['menu_items'],'workspace_chooser_opened':True,'workspace_selected':False}
args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('Actual Android taps: More, Files and Open workspace passed')
adb('shell','input','keyevent','4')
adb('shell','am','force-stop',package)
