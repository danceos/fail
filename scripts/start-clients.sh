#!/bin/bash
#
# start-clients.sh
# Launches clients on all $FAIL_EXPERIMENT_HOSTS.
#
# Prerequisites:
# - (possibly overridden) env variables from fail-env.sh
#

set -e
# determine absolute path of this script
SCRIPTDIR=$(readlink -f $(dirname $0))
# env variable defaults
source $SCRIPTDIR/fail-env.sh

CMD="killall -q client.sh || true; cd $FAIL_EXPERIMENT_TARGETDIR; ./multiple-clients.sh"
CONNECTION_ATTEMPTS=2
SSH="ssh -o BatchMode=yes -o ConnectTimeout=60 -o ConnectionAttempts=$CONNECTION_ATTEMPTS"

for h in $FAIL_EXPERIMENT_HOSTS
do
	if [[ $h == *:* ]]
	then
		# split in host:nclients
		NCLIENTS=${h#*:}
		h=${h%:*}
	else
		NCLIENTS=
	fi

	$SSH $h "$CMD $NCLIENTS" &
	#disown
done

wait
echo 'done.'
