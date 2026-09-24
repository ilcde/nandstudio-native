"""Development-only baseline audit. Never runs as part of the native application."""
import hashlib, io, json, pathlib, stat, sys, zipfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
def sha(data): return hashlib.sha256(data).hexdigest()
def checked_entries(archive, destination):
    result, seen, total = [], set(), 0
    for item in archive.infolist():
        name = item.filename.replace('\\', '/')
        path = pathlib.PurePosixPath(name)
        if path.is_absolute() or '..' in path.parts or ':' in name or '\x00' in name:
            raise ValueError('Unsafe archive path: ' + repr(name))
        target = (destination / name).resolve()
        if not target.is_relative_to(destination.resolve()):
            raise ValueError('Escaping archive path')
        if stat.S_ISLNK(item.external_attr >> 16): raise ValueError('Archive symlink')
        key = str(target).casefold()
        if key in seen: raise ValueError('Duplicate archive path')
        seen.add(key)
        total += item.file_size
        if item.file_size > 64 * 1024 * 1024 or total > 512 * 1024 * 1024:
            raise ValueError('Archive exceeds resource limit')
        result.append((item, target))
    return result

def main():
    archive_path = ROOT / 'nand2tetris.zip'
    dest = ROOT / 'reference' / 'baseline'
    records, jars, differences = [], {}, []
    with zipfile.ZipFile(archive_path) as archive:
        entries = checked_entries(archive, dest)
        for item, target in entries:
            if item.is_dir(): continue
            data = archive.read(item)
            records.append(dict(path=item.filename, size=len(data), sha256=sha(data)))
            if target.exists():
                if sha(target.read_bytes()) != sha(data): raise ValueError('Reference modified: ' + str(target))
            else:
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
            existing = ROOT / item.filename
            if not existing.is_file() or sha(existing.read_bytes()) != sha(data):
                differences.append(item.filename)
            if item.filename.startswith('nand2tetris/') and item.filename.endswith('.jar'):
                with zipfile.ZipFile(io.BytesIO(data)) as jar:
                    jars[item.filename] = [dict(path=e.filename, sha256=sha(jar.read(e))) for e in jar.infolist() if not e.is_dir()]
    report = dict(archive=archive_path.name, archive_sha256=sha(archive_path.read_bytes()),
                  files=records, jar_entries=jars, extracted_differences=differences)
    out = ROOT / 'docs' / 'baseline-inventory.json'
    if '--verify' in sys.argv:
        previous = json.loads(out.read_text(encoding='utf-8'))
        if previous != report: raise ValueError('Baseline inventory changed')
    else: out.write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(f'{len(records)} files; {len(jars)} JARs; {len(differences)} existing extraction differences; SHA256 {report["archive_sha256"]}')

if __name__ == '__main__': main()
