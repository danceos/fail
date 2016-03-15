#!/bin/bash
#
# fail-env.sh
# default values for several FAIL* environment variables
# If you want to set your own defaults, or need a script to source from, e.g.,
# your ~/.bashrc, please copy this file and do not edit it in-place.
#

# A whitespace-separated list of hosts to rsync the experiment data to.  This
# is not necessarily the same list as FAIL_EXPERIMENT_HOSTS (see below), as
# many hosts may share their homes via NFS.
export FAIL_DISTRIBUTE_HOSTS=${FAIL_DISTRIBUTE_HOSTS:-"ios kos virtuos plutonium bigbox.informatik.uni-erlangen.de ls12sp $(for hostnr in $(seq 39 58); do echo -n cloudhost$hostnr\ ; done)"}

# A whitespace-separated list of hosts to run experiments on.  If the host name
# is followed by a ':' and a number, this specifies the number of clients to
# run on that host (defaults to #CPUs).
export FAIL_EXPERIMENT_HOSTS=${FAIL_EXPERIMENT_HOSTS:-"bigbox.informatik.uni-erlangen.de plutonium:4 uran:4 virtuos ios:32 kos:16 bohrium:12 polonium:12 radon $(for hostnr in $(seq 100 254); do echo -n fiws$hostnr\ ; done) $(for hostnr in $(seq 39 58); do echo -n cloudhost$hostnr\ ; done)"}

# A homedir-relative directory on the distribution hosts where all necessary
# FAIL* ingredients reside (see multiple-clients.sh).
export FAIL_EXPERIMENT_TARGETDIR=${FAIL_EXPERIMENT_TARGETDIR:-.fail-experiment}

# Number of parallel build processes.  If unset, #CPUs+1.
if [ -z "$FAIL_BUILD_PARALLEL" ]; then
	if [ -e /proc/cpuinfo ]; then
		export FAIL_BUILD_PARALLEL=$(($(egrep -c ^processor /proc/cpuinfo)+1))
	else
		export FAIL_BUILD_PARALLEL=2
	fi
fi
