# Web IDE comparison and bitmap editor

The [official web IDE](https://nand2tetris.github.io/web-ide/) was inspected on
2026-09-28. This is an incremental inventory, not an exhaustive parity claim.
The uploaded Java desktop distribution remains the compatibility reference for
legacy languages, dependency lookup and generated compiler output.

| Observed web interface | Native coverage | Remaining work |
| --- | --- | --- |
| Hardware: project/chip selection, HDL editor, Eval, Reset, clock, pins | Course workspace, editor and shared hardware engine | Complete component views and interactive script debugging |
| Hardware tests: load, step, run, rewind, speed, script/compare/output/diff tabs | Native batch scripts and comparison results | Pausable script state and equivalent diff inspection |
| Compiler: source open/add, Compile, Run | Single-document build and VM execution; directory CLI | Integrated directory build and verified handoff |
| Converter: binary, signed decimal, unsigned, hex, Hack ASM | Native numeric converter; separate instruction decoder | Combined ASM converter workflow |
| Bitmap: grid, resize, shift, rotate, flip, invert, Jack/ASM code | Native bitmap editor described below | Image import, crop/export modes and animation frames |
| CPU, assembler, VM, guide, settings and about navigation | Native tool panels, settings and packaged guides | Detailed control-by-control audit remains open |

## Native bitmap workflow

Choose **More > Bitmap editor**. Tap or drag across the grid to draw. Starting a
stroke on a black pixel erases; starting on a white pixel draws. Resize preserves
the upper-left part of the drawing. Shift clips pixels at the edge; rotation is
available for square canvases. Undo and redo retain up to 32 operations during
the application session.

Select Jack or Hack assembly, then **Copy generated code**. Create an ordinary
`Bitmap.jack` or `.asm` document, paste, save and build it using the existing
editor. The tool does not replace any project files automatically. Copying
replaces the system clipboard. Drawings themselves are not yet saved or recovered
after an application restart, so keep the generated source before closing.

Generated code writes the full canvas, including white pixels. Hack pixels are
least-significant-bit first. A partial final 16-pixel word is padded with white
pixels, so it also clears pixels beyond the drawing's right edge in that word.
Assembly writes from screen address 16384 and ends in a loop. Jack provides
`Bitmap.draw(location)`, where `location` is a screen-word offset, and requires
`Memory.poke` from a project OS or the supplied OS. Choose an offset that keeps
the drawing within the screen. Large generated programs may exceed Hack ROM
capacity; use smaller drawings or Jack for those cases.

The bitmap model and generators are portable C++; QML handles layout and input.
The utility is an addition inspired by the web workflow. Its generated source
is not claimed to match the web IDE byte for byte. Image import, keyboard pixel
navigation, persisted drawings, fit-to-drawing/rectangular exports, paused code
generation, configurable comments, animation shifts and alternate origins remain
open. Android touch and lifecycle behavior still require device qualification.

Core tests assemble and execute generated screen writes, check signed high-bit
values, compile Jack, exercise transforms and reject invalid dimensions. GUI tests
open the menu, draw using a Qt mouse event and check code updates and undo/redo.
See [the completion checklist](TODO.md) and [parity manifest](parity-manifest.json)
for the wider project and platform blockers.
