
Heap fragmentation profiler (external fragmentation)


Options:
fprof_opt_max_runs  --  the count of the max runs 
fprof_opt_dump_interval -- dump thread interval in seconds
fprof_opt_max_size -- trace file size limit in MB
fprof_opt_debug -- debug mode (verbose)


Launch command:
LD_PRELOAD=./libfprof.so fprof_opt_max_runs=1000 fprof_opt_max_size=2048 fprof_opt_dump_interval=2 fprof_opt_debug=1 <app> <args ...>

Example:

LD_PRELOAD=./libfprof.so fprof_opt_debug=1 ./test
