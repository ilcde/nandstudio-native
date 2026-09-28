# Independent logic demonstrations

New NandStudio examples, GPL-3.0-or-later (2026). These are separate from the
unchanged course projects and are not assignment solutions.

- EntryAlarm: enable a door/window alarm; eight input combinations.
- SignalMismatch: detect two unequal signals; four input combinations.

Open an HDL file, choose Build, change inputs in Machine, then press Eval.
Run its matching TST file to check the complete truth table. Tests write an OUT
file next to the script and compare it with the supplied CMP file.

- BusSelect4: route two four-bit buses with bit slices and a selector.
- RememberBit: capture a bit on a clock cycle, then retain or replace it.

RememberBit distinguishes combinational Eval from Tick/Tock: Eval alone does
not store the input. The test checks initial state, capture, hold and replacement.
