load Feedback.hdl, output-file Feedback.out,
output-list time%S1.4.1 out n q;
eval, output;
repeat 5 { tick, output; tock, output; }
