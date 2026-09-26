"""Check maintained Markdown encoding, local links and common corruption markers.

This is a mechanical documentation check, not a substitute for editorial review.
Original educational resources and historical test diagnostics are not rewritten.
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
files = sorted((root / 'docs').rglob('*.md')) + [root / name for name in
    ('README.md', 'NOTICE.md', 'CONTRIBUTING.md')]
errors = []
for path in files:
    try:
        text = path.read_text(encoding='utf-8')
    except UnicodeError as error:
        errors.append(f'{path.relative_to(root)}: invalid UTF-8: {error}')
        continue
    for marker in ('\ufffd', '\u00c3', '\u00c2', '\u00e2\u20ac'):
        if marker in text:
            errors.append(f'{path.relative_to(root)}: possible encoding corruption {marker!r}')
    for target in re.findall(r'\]\(([^)]+)\)', text):
        target = target.split('#', 1)[0]
        if not target or '://' in target or target.startswith('mailto:'):
            continue
        if not (path.parent / target).exists():
            errors.append(f'{path.relative_to(root)}: missing local link {target}')
print(f'Checked {len(files)} maintained Markdown files')
for error in errors:
    print(error)
sys.exit(bool(errors))
