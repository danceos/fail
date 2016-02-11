#!/bin/bash
set -e

if [ ! $# -eq 3 ]; then
	echo "usage: $0 DATABASE VARIANT BENCHMARK" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
# add "-t" for more readable output
MYSQL="mysql -B --quick $DATABASE"

$MYSQL << EOT
	SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
	SELECT r.resulttype, SUM((t.time2-t.time1+1) * t.width) AS occurrences
	FROM result_GenericExperimentMessage r
	INNER JOIN fspgroup g ON g.pilot_id=r.pilot_id
	INNER JOIN trace t ON g.instr2=t.instr2
		AND g.data_address=t.data_address
		AND g.variant_id=t.variant_id
	INNER JOIN variant v ON t.variant_id=v.id
	WHERE v.variant="$VARIANT"
		AND v.benchmark="$BENCHMARK"
	GROUP BY r.resulttype
	ORDER BY r.resulttype ASC;
EOT
