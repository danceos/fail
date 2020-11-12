#!/bin/bash
set -e

if [[ -z "$SAIL_ARCH" ]]
then
    echo "please set \$SAIL_ARCH to either riscv or cheri"
    exit 1
fi

if [[ "$SAIL_XLEN" == "64" ]]
then
    USE_64="ON"
elif [[ "$SAIL_XLEN" == "32" ]]
then
    USE_64="OFF"
else
    echo "please set \$SAIL_XLEN to 32 or 64."
    exit 1
fi

DEBUG=${SAIL_DEBUG:-OFF}

CONFIG_SCRIPT=configurations/sail_${SAIL_ARCH}.sh
TRACE_DIR=build_${SAIL_ARCH}_trace
EXP_DIR=build_${SAIL_ARCH}_exp
PACK_SCRIPT=configurations/pack.sh
PACK_DIR=packed/"${SAIL_ARCH}"/"${SAIL_XLEN}"/

SIM_DIR=simulators/sail/riscv-cheri
SIMEXTRA_DIR=simulators/sail/riscv/

echo "cleaning external dependecies"
make -C "$SIM_DIR" clean
make -C "$SIMEXTRA_DIR" clean

echo "cleaning build dirs"
rm -rf $TRACE_DIR/*
rm -rf $EXP_DIR/*

mkdir -p $TRACE_DIR
mkdir -p $EXP_DIR

echo "building trace"
"$CONFIG_SCRIPT" generic-tracing $TRACE_DIR $USE_64 $DEBUG
make -C $TRACE_DIR -j$(nproc --all)

echo "building experiment server"
"$CONFIG_SCRIPT" generic-experiment $EXP_DIR $USE_64 $DEBUG
make -C $EXP_DIR -j$(nproc --all)

echo "packing binaries"
"$PACK_SCRIPT" "$TRACE_DIR"/bin/fail-client trace
"$PACK_SCRIPT" "$TRACE_DIR"/bin/import-trace import
"$PACK_SCRIPT" "$TRACE_DIR"/bin/prune-trace prune
"$PACK_SCRIPT" "$EXP_DIR"/bin/fail-client client
"$PACK_SCRIPT" "$EXP_DIR"/bin/generic-experiment-server server
mkdir -p "$PACK_DIR"
mv {trace,import,prune,client,server}.tar.gz "$PACK_DIR"/
