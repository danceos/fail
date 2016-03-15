#!/bin/bash
#
# ab-fail-env.sh (Adrian Böckenkamp)
# default values for several FAIL* environment variables
#

# A whitespace-separated list of hosts to rsync the experiment data to.  This
# is not necessarily the same list as FAIL_EXPERIMENT_HOSTS (see below), as
# many hosts may share their homes via NFS.
export FAIL_DISTRIBUTE_HOSTS=${FAIL_DISTRIBUTE_HOSTS:='kos plutonium'}

# A whitespace-separated list of hosts to run experiments on.  If the host name
# is followed by a ':' and a number, this specifies the number of clients to
# run on that host (defaults to #CPUs+1).
export FAIL_EXPERIMENT_HOSTS=${FAIL_EXPERIMENT_HOSTS:="plutonium uran kos:6 bohrium polonium radon $(for hostnr in $(seq 100 254); do echo fiws$hostnr; done)"}

# A homedir-relative directory on the distribution hosts where all necessary
# FAIL* ingredients reside (see multiple-clients.sh).
export FAIL_EXPERIMENT_TARGETDIR=.fail-experiment
