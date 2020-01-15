
Heap fragmentation profiler (external fragmentation)


LD_PRELOAD=./libfragprof.so fprof_max_runs=1000 fprof_max_size=2048
fprof_dump_interval=2 fprof_debug=1 <app> <args ...>
