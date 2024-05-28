set env PAGET cat 
set pagination off
shell echo set logging file $(date +%Y%m%d_%H%M%S)_pid_$$.txt > tempfile.gdb
source tempfile.gdb
shell rm -rf tempfile.gdb
set logging on
