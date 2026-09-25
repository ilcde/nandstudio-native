"""Stage same-run CI packages for an explicitly incomplete development release.

This script never publishes, downloads, signs, or replaces an existing release.
"""
import argparse
import hashlib
import json
import pathlib
import re
import shutil

PACKAGES = {
    'windows-x86_64': ('evidence-windows-2022', '**/NandStudio-windows-x86_64.zip', 'NandStudio-windows-x86_64.zip'),
    'linux-x86_64': ('evidence-ubuntu-24.04', '**/NandStudio-linux-x86_64.tar.xz', 'NandStudio-linux-x86_64.tar.xz'),
    'macos-arm64': ('evidence-macos-14', '**/NandStudio-macos-14.dmg', 'NandStudio-macos-arm64.dmg'),
    'macos-x86_64': ('evidence-macos-15-intel', '**/NandStudio-macos-15-intel.dmg', 'NandStudio-macos-x86_64.dmg'),
    'android-arm64-v8a': ('android-development-arm64-v8a', '**/outputs/apk/debug/android-build-debug.apk', 'NandStudio-android-arm64-v8a-development.apk'),
    'android-x86_64': ('android-development-x86_64', '**/outputs/apk/debug/android-build-debug.apk', 'NandStudio-android-x86_64-development.apk'),
}


def stage(artifacts, destination, run, expected_sha):
    if (run.get('status') != 'completed' or run.get('conclusion') != 'success'
            or run.get('head_branch') != 'main' or run.get('event') == 'pull_request'
            or run.get('head_sha') != expected_sha
            or run.get('head_repository', {}).get('full_name') != 'ilcde/nandstudio-native'
            or not re.fullmatch(r'[0-9a-f]{40}', expected_sha)):
        raise ValueError('Release requires a successful main-branch run at the checked-out source revision')
    # Check everything before creating the destination: missing targets must not
    # yield an apparently complete release directory.
    selected = []
    for platform, (artifact, pattern, name) in PACKAGES.items():
        folder = artifacts / artifact
        matches = list(folder.glob(pattern))
        if len(matches) != 1 or not matches[0].is_file() or matches[0].is_symlink():
            raise ValueError(f'{platform}: expected exactly one package, found {len(matches)}')
        if matches[0].stat().st_size == 0:
            raise ValueError(f'{platform}: empty package')
        checks = None
        if artifact.startswith('evidence-'):
            reports = list(folder.glob('**/package-test/checks.json'))
            if len(reports) != 1:
                raise ValueError(f'{platform}: packaged GUI test report missing or ambiguous')
            results = json.loads(reports[0].read_text(encoding='utf-8-sig'))
            if not isinstance(results, list) or not results or not all(r.get('passed') is True for r in results):
                raise ValueError(f'{platform}: packaged GUI tests failed or absent')
            checks = len(results)
        selected.append((platform, matches[0], name, checks))
    android_report_path = artifacts/'evidence-android-toolbar/toolbar.json'
    android_report = json.loads(android_report_path.read_text(encoding='utf-8'))
    android_apk = next(source for platform,source,_,_ in selected if platform=='android-x86_64')
    with android_apk.open('rb') as stream:
        android_hash = hashlib.file_digest(stream, 'sha256').hexdigest()
    interaction=android_report.get('interaction',{})
    if (android_report.get('passed') is not True or android_report.get('safe_top',0)<=0
            or interaction.get('passed') is not True or interaction.get('workspace_chooser_opened') is not True
            or android_report.get('workspace_copy',{}).get('passed') is not True
            or android_report.get('hdl_eval',{}).get('passed') is not True
            or android_report.get('apk_sha256') != android_hash):
        raise ValueError('Android toolbar, workspace touch and import/edit/build/export checks must pass for this exact x86_64 APK')
    destination.mkdir(parents=True, exist_ok=False)
    manifest = {'status': 'incomplete-development-prerelease', 'source_revision': expected_sha,
                'ci_run': run['html_url'], 'android_toolbar_evidence':'android-toolbar.json', 'assets': []}
    checksums = []
    for platform, source, name, checks in selected:
        target = destination / name
        shutil.copyfile(source, target)
        with target.open('rb') as stream:
            digest = hashlib.file_digest(stream, 'sha256').hexdigest()
        manifest['assets'].append({'platform': platform, 'file': name, 'sha256': digest,
                                   'packaged_gui_checks': checks,
                                   'signing': 'Android debug key' if platform.startswith('android') else 'unsigned',
                                   'full_workflow_verified': False})
        checksums.append(f'{digest}  {name}')
    shutil.copyfile(android_report_path,destination/'android-toolbar.json')
    checksums.append(hashlib.sha256((destination/'android-toolbar.json').read_bytes()).hexdigest()+'  android-toolbar.json')
    (destination / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    (destination / 'SHA256SUMS').write_text('\n'.join(checksums) + '\n', encoding='utf-8')
    return manifest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('artifacts', type=pathlib.Path)
    parser.add_argument('destination', type=pathlib.Path)
    parser.add_argument('--run', type=pathlib.Path, required=True)
    parser.add_argument('--sha', required=True)
    args = parser.parse_args()
    stage(args.artifacts, args.destination, json.loads(args.run.read_text()), args.sha)
