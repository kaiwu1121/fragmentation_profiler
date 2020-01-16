
Heap fragmentation profiler (external fragmentation)

Build:
git clone https://github.com/sudormroot/fragmentation_profiler.git
cd fragmentation_profiler/src
make

Command:
LD_PRELOAD=./libfprof.so fprof_opt_max_runs=1000 fprof_opt_max_size=2048 fprof_opt_dump_interval=2 fprof_opt_debug=1 <app> <args ...>



Example:
LD_PRELOAD=./libfprof.so fprof_opt_debug=1 ./test
LD_PRELOAD=./libfprof.so fprof_opt_debug=1 ../../amrex/Tutorials/Particles/CellSortedParticles/main3d.gnu.TPROF.MPI.ex ../../amrex/Tutorials/Particles/CellSortedParticles/inputs
