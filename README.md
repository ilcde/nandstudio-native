# NandStudio — native migration in progress

C++20 / Qt 6.11.1 development implementation of parts of the uploaded desktop
Nand2Tetris suite. **Incomplete; not a replacement release.** Android and remaining hardware
compatibility gaps are release blockers, alongside other gaps in the parity register.

The uploaded archive and extracted coursework remain unchanged. Native code is
under `src/`; original runtime and downloaded source references are under
`reference/`. No original Java engine is invoked by native application code.

On this Windows workstation:

```powershell
./scripts/build-windows.ps1
python scripts/audit_baseline.py --verify
python scripts/differential.py
./build/NandStudio.exe
```

Python and Java are development-only audit/reference-test dependencies.

* [Compatibility inventory](docs/feature-parity.md)
* [Baseline and provenance](docs/baseline.md)
* [Build and platform limitations](docs/build.md)
* [User guide](docs/user-guide.md)
* [Architecture](docs/architecture.md)
* [Progress and remaining work](docs/progress.md)
* [Notices](NOTICE.md)
