#!/bin/sh



perf stat -d -d  --interval-print 1000 \
		--event page-faults,major-faults,minor-faults,L1-dcache-load-misses,L1-dcache-loads,L1-dcache-stores,\
L1-icache-load-misses,LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores,branch-load-misses,branch-loads,\
dTLB-load-misses,dTLB-loads,dTLB-store-misses,dTLB-stores,\
iTLB-load-misses,iTLB-loads,\
branch-instructions,\
branch-misses,cache-misses,power/energy-cores/,power/energy-pkg/,power/energy-ram/ \
	sleep 6
			

