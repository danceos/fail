#!/bin/bash

# Script to automate the preparation steps for an L4Sys experiment

FAIL_CMD="fail-client" # -rc bochs-dbg.rc"
FAIL_ARGS="-f bochsrc-bd -q"
FAILDIR=/home/tstumpf/code/fail
BUILDDIR=/home/tstumpf/obj/fail
DBNAME=failtobias
BINDIR=~/local/bin

TYPE=$1

echo -e "\033[32;1m=========================================================================================="
echo "[$(date)] Prepare FI-Experiment ...."
echo -e "==========================================================================================\033[0m"

if [ -n $TYPE ] ; then
  if [ $TYPE = "mem" ]; then
		IMPORTER=MemoryImporter;
		echo "Preparing memory injection experiment."
  elif [ $TYPE = "reg" ]; then
		IMPORTER=RegisterImporter;
		echo "Preparing register injection experiment."
	else
		echo "Specified experiment type not knwon";
		exit
	fi
else
	echo "Specify your experiment type (mem/reg)"
	exit
fi

blink_addr=$(nm -C fiasco.image| grep blink | cut -d\  -f 1)
longjmp_addr=$(nm -C fiasco.image| grep longjmp | cut -d\  -f 1)


$BINDIR/fail-client -Wf,--step=all -Wf,--blink_addr=$blink_addr -Wf,--longjmp_addr=$longjmp_addr $FAIL_ARGS
$BINDIR/import-trace --importer $IMPORTER -e fiasco.image -d $DBNAME -t trace.pb
$BINDIR/prune-trace -d $DBNAME

echo -e "\033[32;1m=========================================================================================="
echo "[$(date)] Preparations are finished. Happy injecting...."
echo -e "==========================================================================================\033[0m"
