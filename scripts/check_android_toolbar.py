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
    result=subprocess.run([args.adb,*command],capture_output=True,text=True,encoding='utf-8',errors='replace',check=False,timeout=90)
    if check and result.returncode:
        raise RuntimeError('adb '+repr(command)+': '+result.stdout+result.stderr)
    return result
if args.workspace_flow:
    fixture=args.report.parent/'Program.asm'
    fixture.parent.mkdir(parents=True,exist_ok=True)
    fixture.write_bytes(b'@2\r\nD=A\r\n')
    chip=fixture.with_name('Xor.hdl')
    chip.write_text('CHIP Xor { IN a,b; OUT out; PARTS: Not(in=a,out=Nota); Not(in=b,out=Notb); And(a=a,b=Notb,out=aAndNotb); And(a=Nota,b=b,out=NotaAndb); Or(a=aAndNotb,b=NotaAndb,out=out); }',encoding='utf-8')
    # sys.boot_completed can precede emulated shared storage becoming writable.
    for attempt in range(30):
        adb('shell','mkdir','-p','/sdcard/Download/NandStudioSmoke','/sdcard/Download/NandStudioExports',check=False)
        pushed=adb('push',str(fixture),'/sdcard/Download/NandStudioSmoke/Program.asm',check=False)
        if pushed.returncode==0: break
        time.sleep(1)
    else: raise RuntimeError('Android fixture storage unavailable: '+pushed.stdout+pushed.stderr)
    adb('push',str(chip),'/sdcard/Download/NandStudioSmoke/Xor.hdl')
adb('install','-r',str(args.apk))
adb('shell','am','force-stop',package)
# Delete only this tool's previous report, never app documents/settings.
adb('shell','run-as',package,'rm','-f','files/layout-report.json')
args.report.parent.mkdir(parents=True,exist_ok=True)
adb('logcat','-c')
launch=adb('shell','am','start','-W','-f','0x10008000','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.layoutCheck','true')
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
adb('shell','am','start','-W','-f','0x10008000','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.interactionCheck','true')
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
        # Never parse stale output after a failed dump or a transition frame.
        dump='/data/local/tmp/nandstudio-ui.xml'
        adb('shell','rm','-f',dump)
        result=adb('shell','uiautomator','dump','--compressed',dump,check=False)
        xml=adb('exec-out','cat',dump,check=False).stdout
        args.report.with_suffix('.ui.xml').write_text(xml,encoding='utf-8')
        args.report.with_suffix('.ui.log').write_text(result.stdout+result.stderr,encoding='utf-8')
        try: return list(ET.fromstring(xml).iter('node'))
        except ET.ParseError: return []

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
        # DocumentsUI can start at a protected storage root with a direct
        # Download child and no navigation drawer. Other launches start on a
        # provider page with a drawer. Wait for the root to finish rendering:
        # a single early dump can still show the previous Qt activity.
        model=adb('shell','getprop','ro.product.model').stdout.strip()
        if not native_tap(lambda n:n.get('text')=='Download' and n.get('resource-id')=='android:id/title'):
            if not native_tap(lambda n:n.get('content-desc') in ('Show roots','Open navigation drawer')):
                raise RuntimeError('Document-provider drawer unavailable')
            if not native_tap(lambda n:n.get('text')=='Downloads',attempts=2):
                # ACTION_OPEN_DOCUMENT_TREE can omit the Downloads root.
                if not native_tap(lambda n:n.get('text')==model and n.get('resource-id')=='android:id/title'):
                    raise RuntimeError('Internal storage document provider unavailable')
                if not native_tap(lambda n:n.get('text')=='Download'):
                    raise RuntimeError('Download child folder unavailable')
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
        # Exercise HDL through the same imported workspace and actual UI taps.
        tap(read_menu(),'workspace_controls','mobileFilesTab')
        chip_files=wait_menu(lambda d:named(d,'workspace_controls','workspaceFile_Xor.hdl'))
        tap(chip_files,'workspace_controls','workspaceFile_Xor.hdl')
        chip_editor=wait_menu(lambda d:d.get('active_document',{}).get('name')=='Xor.hdl')
        tap(chip_editor,'controls','buildButton')
        loaded=wait_menu(lambda d:d.get('hardware_state',{}).get('chip')=='Xor' and not d.get('busy') and named(d,'workspace_controls','togglePin_b'))
        tap(loaded,'workspace_controls','togglePin_b')
        tap(read_menu(),'workspace_controls','hardwareEval')
        evaluated=wait_menu(lambda d:d.get('hardware_evaluation')=='Eval completed: out=1' and not d.get('busy'))
        if not any(p['name']=='out' and p['value']==1 for p in evaluated['hardware_state']['pins']):
            raise RuntimeError('Xor Eval output did not update')
        screenshot('xor-evaluated')
        report['hdl_eval']={'passed':True,'chip':'reported composite Xor','inputs':{'a':0,'b':1},'out':1,'actual_taps':True}
        adb('shell','input','keyevent','KEYCODE_HOME')
        time.sleep(1)
        adb('shell','am','force-stop',package)
        adb('shell','run-as',package,'rm','-f','files/menu-report.json')
        adb('shell','am','start','-W','-f','0x10008000','-n',package+'/org.qtproject.qt.android.bindings.QtActivity','--ez','nandstudio.interactionCheck','true')
        reopened=wait_menu(lambda d:d.get('workspace')==workspace and d.get('active_document',{}).get('name')=='Xor.hdl')
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
