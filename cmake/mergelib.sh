#!/bin/bash
#
# Merge a list of static libraries (.a) and standalone objects (.o) into a
# common static library, and avoid .o naming conflicts.
#
set -e
shopt -s nullglob	# expand "*.o" to "" instead of "*.o" if no .o file exists

[ -z "$1" ] && echo "usage: $0 output.a input1.a input2.a input3.o ..." >&2 && exit 1
OUT=$1
shift

TMPDIR=$(mktemp -d)
COUNT=1

# collect files in tmpdir, assign unique names
for f in "$@"
do
	if echo "$f"|egrep -vq '\.[ao]'
	then
		echo "$0: can only merge .a/.o files, ignoring '$f'" >&2
		continue
	fi
	cp $f $TMPDIR/$(printf %03d $COUNT)$(basename $f)
	COUNT=$(($COUNT+1))
done

# create empty output lib
rm -f "$OUT"
ar rc "$OUT"

for lib in $TMPDIR/*.a
do
	echo "[FAIL*] Unpacking/merging: $(basename $lib) " >&2

	EXTRACTDIR=$TMPDIR/"$(basename $lib).dir"
	( # subshell to preserve cwd
	mkdir "$EXTRACTDIR"
	cd "$EXTRACTDIR"

	# unpack .o files to cwd
	ar x "$lib"

	# make sure the .o file names are unique
	for f in *.o
	do
		mv "$f" $(basename "$lib")_"$f"
	done
	)

	# move into merged library
	ar r "$OUT" "$EXTRACTDIR"/*.o
	rm -rf "$EXTRACTDIR"
done

ar r "$OUT" $TMPDIR/*.o

rm -rf "$TMPDIR" &
ranlib "$OUT"
