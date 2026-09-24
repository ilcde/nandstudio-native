"""Release safety checks, using deliberately tiny non-executable fixtures."""
import copy
import importlib.util
import json
import pathlib
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('release', ROOT / 'scripts/prepare_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class ReleaseStaging(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        self.run = dict(status='completed', conclusion='success', head_branch='main',
                        event='push', head_sha='a'*40, html_url='https://example.invalid/run/1',
                        head_repository={'full_name': 'ilcde/nandstudio-native'})
        self.files = []
        for _, (artifact, pattern, _) in release.PACKAGES.items():
            path = self.root / 'artifacts' / artifact / pattern.removeprefix('**/')
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b'test-only-not-an-executable')
            self.files.append(path)
            if artifact.startswith('evidence-'):
                report = self.root / 'artifacts' / artifact / 'package-test/checks.json'
                report.parent.mkdir()
                report.write_text('[{"check":"fixture","passed":true}]')

    def stage(self, run=None):
        return release.stage(self.root/'artifacts', self.root/'staged', run or self.run, 'a'*40)

    def test_complete_set_has_all_six_targets_and_checksums(self):
        result = self.stage()
        self.assertEqual(len(result['assets']), 6)
        self.assertTrue(all(not a['full_workflow_verified'] for a in result['assets']))
        self.assertEqual(len((self.root/'staged/SHA256SUMS').read_text().splitlines()), 6)

    def test_missing_android_aborts_before_staging(self):
        self.files[-1].unlink()
        with self.assertRaises(ValueError): self.stage()
        self.assertFalse((self.root/'staged').exists())

    def test_failed_or_foreign_run_rejected(self):
        for field, value in [('conclusion', 'failure'), ('status', 'in_progress'),
                             ('head_sha', 'b'*40), ('head_branch', 'other'),
                             ('event', 'pull_request'),
                             ('head_repository', {'full_name': 'someone/fork'})]:
            run = copy.deepcopy(self.run)
            run[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError): self.stage(run)

    def test_failed_or_empty_packaged_tests_rejected(self):
        report = self.root/'artifacts/evidence-macos-14/package-test/checks.json'
        for checks in [[], [{'passed': False}], [{'passed': 'true'}]]:
            report.write_text(json.dumps(checks))
            with self.assertRaises(ValueError): self.stage()

    def test_existing_stage_is_never_overwritten(self):
        self.stage()
        with self.assertRaises(FileExistsError): self.stage()


if __name__ == '__main__': unittest.main()
