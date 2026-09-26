# HDL Eval on every platform

Windows, Linux, macOS and Android use `Studio::evaluateHardwareWithInputs` and
the same C++ HDL engine. There is no separate Android simulator implementation.
CI 36211395573 passed the shared GUI regressions on Windows x86-64, Linux x86-64,
macOS ARM64 and macOS Intel, plus the Android x86-64 Xor touch workflow.

1. Open the intended HDL file and choose Load & Eval HDL or Build.
2. Set the input pins in Machine. For Xor, a=0 and b=1 should produce out=1
   when all its dependencies implement their declared logic.
3. Press Eval. Look for the result and any loading/parser message above the pins.
4. Edited open HDL documents are taken from their visible buffers. Closed
   dependencies are read from the chip's folder; changed, added and removed files
   are checked on explicit Eval. Reload resets sequential clock/internal state.
5. Local HDL files override built-ins. Empty starter Not/And/Or implementations
   are valid incomplete exercises, not automatically solved circuits. The load
   warning lists empty dependencies. Complete them or explicitly use a separate
   fixture workspace with known implementations; do not overwrite course originals.

Parser failures retain the last valid simulation and replace the earlier success
message with an error. Eval is disabled during another running operation; Stop
that operation before evaluating. Ordinary combinational Eval does not advance
the clock; sequential circuits require Tick/Tock according to their specification.

For a reproducible report, include the release tag, operating system, complete
chip and local dependency files, input values, and exact load/Eval message.
Include whether the files were changed in NandStudio or another editor. These
details distinguish a stale snapshot, incomplete dependency, parser failure and
sequential-clock behavior without guessing from an unchanged output alone.
