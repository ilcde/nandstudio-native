load Devices.hdl, output-file Devices.out,
output-list time%S1.4.1 pixels%D1.6.1 instruction%B1.16.1 key%D1.6.1 Screen[0]%D1.6.1;
ROM32K load Devices.asm, eval, output;
set romAddress 1, set in -1, set load 1, tick, output;
tock, output;
eval, output;
set Screen[0] 1, eval, output;
set romAddress 30, eval, output;
