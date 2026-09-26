// Independent NandStudio demonstration. GPL-3.0-or-later.
load EntryAlarm.hdl,
output-file EntryAlarm.out,
compare-to EntryAlarm.cmp,
output-list enabled%B1.7.1 door%B1.4.1 window%B1.6.1 alarm%B1.5.1;
set enabled 0, set door 0, set window 0, eval, output;
set enabled 0, set door 0, set window 1, eval, output;
set enabled 0, set door 1, set window 0, eval, output;
set enabled 0, set door 1, set window 1, eval, output;
set enabled 1, set door 0, set window 0, eval, output;
set enabled 1, set door 0, set window 1, eval, output;
set enabled 1, set door 1, set window 0, eval, output;
set enabled 1, set door 1, set window 1, eval, output;
