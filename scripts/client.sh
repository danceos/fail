#!/bin/bash
#
# client.sh
# Starts a single FailBochs client in an endless loop.  Adds ./bochslibs to
# LD_LIBRARY_PATH if existent.  Terminates if ./stop exists, or bochs
# terminates with exit status 1.
#
# Prerequisite: All necessary FailBochs ingredients (see multiple-clients.sh)
# in the current directory.
#
rm -f stop

LIBDIR=$PWD/bochslibs
if [ -d $LIBDIR ]
then
	export LD_LIBRARY_PATH=$LIBDIR
fi

while [ ! -e stop ]
do
	# fiws fairness (don't use this host while someone else is logged in)
	# since these pool computers are used by students
	# NOTE: this will only work if fail-client terminates from time to time
	# TODO: only on Mo-Fr/20h-6h and Sa-So/24h ?
	if [[ $(uname -n) == fiws[1-2][0-9][0-9] ]]; then
		FIWS_FREE=0
		while [ "$FIWS_FREE" -eq 0 ]; do
			if [ -e stop ]; then
				#repeated exit condition from above
				exit 0
			fi
			FIWS_FREE=1
			for user in $(users); do
				if [[ "$user" != `whoami` ]]; then
					# someone else uses this host, don't do any experiments
					FIWS_FREE=0
				fi
			done
			if [ "$FIWS_FREE" -eq 0 ]; then
				sleep 300
			fi
		done
	fi

	# only start a client if at least 500 MiB is available
	FREE_MEM=$(LC_ALL=C free -m | grep "buffers/cache:" | awk '{print $4}')
	if [ $FREE_MEM -lt "500" ]; then
		# waiting for free memory. sleep 1-60s and retry.
		sleep $(($RANDOM / (32768 / 60) + 1))
	else
		#nice -n 19 ./bochs -q 2>&1 | tee log.$$.txt | fgrep Result
		#nice -n 18 ./bochs -q 2>&1 | fgrep Result
		nice -n 18 ./fail-client -q >/dev/null 2>&1
		if [ $? -gt 0 ] # exit on any error
		then
			break
		fi
	fi
done
echo DONE
