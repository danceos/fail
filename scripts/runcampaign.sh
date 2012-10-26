#!/bin/bash
#
# runcampaign.sh <campaignserver-binary>
# Runs the campaign server, and launches clients on all $FAIL_EXPERIMENT_HOSTS.
#
# Prerequisites:
# - (possibly overridden) env variables from fail-env.sh
#

# runcampaign.sh <campaignserver-binary>
if [ ! -x "$1" ]; then echo "usage: $0 <campaignserver-binary>" >&2; exit 1; fi
CAMPAIGNSERVER_BINARY=$1

date
/usr/bin/time -po .time "$CAMPAIGNSERVER_BINARY" > $(basename "$CAMPAIGNSERVER_BINARY")_progress.txt 2>&1 &
sleep .1 # wait until server is likely to have a listening socket opened

$(dirname $0)/start-clients.sh

wait
echo "duration:"
cat .time
