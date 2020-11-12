#!/bin/bash
## A cmake configuration call for a FailBochs pruning
EXP=$1
BUILDPATH=$2
USE_64=$3
DEBUG=$4
if [[ "$DEBUG" == "ON" ]]
then
    TYPE="Debug"
else
    TYPE="Release"
fi

if [[ "$EXP" == "generic-experiment" ]]
then
    BUILD_TOOLS='OFF'
else
    BUILD_TOOLS='ON'
fi

FAILPATH=$(dirname $0)/..
if [ -z $1 ]; then
    echo "Experiment not set. Usage: $0 <experiment name>"
    echo "Existing experiments:"
    find ${FAILPATH}/src/experiments -maxdepth 1 -mindepth 1 -type d -printf \-\>\ %P\\n | sort
else

    CONFIG='-DBUILD_64BIT:BOOL='${USE_64}' -DVERBOSE_MAKE:BOOL='${DEBUG}' -DCMAKE_BUILD_TYPE='${TYPE}' -DBUILD_SAIL:BOOL=ON -DBUILD_RISCV:BOOL=ON -DBUILD_BOCHS:BOOL=OFF -DBUILD_GEM5:BOOL=OFF -DBUILD_X86:BOOL=OFF -DBUILD_ARM:BOOL=OFF -DCONFIG_EVENT_BREAKPOINTS:BOOL=ON -DCONFIG_EVENT_IOPORT:BOOL=ON -DCONFIG_EVENT_MEMREAD:BOOL=ON -DCONFIG_EVENT_MEMWRITE:BOOL=ON -DCONFIG_EVENT_TRAP:BOOL=ON -DCONFIG_SR_RESTORE:BOOL=ON -DCONFIG_SR_SAVE:BOOL=ON -DEXPERIMENTS_ACTIVATED:STRING='${EXP}' -DBUILD_DUMP_TRACE:BOOL=OFF -DBUILD_IMPORT_TRACE:BOOL='${BUILD_TOOLS}' -DBUILD_PRUNE_TRACE:BOOL='${BUILD_TOOLS}' -DBUILD_LLVM_DISASSEMBLER='${BUILD_TOOLS}' -DCLIENT_RETRY_COUNT=1'

    if [ -e "${FAILPATH}/src/experiments/${EXP}/config.cmake" ]; then
        CONFIG="-C ${FAILPATH}/src/experiments/${EXP}/config.cmake ${CONFIG}"
    fi

    cmake ${CONFIG} "-B$BUILDPATH" "-H${FAILPATH}"
fi
