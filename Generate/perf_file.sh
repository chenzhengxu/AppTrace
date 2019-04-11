#!/bin/bash

path=`pwd`

traceFile=$(find . -name "trace.json" -maxdepth 1 | xargs basename)

if [[ $traceFile == "" ]]; then
    echo "----------------------------------------"
    echo "没有找到trace.json文件，请确认"
    echo "----------------------------------------"
    exit 1
fi

cp $path/$traceFile $path/trace.json.tmp
sed '$ d' $path/trace.json.tmp > $path/$traceFile
rm -f $path/trace.json.tmp
exit 0