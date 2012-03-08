#!/bin/bash
#
# runcampaign.sh <campaignserver-binary>
# Runs the campaign server, and launches clients on all $FAIL_EXPERIMENT_HOSTS.
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
SSH='ssh -o BatchMode=yes -o ConnectTimeout=60'
CONNECTION_ATTEMPTS=2

# runcampaign.sh <campaignserver-binary>
if [ ! -x "$1" ]; then echo "usage: $0 <campaignserver-binary>" >&2; exit 1; fi
CAMPAIGNSERVER_BINARY=$1

date
/usr/bin/time -po .time "$CAMPAIGNSERVER_BINARY" >/dev/null 2>&1 &
sleep .1 # wait until server is likely to have a listening socket opened

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
			$SSH $h "$CMD $NCLIENTS" && break
			# failed?  sleep 1-10s and retry.
			sleep $(($RANDOM / (32768 / 10) + 1))
			echo retrying $h ...
		done
	) &
	disown
done

wait
echo "duration:"
cat .time
