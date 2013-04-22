#!/bin/bash
#
# killall-fail.sh
# Kills all remaining FailBochs instances on $FAIL_EXPERIMENT_HOSTS.
#
# Prerequisites:
# - (possibly overridden) env variables from fail-env.sh
#
# FIXME: unify with runcampaign.sh

set -e
# determine absolute path of this script
SCRIPTDIR=$(readlink -f $(dirname $0))
# env variable defaults
source $SCRIPTDIR/fail-env.sh

CMD="killall -q client.sh; killall -q fail-client"
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
done

wait
echo "Done."
