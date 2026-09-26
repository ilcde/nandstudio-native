// Independent NandStudio demonstration. GPL-3.0-or-later.
load SignalMismatch.hdl,
output-file SignalMismatch.out,
compare-to SignalMismatch.cmp,
output-list left%B1.4.1 right%B1.5.1 alarm%B1.5.1;
set left 0, set right 0, eval, output;
set left 0, set right 1, eval, output;
set left 1, set right 0, eval, output;
set left 1, set right 1, eval, output;
