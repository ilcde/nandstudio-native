load ClockWire.hdl, output-file ClockWire.out, output-list time%S1.4.1 out;
eval, output;
repeat 4 { tick, output; tock, output; }
