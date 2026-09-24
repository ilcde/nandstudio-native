load Positional.hdl, output-file Positional.out,
output-list right%B1.16.1 left%B1.16.1 result%B1.16.1;
set right %X1234, set left %X5555, eval, output;
set right -1, eval, output;
