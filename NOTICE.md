# Notices and licensing

This is an incomplete native reimplementation of the Nand2Tetris desktop suite.
Original software architecture: Yaron Ukrainitz and Yannai A. Gonczarowski.
Original educational materials: Noam Nisan and Shimon Schocken, *The Elements of
Computing Systems*, MIT Press. Original launcher scripts: Mark Armbrust.

The Java reference source carries GPL-2.0-or-later notices. Those notices remain
intact in reference/. New native implementation files are GPL-3.0-or-later, a
compatible later GPL version; this does not relicense the reference source.
Qt open-source modules used here are distributed under their own LGPL/GPL terms.
Retain Qt's installed license texts and corresponding-source/relinking rights
when distributing binaries. GNU license texts are included under licenses/.
The complete Qt third-party notice and distribution audit remains pending.

The official website separately describes course materials as CC BY-NC-SA 3.0.
Uploaded projects and OS assets retain their original notices; no claim is made
that these educational assets have been relicensed under the code license.
Student projects and OS assets are preserved as reference materials, not installed by the application. The 35 original built-in HDL declarations are now copied verbatim under resources/hdl, embedded for offline native lookup, and installed under share/nandstudio/hdl with their original notices. This does not relicense those declarations under the native code license.

Sources: https://www.nand2tetris.org/license and reference/upstream/README.md.
Native files are new C++ implementations (2026), informed by reference behavior
and the source identified in docs/baseline.md. No Java engine is linked or invoked
by the native runtime. Release licensing audit remains incomplete.
