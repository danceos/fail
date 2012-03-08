#!/bin/bash
#
# Merge all static (.a) libraries into $LIBFAIL, and avoid .o naming conflicts.
#
set -e

LIBFAIL=libfail.a

cd "$1"
rm -f $LIBFAIL
ar rc $LIBFAIL

for lib in *.a
do
	[ "$lib" = "$LIBFAIL" ] && continue

	echo "[FAIL*] Unpacking/merging: $lib ";
	# unpack .o files to cwd
	ar x "$lib"

	# make sure the .o file names are unique
	for f in *.o
	do
		mv $f ${lib}_$f
	done

	# move into merged library
	ar r $LIBFAIL *.o
	rm -f *.o
done
