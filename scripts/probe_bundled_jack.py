"""Compare every supplied project 9–11 Jack application with the Java compiler.

Development-only. All inputs and outputs live in disposable reference-test copies.
This verifies compiler output, not interactive application or native OS parity.
"""
import json
import shutil
import differential as d

shutil.copytree(d.BASE / 'tools', d.TOOLS, dirs_exist_ok=True)
for project in ('9', '10', '11'):
    root = d.ROOT / 'resources/starters/projects' / project
    for folder in sorted({path.parent for path in root.rglob('*.jack')}):
        sources = sorted(folder.glob('*.jack'))
        d.test('bundled-jack-' + project + '-' + folder.name,
               'JackCompiler', 'Hack.Compiler.JackCompiler',
               {path.name: path.read_bytes() for path in sources}, ['.'],
               [path.stem + '.vm' for path in sources])
destination = d.ROOT / 'docs/evidence/bundled-jack.json'
destination.write_text(json.dumps(d.report, indent=2) + '\n', encoding='utf-8')
print(f'{sum(row["passed"] for row in d.report)}/{len(d.report)} compiler comparisons passed')
raise SystemExit(not all(row['passed'] for row in d.report))
