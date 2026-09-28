// Independent NandStudio example. GPL-3.0-or-later.
load BusSelect4.hdl,
output-file BusSelect4.out,
compare-to BusSelect4.cmp,
output-list first%B1.5.1 second%B1.6.1 chooseSecond%B1.12.1 selected%B1.8.1;
set first 0, set second 15, set chooseSecond 0, eval, output;
set first 0, set second 15, set chooseSecond 1, eval, output;
set first 5, set second 10, set chooseSecond 0, eval, output;
set first 5, set second 10, set chooseSecond 1, eval, output;
set first 9, set second 6, set chooseSecond 0, eval, output;
set first 9, set second 6, set chooseSecond 1, eval, output;
set first 15, set second 0, set chooseSecond 0, eval, output;
set first 15, set second 0, set chooseSecond 1, eval, output;
