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


# get fail*'s variant_id
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
	nm -n -C $ELF | egrep '^[0-9a-fA-F]' | while read line
	do
		ADDR=$(echo "$line"|awk '{print $1}')
		SYMBOL=$(echo "$line"|awk '{for (i=1; i<=NF-2; i++) $i = $(i+2); NF-=2; print}')

		if [ $LAST_ADDR ]; then
			# only create INSERT-line, if $ADDR is new to us
			if [ ! $ADDR = $LAST_ADDR ]; then
				echo "INSERT INTO $TABLE (variant_id, address, size, name) VALUES "
				echo "($ID, cast(0x$LAST_ADDR AS UNSIGNED), CAST(0x$ADDR AS UNSIGNED)-CAST(0x$LAST_ADDR AS UNSIGNED), '$LAST_SYMBOL');"
			fi
		fi
		LAST_ADDR=$ADDR
		LAST_SYMBOL=$SYMBOL
	done
) | $MYSQL

echo "ANALYZE TABLE $TABLE;" | $MYSQL
