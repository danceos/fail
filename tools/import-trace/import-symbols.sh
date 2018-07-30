#!/bin/bash
# TODO: make this part of import-trace
set -e

# handle command line arguments
if [ ! $# -eq 4 ]; then
	echo "usage: $0 DATABASE VARIANT BENCHMARK ELF" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
ELF=$4
TABLE=symbol
MYSQL="mysql -B --quick $DATABASE"


# check if ELF exists
if [ ! -e $ELF ]; then
	echo "$ELF not found" >&2
	exit 1
fi
echo "importing $VARIANT/$BENCHMARK symbols from $ELF ..."


# get FAIL*'s variant_id
ID=$(echo "SELECT id FROM variant WHERE variant='$VARIANT' AND benchmark='$BENCHMARK'" | $MYSQL -N)
if [ -z $ID ]; then
	echo "no such variant/benchmark!" >&2
	exit 1
fi


# generate huge SQL statement and execute
$MYSQL <<EOT
CREATE TABLE IF NOT EXISTS $TABLE (
variant_id INT NOT NULL,
address INT UNSIGNED NOT NULL,
size INT UNSIGNED NOT NULL,
name VARCHAR(255) NOT NULL,
PRIMARY KEY (variant_id, address)
) ENGINE=MyISAM;
EOT
(
	# clean symbol table
	echo "DELETE FROM $TABLE WHERE variant_id = $ID;"

	LAST_ADDR=
	LAST_SYMBOL=

	LAST_INSERTED_ADDR=

	# The "dummy" entry at the end makes sure the last real symbol from the
	# nm output makes it into the database.
	(nm -n -S -C $ELF; echo ffffffff a dummy) \
	| egrep '^[0-9a-fA-F]' | while read line
	do
		ADDR=$(echo "$line"|awk '{print $1}')
		SIZE=$(echo "$line"|awk '{print $2}')
		if echo $SIZE | egrep -q '^[0-9a-fA-F]{8}'; then
			SYMBOL=$(echo "$line"|awk '{for (i=1; i<=NF-3; i++) $i = $(i+3); NF-=3; print}')
		else
			SIZE=unknown
			SYMBOL=$(echo "$line"|awk '{for (i=1; i<=NF-2; i++) $i = $(i+2); NF-=2; print}')
		fi

		if [ $LAST_ADDR ]; then
			# only create INSERT-line, if $ADDR is new to us
			if [ $LAST_ADDR != "$LAST_INSERTED_ADDR" ]; then
				echo "INSERT INTO $TABLE (variant_id, address, size, name) VALUES "
				echo "($ID, cast(0x$LAST_ADDR AS UNSIGNED), CAST(0x$ADDR AS UNSIGNED)-CAST(0x$LAST_ADDR AS UNSIGNED), '$LAST_SYMBOL');"
				LAST_INSERTED_ADDR=$LAST_ADDR
			fi
		fi

		if [ $SIZE != unknown ]; then
			if [ $ADDR != "$LAST_INSERTED_ADDR" ]; then
				echo "INSERT INTO $TABLE (variant_id, address, size, name) VALUES "
				echo "($ID, cast(0x$ADDR AS UNSIGNED), CAST(0x$SIZE AS UNSIGNED), '$SYMBOL');"
				LAST_ADDR=
				LAST_SYMBOL=
				LAST_INSERTED_ADDR=$ADDR
			fi
		else
			LAST_ADDR=$ADDR
			LAST_SYMBOL=$SYMBOL
		fi
	done
) | $MYSQL

echo "ANALYZE TABLE $TABLE;" | $MYSQL
