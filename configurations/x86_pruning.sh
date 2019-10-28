#!/bin/bash
## A cmake configuration call for a FailBochs pruning
EXP=$1
FAILPATH=$(dirname $0)/..
if [ -z $1 ]; then
    echo "Experiment not set. Usage: $0 <experiment name>"
    echo "Existing experiments:"
    find ${FAILPATH}/src/experiments -maxdepth 1 -mindepth 1 -type d -printf \-\>\ %P\\n | sort
else

    CONFIG='-DBUILD_BOCHS:BOOL=ON -DBUILD_X86:BOOL=ON -DBUILD_GEM5:BOOL=OFF -DBUILD_ARM:BOOL=OFF -DBUILD_SIMULAVR:BOOL=OFF -DBUILD_AVR:BOOL=OFF -DCONFIG_BOCHS_NO_ABORT:BOOL=ON -DCONFIG_EVENT_BREAKPOINTS:BOOL=ON -DCONFIG_EVENT_BREAKPOINTS_RANGE:BOOL=ON -DCONFIG_EVENT_INTERRUPT:BOOL=ON -DCONFIG_EVENT_IOPORT:BOOL=ON -DCONFIG_EVENT_MEMREAD:BOOL=ON -DCONFIG_EVENT_MEMWRITE:BOOL=ON -DCONFIG_EVENT_TRAP:BOOL=ON -DCONFIG_SR_RESTORE:BOOL=ON -DCONFIG_SR_SAVE:BOOL=ON -Dbochs_configure_params:STRING="--enable-a20-pin;--enable-x86-64;--enable-cpu-level=6;--enable-ne2000;--enable-acpi;--enable-pci;--enable-usb;--enable-trace-cache;--enable-fast-function-calls;--enable-host-specific-asms;--enable-disasm;--enable-readline;--enable-clgd54xx;--enable-fpu;--enable-vmx=2;--enable-monitor-mwait;--enable-cdrom;--enable-sb16=linux;--enable-gdb-stub;--with-nogui" -DEXPERIMENTS_ACTIVATED:STRING='${EXP}' -DBUILD_IMPORT_TRACE:BOOL=ON -DBUILD_PRUNE_TRACE:BOOL=ON -DBUILD_LLVM_DISASSEMBLER=ON'

    if [ -e "${FAILPATH}/src/experiments/${EXP}/config.cmake" ]; then
        CONFIG="-C ${FAILPATH}/src/experiments/${EXP}/config.cmake -DEXPERIMENTS_ACTIVATED:STRING=${EXP}"
    fi

    cmake ${CONFIG} ${FAILPATH}
fi
