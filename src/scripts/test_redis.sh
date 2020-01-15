#!/bin/sh

REDIS_SEVER=/home/cc/redis-5.0.5/src/redis-server
LIBHEAPTRACE=/home/cc/heaptrace/src/libhtrace.so
YCSB_PATH=/home/cc/ycsb-0.17.0
YCSB_WORKLOADS_PATH=/home/cc/heaptrace/src/scripts/ycsb_workloads
YCSB_WORKLOADS_LIST="a b c d e f"

rm dump.rdb 2>/dev/null

for _workload in $YCSB_WORKLOADS_LIST; do
	LD_PRELOAD=$LIBHEAPTRACE $REDIS_SEVER &

	workload="$YCSB_WORKLOADS_PATH""/""$_workload"

	echo "workload=$workload ..."

	$YCSB_PATH/bin/ycsb load redis -s -P $workload -p "redis.host=127.0.0.1" -p "redis.port=6379"  
	$YCSB_PATH/bin/ycsb run redis -s -P  $workload -p "redis.host=127.0.0.1" -p "redis.port=6379"  

	killall redis-server 2>/dev/null
	#rm dump.rdb 2>/dev/null

	exit 0
done
