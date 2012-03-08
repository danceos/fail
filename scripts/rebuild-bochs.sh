#!/bin/bash
#
# - needs to be called from within your build directory
# - "rebuild-bochs.sh": rebuild all of both Fail and Bochs
#   (e.g., possibly necessary if you don't understand what was changed by others)
# - "rebuild-bochs.sh fail": rebuild all of Fail and re-link Bochs
#   (e.g., possibly necessary if you changed Fail-affecting aspects or the
#   build system)
# - "rebuild-bochs.sh bochs": rebuild all of Bochs
#   (e.g., necessary if you changed Bochs-affecting aspects/code that must be
#   inlined in Bochs)
# - "rebuild-bochs.sh -": rebuild only changed parts of Fail and Bochs
#   (e.g., sufficient if you only changed experiment code)
# - all of the previous options finally install Bochs
#
set -e

if [ "$1" = fail -o -z "$1" ]
then
	make clean
fi
if [ "$1" = bochs -o -z "$1" ]
then
	make bochsallclean
fi

#export PATH=/fs/staff/hsc/bin/ccache:$PATH

# even if we only rebuilt fail, we need to link and install bochs again
nice make -j10 bochs 2>&1 | $(dirname $0)/colorize.pl 2>&1
make bochsinstall
