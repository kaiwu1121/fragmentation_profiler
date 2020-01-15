#!/bin/sh


HTRACE_PATH="`dirname $0`""/.."

LD_PRELOAD="$HTRACE_PATH/libhtrace.so" $HTRACE_PATH/htrace_test


