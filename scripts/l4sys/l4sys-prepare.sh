#!/bin/bash

# Script to automate the preparation steps for an L4Sys experiment
#
# The L4Sys experiment manual lists four steps that need to be performed
# in order to launch an L4Sys campaign. This script automates the final
# three steps.
#
# Usage: Follow the manual. Find injection addresses and CR3 value and
#        adjust experimentInfo.hpp accordingly. Then run this script.
#
# TODO: Adjust $FAILDIR to your needs!

BAK=experimentInfo.hpp.bak
CFG=experimentInfo.hpp
FAIL_CMD="fail-client -q" # -rc bochs-dbg.rc"
FAILDIR=/home/doebel/src/fail
DBNAME=fail

if [ -n "$1" ] ; then
  if [ "$1" = "mem" ]; then
	IMPORTER=MemoryImporter;
  fi
fi

if [ "$IMPORTER" = "MemoryImporter" ]; then
	echo "Preparing memory injection experiment."
else
	IMPORTER=RegisterImporter;
	echo "Preparing register injection experiment."
fi

function buildfail {
	echo -e "\033[33mCompiling....\033[0m"
	cd $FAILDIR/build
	FAIL_BUILD_PARALLEL=16 $FAILDIR/scripts/rebuild-bochs.sh - >/dev/null
	cd -
}

function BuildNRun {
	buildfail
	echo -e "\033[33mRunning...\033[0m"
	$FAIL_CMD $@
}

# backup experiment config
cp $CFG $BAK

#echo -e "\033[35;1m[$(date)] ================== Step 0: Getting CR3 =================\033[0m"
#cat $BAK | sed -e 's/PREPARATION_STEP.*/PREPARATION_STEP 4/' >$CFG
#buildfail
#cr3=`$FAIL_CMD -f bochsrc-bd 2>/dev/null | grep CR3 | sed -e 's/ //g' | cut -d\= -f 2`
#echo \#defne L4SYS_ADDRESS_SPACE 0x$cr3
#cat $BAK | sed -e "s/L4SYS_ADDRESS_SPACE .*/L4SYS_ADDRESS_SPACE 0x$cr3/" >$CFG
#mv $CFG $BAK

echo -e "\033[35;1m[$(date)] ================== Step 1: Generating Snapshot =================\033[0m"
cat $BAK | sed -e 's/PREPARATION_STEP.*/PREPARATION_STEP 1/' >$CFG
BuildNRun -f bochsrc-bd

echo -e "\033[35;1m[$(date)] ================== Step 2: Get Instruction Count ===============\033[0m"
cat $BAK | sed -e 's/PREPARATION_STEP.*/PREPARATION_STEP 2/' >$CFG
buildfail

echo -e "\033[32mRunning...\033[0m"
values=`$FAIL_CMD 2>/dev/null | grep instructions\; | sed -e 's/.*after \(.*\) instructions;\(.*\) accepted/\1 \2/'`
echo $values
filtered_instr=`echo $values | cut -d\  -f 2`
total_instr=`echo $values | cut -d\  -f 1`

echo -e "\033[35;1m[$(date)] ================== Step 3: Golden Run ==========================\033[0m"
cat $BAK | sed -e 's/PREPARATION_STEP.*/PREPARATION_STEP 3/' >$CFG
BuildNRun

# now get ready to rumble...
echo -e "\033[35;1m[$(date)] ================== Step 4: Build Injection Client ==============\033[0m"
cat $BAK | sed -e "s/L4SYS_NUMINSTR.*/L4SYS_NUMINSTR $filtered_instr/" >$BAK.2
cat $BAK.2 | sed -e "s/L4SYS_TOTINSTR.*/L4SYS_TOTINSTR $total_instr/" >$BAK.3
cat $BAK.3 | sed -e "s/PREPARATION_STEP.*/PREPARATION_STEP 0/">$CFG
rm $BAK $BAK.2 $BAK.3
buildfail

echo -e "\033[35;1m[$(date)] ================== Step 5: Trace Import/Prune ==============\033[0m"
import-trace --importer $IMPORTER -e fiasco.image -d $DBNAME -t trace.pb
prune-trace -d $DBNAME

echo -e "\033[32;1m=========================================================================================="
echo "[$(date)] Preparations are finished. Happy injecting...."
echo -e "==========================================================================================\033[0m"
