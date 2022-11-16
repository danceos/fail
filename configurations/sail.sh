#!/bin/bash
FAILPATH=$(dirname $0)/..
ARCH=$1; shift
DEBUG=$1; shift

USE_64=OFF
RISCV=OFF
RISCV_CHERI=OFF

case $ARCH in
    riscv32)
        RISCV=ON
        ;;
    riscv64)
        USE_64=ON
        RISCV=ON
        ;;
    cheri32)
        RISCV_CHERI=ON
        ;;
    cheri64)
        USE_64=ON
        RISCV_CHERI=ON
        ;;
    *)
        echo "Unknown ARCH argument"
        exit 1
        ;;
esac

case $DEBUG in
    ON)
        TYPE=Debug
        ;;

    OFF|"")
       TYPE=Release
        ;;

    *)
        echo "Unknown DEBUG Arg"
        exit 1
esac


cmake \
    -DCMAKE_AGPP_FLAGS:STRING="--c_compiler clang++" \
    -DBUILD_64BIT:BOOL="${USE_64}"\
    -DVERBOSE_MAKE:BOOL="${DEBUG}" \
    -DCMAKE_BUILD_TYPE="${TYPE}" \
    -DBUILD_SAIL:BOOL=ON \
    -DBUILD_RISCV_CHERI:BOOL=${RISCV_CHERI} \
    -DBUILD_RISCV:BOOL=${RISCV} \
    -DBUILD_BOCHS:BOOL=OFF \
    -DBUILD_GEM5:BOOL=OFF \
    -DBUILD_X86:BOOL=OFF \
    -DBUILD_ARM:BOOL=OFF \
    -DCONFIG_EVENT_BREAKPOINTS:BOOL=ON \
    -DCONFIG_EVENT_IOPORT:BOOL=ON \
    -DCONFIG_EVENT_MEMREAD:BOOL=ON \
    -DCONFIG_EVENT_MEMWRITE:BOOL=ON \
    -DCONFIG_EVENT_TRAP:BOOL=ON \
    -DCONFIG_SR_RESTORE:BOOL=ON \
    -DCONFIG_SR_SAVE:BOOL=ON \
    -DEXPERIMENTS_ACTIVATED:STRING="generic-tracing;generic-experiment" \
    -DPLUGINS_ACTIVATED:STRING="tracing;serialoutput" \
    -DBUILD_DUMP_TRACE:BOOL=ON \
    -DBUILD_IMPORT_TRACE:BOOL=ON \
    -DBUILD_PRUNE_TRACE:BOOL=ON \
    -DBUILD_LLVM_DISASSEMBLER=ON  \
    -DCLIENT_RETRY_COUNT=1 \
    -H${FAILPATH}
