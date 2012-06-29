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

CMD="killall -q client.sh"
SSH='ssh -o BatchMode=yes -o ConnectTimeout=60'
CONNECTION_ATTEMPTS=2

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

	(
		for i in $(seq $CONNECTION_ATTEMPTS)
		do
			$SSH $h "$CMD" && break
			# failed?  sleep 1-10s and retry.
			sleep $(($RANDOM / (32768 / 10) + 1))
			echo retrying $h ...
		done
	) &
done
