#!/bin/bash
#
# multiple-clients.sh [num_clients]
# Starts multiple client.sh instances in a new tmux session.  The number of
# clients defaults to #CPUs+1.
#
# Prerequisites:
#  - client.sh and all necessary FailBochs ingredients (fail-client binary,
#    bochsrc, BIOS/VGA-BIOS, boot image, possibly a saved state) in the current
#    directory
#  - tmux installed somewhere in $PATH
#  - possibly missing dynamic libraries in ~/bochslibs (e.g., for running a
#    i386 fail-client/bochs binary in an x86_64 environment)
#

set -e
umask 0077
LIBDIR=~/bochslibs

# cleanup earlier failures
# (FIXME: you probably don't want this on your local machine!)
killall -q client.sh || true
killall -q fail-client || true
sleep .5
killall -q -9 fail-client || true

# On many machines, ~ is mounted via NFS.  To avoid the (severe) performance
# penalty, copy all experiment-related stuff to /tmp.
TMP=/tmp/fail.$(id -nu)
mkdir -p $TMP
rsync -a --delete-before * $TMP/
if [ -d $LIBDIR ]
then
	rsync -a --delete-before $LIBDIR $TMP/
fi
cd $TMP

# tmux, please shut up.
TMUX='tmux -q'
COMMAND=./client.sh
SESSION=fail-client.$$

# Calculate number of clients from #processors.
PROCESSORS=$(fgrep processor /proc/cpuinfo|wc -l)
if [ -z "$1" ]
then
	NWIN=$PROCESSORS
else
	NWIN=$1
fi

# Run $NWIN clients in a new tmux session.
if [ $NWIN -ge 1 ]; then
	$TMUX new-session -s $SESSION -d "$COMMAND"
fi
for i in $(seq 1 $(($NWIN-1)))
do
	$TMUX new-session -t $SESSION -d \; split-window -h "$COMMAND"
	$TMUX new-session -t $SESSION -d \; select-layout tiled
done

# Some diagnostics for the operator.
echo "$(uname -n) ($PROCESSORS CPUs @ $(fgrep MHz /proc/cpuinfo|head -n1|awk '{print $(NF)}') MHz): started $NWIN clients"
#$TMUX attach -t $SESSION
