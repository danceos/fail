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
	#nice -n 19 ./bochs -q 2>&1 | tee log.$$.txt | fgrep Result
	#nice -n 18 ./bochs -q 2>&1 | fgrep Result
	nice -n 18 ./fail-client -q >/dev/null 2>&1
	if [ $? -eq 1 ]
	then
		break
	fi
done
echo DONE
