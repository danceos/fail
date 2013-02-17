#!/bin/bash
INSTDIR=/proj/i4danceos/ovp/current

source $INSTDIR/bin/setup.sh

setupImperas -m32 $INSTDIR 

export PATH=${PATH}:$IMPERAS_HOME/bin/$IMPERAS_ARCH
export IMPERASD_LICENSE_FILE=@faui49
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$IMPERAS_HOME/bin/$IMPERAS_ARCH:$IMPERAS_HOME/bin/$IMPERAS_HOME/External/lib
export IMPERASD_LICENSE_FILE=@faui49.informatik.uni-erlangen.de
