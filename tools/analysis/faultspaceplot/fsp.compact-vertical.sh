#!/bin/bash
set -e

TMP=$(mktemp)

read HEADER
echo "$HEADER"

BASE=''
LAST=''
sort -snk 4 | sort -snk 3 | sort -snk 1 > $TMP

while read CUR
do
	a=($CUR)
	C_I1=${a[0]}
	C_I2=${a[1]}
	C_ADDR=${a[2]}
	C_BIT=${a[3]}
	C_BW=${a[4]}
	C_COL=${a[5]}
	C_N=$(($C_ADDR*8 + $C_BIT))

	if [ -z "$BASE" ]
	then
		BASE=$CUR
		LAST=$CUR
	elif (($C_I1 != $L_I1 || $C_I2 != $L_I2 || \
	          $L_N + $L_BW != $C_N)) || \
		[ "$C_COL" != "$L_COL" ]
	then
		a=($BASE)
		B_ADDR=${a[2]}
		B_BIT=${a[3]}
		B_N=$(($B_ADDR*8 + $B_BIT))

		BW=$(($L_N + $L_BW - $B_N))

		echo -e "$L_I1\t$L_I2\t$B_ADDR\t$B_BIT\t$BW\t$L_COL"

		BASE=$CUR
	fi

	LAST="$CUR"
	L_I1=$C_I1
	L_I2=$C_I2
	L_ADDR=$C_ADDR
	L_BIT=$C_BIT
	L_BW=$C_BW
	L_COL=$C_COL
	L_N=$C_N
done < $TMP

a=($BASE)
B_ADDR=${a[2]}
B_BIT=${a[3]}
B_N=$(($B_ADDR*8 + $B_BIT))

BW=$(($L_N + $L_BW - $B_N))

echo -e "$L_I1\t$L_I2\t$B_ADDR\t$B_BIT\t$BW\t$L_COL"

rm $TMP
