#!/bin/bash

#
# This script removes dangling rows from the database, for example 'trace'
# entries with a variant_id not mentioned in the 'variants' table, or result
# rows referencing a nonexistent 'fsppilot' entry.  IOW, this script enforces
# referential integrity as it would be maintained by foreign key constraints
# (that can only be used with InnoDB tables).
#

if [ -z "$1" -o -z "$2" ]
then
	echo "usage: $0 dbname resulttable" >&2
	exit 1
fi
DB=$1
RESULT=$2
MYSQL=mysql

function table_exists()
{
	N=$(echo "SHOW TABLES LIKE '$1'" | $MYSQL $DB | wc -l)
	[ $N -gt 0 ]
	return
}

if table_exists trace
then
	echo -n "removing widowed entries in trace ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM trace
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists fsppilot
then
	echo -n "removing widowed entries in fsppilot ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE p FROM fsppilot p
LEFT JOIN trace t
  ON p.variant_id = t.variant_id
 AND p.instr2 = t.instr2
 AND p.data_address = t.data_address
WHERE t.variant_id IS NULL;
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists fspgroup
then
	echo -n "removing widowed entries in fspgroup ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE g FROM fspgroup g
LEFT JOIN fsppilot p
  ON g.pilot_id = p.id
WHERE p.id IS NULL;
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists $RESULT
then
	echo -n "removing widowed entries in $RESULT ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE r FROM $RESULT r
LEFT JOIN fsppilot p
  ON r.pilot_id = p.id
WHERE p.id IS NULL;
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists objdump
then
	echo -n "removing widowed entries in objdump ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM objdump
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists fulltrace
then
	echo -n "removing widowed entries in fulltrace ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM fulltrace
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists dbg_source
then
	echo -n "removing widowed entries in dbg_source ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM dbg_source
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists dbg_filename
then
	echo -n "removing widowed entries in dbg_filename ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM dbg_filename
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists dbg_mapping
then
	echo -n "removing widowed entries in dbg_mapping ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM dbg_mapping
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

if table_exists symbol
then
	echo -n "removing widowed entries in symbol ..."
	echo " "$(
	$MYSQL $DB <<EOT
DELETE FROM symbol
WHERE variant_id NOT IN
(SELECT id FROM variant);
SELECT ROW_COUNT() AS "deleted rows:";
EOT
	)
fi

echo "done.  Consider running \`mysqloptimize $DB' now."
