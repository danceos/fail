#!/bin/bash
set -e

TMP=$(mktemp)

read HEADER
echo "$HEADER"

BASE=''
LAST=''
sort -snk 1 | sort -snk 4 | sort -snk 3 > $TMP

while read CUR
do
	a=($CUR)
	C_I1=${a[0]}
	C_I2=${a[1]}
	C_ADDR=${a[2]}
	C_BIT=${a[3]}
	C_BW=${a[4]}
	C_COL=${a[5]}

	if [ -z "$BASE" ]
	then
		BASE=$CUR
		LAST=$CUR
	elif (($C_I1 != $L_I2 + 1 || $L_ADDR != $C_ADDR || $L_BIT != $C_BIT || $L_BW != $C_BW)) || \
		[ "$C_COL" != "$L_COL" ]
	then
		a=($BASE)
		B_I1=${a[0]}

		echo -e "$B_I1\t$L_I2\t$L_ADDR\t$L_BIT\t$L_BW\t$L_COL"

		BASE=$CUR
	fi

	LAST="$CUR"
	L_I1=$C_I1
	L_I2=$C_I2
	L_ADDR=$C_ADDR
	L_BIT=$C_BIT
	L_BW=$C_BW
	L_COL=$C_COL
done < $TMP

a=($BASE)
B_I1=${a[0]}

echo -e "$B_I1\t$L_I2\t$L_ADDR\t$L_BIT\t$L_BW\t$L_COL"

rm $TMP
