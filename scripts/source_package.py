"""Deterministic source-only archive; excludes build state and reference engines.

The original runtime ZIP is included verbatim for the development reference audit.
Timestamps inside this new archive are fixed; the uploaded archive is never edited.
"""
import argparse
import hashlib
import pathlib
import stat
import zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
DIRECTORIES = ('src', 'resources', 'tests', 'scripts', 'docs', 'licenses', '.github')
FILES = ('CMakeLists.txt', 'CMakePresets.json', 'README.md', 'CONTRIBUTING.md', 'NOTICE.md',
         'toolchain-lock.json', '.gitignore', '.editorconfig', 'nand2tetris.zip')


def selected_files():
    paths = [ROOT / name for name in FILES if (ROOT / name).is_file()]
    for directory in DIRECTORIES:
        for path in (ROOT / directory).rglob('*'):
            relative = path.relative_to(ROOT)
            if path.is_symlink():
                raise ValueError(f'Source symlink requires explicit handling: {relative}')
            if not path.is_file() or '__pycache__' in path.parts:
                continue
            if relative.parts[:2] == ('tests', 'work'):
                continue
            paths.append(path)
    return sorted(paths, key=lambda p: p.relative_to(ROOT).as_posix())


def package(destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(destination, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in selected_files():
            name = path.relative_to(ROOT).as_posix()
            entry = zipfile.ZipInfo(name, (2000, 1, 1, 0, 0, 0))
            entry.create_system = 3
            entry.external_attr = (stat.S_IFREG | (0o755 if name.endswith('.sh') else 0o644)) << 16
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, path.read_bytes())
    return hashlib.sha256(destination.read_bytes()).hexdigest()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=pathlib.Path)
    args = parser.parse_args()
    print(package(args.destination), args.destination)
