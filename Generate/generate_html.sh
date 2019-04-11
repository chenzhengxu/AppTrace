#!/bin/bash

path=`pwd`

sh ./perf_file.sh

generate_html() {
  echo "executing catapult/tracing/bin/trace2html $path/trace.json --output=$path/trace.html"
  python $path/catapult/tracing/bin/trace2html $path/trace.json --output=$path/trace.html

  echo "opening $path/trace.html"
  open $path/trace.html
  exit 0
}

if [ -d "catapult" ]; then
    generate_html
else
  echo "executing git clone https://github.com/catapult-project/catapult.git"
  git clone https://github.com/catapult-project/catapult.git
  generate_html
fi
