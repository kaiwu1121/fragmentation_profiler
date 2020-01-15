#!/bin/sh

if [ "$#" != "1" ];then
    echo "$0 <trace-file>"
    exit
fi

script_path="`dirname $0`"
result_file="`basename $0`"

filename_main="`echo $result_file|cut -d'.' -f1`"

outfile="$filename_main"".csv"

