load Delay.hdl, output-file Delay.out,
output-list time%S1.4.1 in first second inverse;
eval, output;
set in 1, tick, output;
tock, output;
set in 0, tick, output;
tock, output;
tick, output;
tock, output;
