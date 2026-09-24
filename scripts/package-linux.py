"""Bundle Qt and non-glibc runtime dependencies for the Linux development package."""
import json, pathlib, re, shutil, subprocess, sys
qt, package = map(lambda p: pathlib.Path(p).resolve(), sys.argv[1:])
lib = package / 'lib'
lib.mkdir(parents=True, exist_ok=True)
licenses = package / 'licenses'
licenses.mkdir(exist_ok=True)
source_root = pathlib.Path(__file__).resolve().parents[1]
shutil.copytree(source_root / 'licenses', licenses, dirs_exist_ok=True)
shutil.copy2(source_root / 'NOTICE.md', package / 'NOTICE.md')
for path in (qt / 'licenses', qt.parent.parent / 'Licenses'):
    if path.is_dir():
        shutil.copytree(path, licenses / 'Qt', dirs_exist_ok=True)
for directory in ('plugins', 'qml'):
    shutil.copytree(qt / directory, package / directory, dirs_exist_ok=True)
for path in (qt / 'lib').glob('libQt6*.so*'):
    shutil.copy2(path, lib / path.name, follow_symlinks=True)
# Keep the target's glibc/loader together. Target runtime: glibc >= 2.39.
base = re.compile(r'^(ld-linux|libc\.|libm\.|libpthread\.|libdl\.|librt\.|libresolv\.)')
pending = [p for p in package.rglob('*') if p.is_file() and ('.so' in p.name or p.parent.name == 'bin')]
seen = set()
system_dependencies = set()
while pending:
    path = pending.pop()
    if path in seen:
        continue
    seen.add(path)
    result = subprocess.run(['ldd', str(path)], capture_output=True, text=True)
    for line in result.stdout.splitlines():
        match = re.search(r'=> (/\S+)', line)
        if not match:
            continue
        dependency = pathlib.Path(match[1])
        if not dependency.is_relative_to(package) and not dependency.is_relative_to(qt):
            system_dependencies.add(dependency.resolve())
        if base.match(dependency.name) or (lib / dependency.name).exists():
            continue
        dest = lib / dependency.name
        shutil.copy2(dependency, dest, follow_symlinks=True)
        pending.append(dest)
records = []
for dependency in sorted(system_dependencies):
    result = subprocess.run(['dpkg-query','-S',str(dependency)], capture_output=True, text=True)
    for line in result.stdout.splitlines():
        owner = line.split(': ', 1)[0]
        notice = pathlib.Path('/usr/share/doc') / owner.split(':')[0] / 'copyright'
        if notice.is_file():
            shutil.copy2(notice, licenses / (owner.replace(':','-') + '-copyright'))
            records.append({'library': str(dependency), 'package': owner, 'notice': notice.as_posix()})
(package / 'system-library-notices.json').write_text(json.dumps(records, indent=2))
launcher = package / 'NandStudio.sh'
launcher.write_text('#!/bin/sh\nset -eu\nroot=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\nexport LD_LIBRARY_PATH="$root/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"\nexport QT_PLUGIN_PATH="$root/plugins"\nexport QML_IMPORT_PATH="$root/qml"\nexec "$root/bin/NandStudio" "$@"\n')
launcher.chmod(0o755)
(package / 'RUNTIME.txt').write_text('Linux x86-64 development bundle. Requires glibc >= 2.39 and a display server for interactive use. Qt, QML, plugins and discovered non-glibc libraries are bundled. Launch with NandStudio.sh.\n')
