#!/bin/bash
cd $(dirname $0)
exec make CXX="ag++ -p $(pwd)/.. -I$(pwd)/../core/ -I$(pwd)/../build/core  --real-instances --Xcompiler" LIBTOOL="/bin/sh ./libtool --tag=CXX" "$@"
#exec make CXX="ag++ -p $(pwd)/.. --real-instances --Xcompiler" LIBTOOL="/bin/sh ./libtool --tag=CXX" "$@"
