load RAM8.hdl, output-file MemoryTiming.out,
output-list time%S1.4.1 in%D1.6.1 load address%D1.1.1 out%D1.6.1 RAM8[3]%D1.6.1;
set address 3, set in 123, set load 1, eval, output;
tick, output;
eval, output;
set address 4, eval, output;
set address 3, eval, output;
tock, output;
set RAM8[3] -32768, output;
set load 0, tick, output;
tock, output;
