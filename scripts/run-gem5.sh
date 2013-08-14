#!/bin/bash
#
# - needs to be called from within your build directory (i.e., $FAIL/build/)
# - expects 1 argument: path to experiment target (any location is possible)

../simulators/gem5/build/ARM/gem5.debug ../simulators/gem5/configs/example/fail_fs.py --kernel "$1"
