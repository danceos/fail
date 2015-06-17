#!/bin/bash
#
# distribute-experiment.sh [path/to/experiment-target]
# Distribute necessary FailBochs ingredients for experiment target to
# FAIL_DISTRIBUTE_HOSTS.  Defaults to an experiment target in the current
# directory.
#
# Prerequisites:
# - (possibly overridden) env variables from fail-env.sh
#

set -e
# determine absolute path of this script
SCRIPTDIR=$(readlink -f $(dirname $0))
# env variable defaults
source $SCRIPTDIR/fail-env.sh

if [ -n "$1" ]; then cd "$1"; fi

# possibly necessary files
[ ! -e bochsrc ] && echo 'Warning: no bochsrc found' >&2
[ ! -e BIOS-bochs-latest ] && echo 'Warning: no BIOS-bochs-latest found' >&2
[ ! -e vgabios.bin ] && echo 'Warning: no vgabios.bin found' >&2

# necessary files
[ ! -e client.sh ] && cp -v $SCRIPTDIR/client.sh .
[ ! -e multiple-clients.sh ] && cp -v $SCRIPTDIR/multiple-clients.sh .

# add fail-client binary if it doesn't exist
if [ -e fail-client ]
then
	echo 'Info: Using local "fail-client" binary.' >&2
else
	cp -v $(which fail-client) .
	strip fail-client
fi

# sync everything to experiment hosts
for h in $FAIL_DISTRIBUTE_HOSTS
do
	echo Distributing to $h ...
	rsync -az --partial --delete-before --delete-excluded --exclude=core --exclude=*.tc --exclude=*.pb . $h:"$FAIL_EXPERIMENT_TARGETDIR" &
done

wait
echo "Done."
