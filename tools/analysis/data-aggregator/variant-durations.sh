#!/bin/bash
set -e

if [ "$1" = -t ]; then
	FORMAT=-t
	shift
else
	FORMAT=-B
fi

if [ $# -ne 3 -a $# -ne 1 ]; then
	echo "usage: $0 [ -t ] DATABASE [ VARIANT BENCHMARK ]" >&2
	echo "  -t      Display output in table format (tab-separated CSV otherwise)" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
MYSQL="mysql $FORMAT $DATABASE"

if [ -z "$VARIANT" ]; then
$MYSQL << EOT
	SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
	SELECT v.benchmark, v.variant,
	MAX(t.time2)-MIN(t.time1)+1 AS duration,
	MAX(t.instr2)-MIN(t.instr1)+1 AS dyn_instr
	FROM trace t
	JOIN variant v ON t.variant_id = v.id
	GROUP BY v.id
	ORDER BY v.benchmark, v.variant;
EOT
else
$MYSQL << EOT
	SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
	SELECT
	MAX(t.time2)-MIN(t.time1)+1 AS duration,
	MAX(t.instr2)-MIN(t.instr1)+1 AS dyn_instr
	FROM trace t
	JOIN variant v ON t.variant_id = v.id
	WHERE v.variant = "$VARIANT"
	  AND v.benchmark = "$BENCHMARK"
EOT
fi
