"""Bundle Qt and non-glibc runtime dependencies for the Linux development package."""
import pathlib, re, shutil, subprocess, sys
qt, package = map(lambda p: pathlib.Path(p).resolve(), sys.argv[1:])
lib = package / 'lib'
lib.mkdir(parents=True, exist_ok=True)
for directory in ('plugins', 'qml'):
    shutil.copytree(qt / directory, package / directory, dirs_exist_ok=True)
for path in (qt / 'lib').glob('libQt6*.so*'):
    shutil.copy2(path, lib / path.name, follow_symlinks=True)
# Keep the target's glibc/loader together. Target runtime: glibc >= 2.39.
base = re.compile(r'^(ld-linux|libc\.|libm\.|libpthread\.|libdl\.|librt\.|libresolv\.)')
pending = [p for p in package.rglob('*') if p.is_file() and ('.so' in p.name or p.parent.name == 'bin')]
seen = set()
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
        if base.match(dependency.name) or (lib / dependency.name).exists():
            continue
        dest = lib / dependency.name
        shutil.copy2(dependency, dest, follow_symlinks=True)
        pending.append(dest)
launcher = package / 'NandStudio.sh'
launcher.write_text('#!/bin/sh\nset -eu\nroot=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\nexport LD_LIBRARY_PATH="$root/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"\nexport QT_PLUGIN_PATH="$root/plugins"\nexport QML_IMPORT_PATH="$root/qml"\nexec "$root/bin/NandStudio" "$@"\n')
launcher.chmod(0o755)
(package / 'RUNTIME.txt').write_text('Linux x86-64 development bundle. Requires glibc >= 2.39 and a display server for interactive use. Qt, QML, plugins and discovered non-glibc libraries are bundled. Launch with NandStudio.sh.\n')
