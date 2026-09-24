# Baseline, 24 September 2026

Workspace: `C:\Users\ilcde\Downloads\nand2teris_tool`. No Git repository or
AGENTS.md was found at the workspace or checked ancestor locations. The initial
workspace contained `nand2tetris.zip`, `nand2tetris/`, and `__MACOSX/`.

Uploaded archive SHA-256:
`22be29e048f1b90f58dc3c211bb870cc21ca4c680b6ecadff5d9db5bb213a8b4`.
881 non-directory entries, including Apple metadata; seven actual runtime JARs.
Existing extraction matches every archive file byte for byte. The separate
`reference/baseline/` extraction is never used as an output destination.
`scripts/audit_baseline.py --verify` verifies both copies against the archive.
It rejects traversal, absolute paths, drive names, duplicate case-folded targets,
symbolic links and excessive expanded sizes before extracting any entry.

`docs/baseline-inventory.json` records each archive file and every JAR member with
SHA-256. Actual project directories are `0` through `13`, not zero-padded names.
The ZIP contains compiled classes, resources, built-in HDL declarations and eight
compiled OS VM files. It does not contain editable Java sources. Student files
were not replaced or completed.

The [official software page](https://www.nand2tetris.org/software) distinguishes
desktop software, source distribution and browser IDE. Its source link supplied
`nand2tetris-open-source-2.5.7.zip`, saved at `reference/official-source.zip`:
SHA-256 `1b2d9096c711eb932edb9eb3b3573f4bd36ca5d9fd4d58ce9a083cef02b21364`.
That archive was independently validated before extraction.

The [official simulator repository](https://github.com/nand2tetris/nand2tetris_simulator)
was cloned at `327b7e4337b31efb815e1c1e3913c0efd299bcf2`. Of 257 Java files in
the source ZIP, 176 match this checkout after line-ending normalization; 81
differ or are absent. `source-comparison.json` records the list. **Neither source
tree is asserted to be a complete match to the uploaded binaries.** The Jack
compiler's implementation source is absent in both, although its runtime classes
are present in the uploaded Compilers.jar. Compiler compatibility is measured
against those binaries.

Important observed/source-supported details:

* Assembler aliases include NOTD/NOTA/NOTM, commutative forms and compact `[]`
  notation. Out-of-range numeric A operands become variable symbols. Duplicate
  labels replace previous definitions. These behaviors must not be tightened.
* CPU Emulator writes M through old A, updates A/D, then checks jumps through new
  A. This differs from the hardware CPU's simultaneous edge semantics. Native
  CPU traces preserve the emulator behavior.
* CPU restart preserves ordinary RAM but clears screen memory. A/PC script input
  rejects negative addresses, although CPU computations can produce negative A.
* TextComparer removes ASCII spaces throughout a line and trims low-valued edge
  characters. It does not erase internal tabs. Reports use zero-based lines.
* Script comparison is exact-width with single-character `*` wildcards. It is
  separate from TextComparer and reports one-based lines.
* The legacy VM symbol pass recognizes function/label declarations only at the
  start of the source line. The bundled NestedCall fixture has indented labels
  and fails baseline loading. Native now reproduces that rejection and diagnostic.
  This is a rejection-parity case, not a successful NestedCall execution test;
  the fixture was not modified.

Evidence precedence is uploaded executable behavior, verified corresponding
source, bundled docs, then supplementary official docs. Source findings lacking
binary tests remain provisional. Full GUI and extension audit is unfinished.
