#!/bin/bash
set -e

FIRST=vertical

[ $# -lt 2 -o $# -gt 3 ] && echo "usage: $0 input.csv output.csv [vertical|horizontal]" >&2 && exit 1

if [ -z $3 ]; then
	FIRST=vertical
else
	FIRST=$3
fi

TMP1=$(mktemp)
TMP2=$(mktemp)

cp $1 $TMP1
COUNT=$(wc -l <$TMP1)

NEXT=$FIRST

while true
do
	echo "at $COUNT, $NEXT ..."
	fsp.compact-$NEXT.sh < $TMP1 > $TMP2

	PREVCOUNT=$COUNT
	COUNT=$(wc -l <$TMP2)

	if (($COUNT >= $PREVCOUNT))
	then
		echo "no improvement (now $COUNT), stop."
		cp $TMP1 $2
		break
	fi

	mv $TMP2 $TMP1
	if [ $NEXT = vertical ]; then
		NEXT=horizontal
	else
		NEXT=vertical
	fi
done
