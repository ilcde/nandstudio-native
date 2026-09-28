// Independent NandStudio example. GPL-3.0-or-later.
load RememberBit.hdl,
output-file RememberBit.out,
compare-to RememberBit.cmp,
output-list value%B1.5.1 capture%B1.7.1 remembered%B1.10.1;
set value 1, set capture 1, eval, output;
tick, tock, output;
set value 0, set capture 0, tick, tock, output;
set capture 1, tick, tock, output;
