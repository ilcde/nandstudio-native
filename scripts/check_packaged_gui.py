"""Exercise a deployed GUI with its normal visible application window.

Hiding a window under test prevents native pointer events from reaching its
controls. No background/hidden launch flags are used for this interactive test.
"""
import argparse
import os
import pathlib
import subprocess
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('executable', type=pathlib.Path)
parser.add_argument('evidence', type=pathlib.Path)
args = parser.parse_args()
executable = args.executable.resolve(strict=True)
evidence = args.evidence.resolve()
evidence.mkdir(parents=True, exist_ok=True)
env = os.environ.copy()
for key in ('QT_PLUGIN_PATH', 'QML_IMPORT_PATH', 'QML2_IMPORT_PATH'):
    env.pop(key, None)
if sys.platform == 'win32':
    windows = env.get('SystemRoot', 'C:/Windows')
    env.update(PATH=windows+'/System32;'+windows, QT_QPA_PLATFORM='windows')
env.update(QT_QUICK_BACKEND='software', NAND_TEST_STATE_DIR=str(evidence/'state'))
result = subprocess.run([str(executable), '--self-test', str(evidence)],
                        cwd=executable.parent, env=env, timeout=90)
if result.returncode:
    log = evidence/'state/qt.log'
    if log.exists():
        print(log.read_text(encoding='utf-8', errors='replace'))
raise SystemExit(result.returncode)
