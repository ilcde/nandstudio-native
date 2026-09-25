"""Install a development APK and verify actual Android toolbar safe-area bounds."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import time
import re
import xml.etree.ElementTree as ET

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('apk',type=pathlib.Path)
parser.add_argument('report',type=pathlib.Path)
parser.add_argument('--adb',default='adb')
parser.add_argument('--workspace-flow',action='store_true')
args=parser.parse_args()
package='org.qtproject.example.NandStudio'
def adb(*command,check=True):
    return subprocess.run([args.adb,*command],capture_output=True,text=True,encoding='utf-8',errors='replace',check=check,timeout=90)
if args.workspace_flow:
    fixture=args.report.parent/'Program.asm'
    fixture.parent.mkdir(parents=True,exist_ok=True)
    fixture.write_bytes(b'@2\r\nD=A\r\n')
    adb('shell','mkdir','-p','/sdcard/Download/NandStudioSmoke','/sdcard/Download/NandStudioExports')
    adb('push',str(fixture),'/sdcard/Download/NandStudioSmoke/Program.asm')
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
# Android may suspend Qt timers while its native document chooser owns focus.
# In that case verify the resumed system chooser instead of waiting on a timer.
chooser=None
for _ in range(60):
    folder=read_menu()
    activities=adb('shell','dumpsys','activity','activities').stdout
    resumed=[line.strip() for line in activities.splitlines() if 'ResumedActivity' in line and 'documentsui' in line.lower()]
    if resumed:
        chooser={'kind':'android-documents-ui','resumed_activity':resumed};break
    if folder and folder.get('folder_dialog_visible'):
        chooser={'kind':'qt-folder-dialog','visible':True};break
    time.sleep(0.5)
if chooser is None:
    args.report.with_suffix('.chooser.log').write_text(adb('logcat','-d').stdout,encoding='utf-8')
    raise RuntimeError('Open workspace touch did not open a folder chooser')
screenshot('workspace-chooser')
report['interaction']={'passed':True,'files_menu':files['menu_items'],'more_menu':more['menu_items'],'workspace_chooser_opened':True,'chooser':chooser,'workspace_selected':False}
if args.workspace_flow:
    # Native Android document UI is inspected separately from Qt's scene.
    def native_nodes():
        adb('shell','uiautomator','dump','/sdcard/nandstudio-ui.xml',check=False)
        xml=adb('exec-out','cat','/sdcard/nandstudio-ui.xml').stdout
        args.report.with_suffix('.ui.xml').write_text(xml,encoding='utf-8')
        return list(ET.fromstring(xml).iter('node'))

    def native_tap(predicate,attempts=12):
        for _ in range(attempts):
            for node in native_nodes():
                if predicate(node.attrib) and node.get('enabled')=='true':
                    bounds=[int(v) for v in re.findall(r'\d+',node.get('bounds',''))]
                    if len(bounds)==4:
                        adb('shell','input','tap',str((bounds[0]+bounds[2])//2),str((bounds[1]+bounds[3])//2));return True
            time.sleep(0.5)
        return False

    def select_folder(name):
        # Always choose Downloads from the drawer; no assumption about the
        # last selected provider/folder or filesystem conversion in the app.
        if not native_tap(lambda n:n.get('content-desc') in ('Show roots','Open navigation drawer')):
            raise RuntimeError('Document-provider drawer unavailable')
        if not native_tap(lambda n:n.get('text')=='Downloads'):
            raise RuntimeError('Downloads document provider unavailable')
        if not native_tap(lambda n:n.get('text')==name):
            raise RuntimeError('Fixture folder absent in Documents UI: '+name)
        if not native_tap(lambda n:n.get('text','').lower()=='use this folder'):
            raise RuntimeError('Folder selection action unavailable')
        if not native_tap(lambda n:n.get('text','').lower()=='allow'):
            raise RuntimeError('Android tree grant confirmation unavailable')

    try:
        select_folder('NandStudioSmoke')
        imported=wait_menu(lambda d:named(d,'workspace_controls','importWorkspaceDialogConfirm'))
        tap(imported,'workspace_controls','importWorkspaceDialogConfirm')
        ready=wait_menu(lambda d:d.get('workspace') and not d.get('busy') and named(d,'workspace_controls','workspaceFile_Program.asm'))
        workspace=ready['workspace']
        tap(ready,'workspace_controls','workspaceFile_Program.asm')
        editing=wait_menu(lambda d:d.get('active_document',{}).get('name')=='Program.asm' and named(d,'workspace_controls','editor_Program.asm'))
        tap(editing,'workspace_controls','editor_Program.asm')
        adb('shell','input','keycombination','KEYCODE_CTRL_LEFT','KEYCODE_A')
        adb('shell','input','text','@3')
        adb('shell','input','keyevent','KEYCODE_ENTER')
        adb('shell','input','text','D=A')
        wait_menu(lambda d:d.get('active_document',{}).get('text')=='@3\nD=A')
        adb('shell','input','keyevent','KEYCODE_BACK')
        tap(read_menu(),'controls','saveButton')
        wait_menu(lambda d:not d.get('active_document',{}).get('dirty',True))
        tap(read_menu(),'controls','buildButton')
        built=wait_menu(lambda d:d.get('active_document',{}).get('name')=='Program.hack' and not d.get('busy'))
        expected='0000000000000011\n1110110000010000\n'
        if built['active_document']['text']!=expected: raise RuntimeError('Native Android assembler output differs')
        # Save must affect the imported copy only.
        if adb('exec-out','cat','/sdcard/Download/NandStudioSmoke/Program.asm').stdout.replace('\r\n','\n')!='@2\nD=A\n':
            raise RuntimeError('Import modified original provider file')
        tap(built,'controls','filesButton')
        export_menu=wait_menu(lambda d:named(d,'workspace_controls','exportWorkspaceMenuItem'))
        tap(export_menu,'workspace_controls','exportWorkspaceMenuItem')
        select_folder('NandStudioExports')
        exported=wait_menu(lambda d:not d.get('busy') and 'Exported saved workspace as a new folder:' in d.get('output',''))
        candidates=adb('shell','find','/sdcard/Download/NandStudioExports','-name','Program.hack').stdout.splitlines()
        if len(candidates)!=1: raise RuntimeError('Expected one exported artifact')
        if adb('exec-out','cat',candidates[0]).stdout!=expected: raise RuntimeError('Exported machine code differs')
        if adb('exec-out','cat',candidates[0].replace('Program.hack','Program.asm')).stdout.replace('\r\n','\n')!='@3\nD=A':
            raise RuntimeError('Exported edited source differs')
        screenshot('workspace-exported')
        adb('shell','input','keyevent','KEYCODE_HOME')
        time.sleep(1)
        adb('shell','am','force-stop',package)
        adb('shell','run-as',package,'rm','-f','files/menu-report.json')
        adb('shell','am','start','-W','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.interactionCheck','true')
        reopened=wait_menu(lambda d:d.get('workspace')==workspace and d.get('active_document',{}).get('name')=='Program.hack')
        report['interaction']['workspace_selected']=True
        report['workspace_copy']={'passed':True,'provider':'Android Downloads','imported':True,'edited':True,'saved':True,'assembled':True,'exported':True,'reopened_after_process_restart':True,'original_unchanged':True,'all_providers_verified':False}
    except Exception:
        args.report.with_suffix('.workspace.log').write_text(adb('logcat','-d').stdout,encoding='utf-8')
        current=read_menu()
        if current: args.report.with_suffix('.workspace.json').write_text(json.dumps(current,indent=2),encoding='utf-8')
        screenshot('workspace-failure')
        raise
args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('Actual Android taps: More, Files and Open workspace passed')
adb('shell','input','keyevent','4')
adb('shell','am','force-stop',package)
