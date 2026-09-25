# Contributing

Compatibility with the uploaded desktop baseline takes priority over new syntax
or optimizations. Preserve student starters, expected comparisons and attribution.
Native engines must not invoke Java tools at runtime; Java is for reference tests.

## Report a problem

Use [GitHub Issues](https://github.com/ilcde/nandstudio-native/issues). Include:

- Release tag/source commit and package name.
- OS version, architecture and Android API/device model where applicable.
- Minimal source and required local dependencies, exact steps and input values.
- Expected/actual results and whether the original Java tools behave alike.
- Relevant console output or screenshots, with private data removed.

Empty student dependencies are not evidence of an engine defect. Avoid uploading
an entire private workspace when a minimal reproduction suffices.

## Make a change

Read the [build guide](docs/build.md), [architecture](docs/architecture.md) and
[parity register](docs/parity-manifest.json). Keep commits focused and add regression
evidence for behavior changes. Test core code headlessly and changed GUI controls
through actual interactions. Preserve encoding and ordinary folder layouts.

Record original evidence, implementation, tests, platform coverage and remaining
differences. Do not change expected outputs to obtain a pass. Put independent
fixtures outside coursework and record provenance. Never commit signing keys,
tokens, local recovery data or build trees. See [notices](NOTICE.md) and `licenses/`.

CI covers six package targets, packaged desktop tests, sanitizers and an Android
toolbar runtime gate. Development publication is not complete parity certification.
