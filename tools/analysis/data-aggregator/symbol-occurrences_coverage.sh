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
	SELECT v.benchmark, v.variant, s.name, s.size, r.resulttype,
		SUM(t.time2-t.time1+1)
			/
		   (SELECT SUM(t.time2-t.time1+1)
			FROM variant v
			INNER JOIN symbol s ON v.id = s.variant_id
			INNER JOIN trace t ON t.variant_id = v.id AND t.data_address BETWEEN s.address AND s.address + s.size - 1
			INNER JOIN fspgroup g ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
			INNER JOIN result_GenericExperimentMessage r ON r.pilot_id = g.pilot_id
			WHERE v.variant="$VARIANT"
				AND v.benchmark="$BENCHMARK"
		   ) AS coverage
	FROM variant v
	INNER JOIN symbol s ON v.id = s.variant_id
	INNER JOIN trace t ON t.variant_id = v.id AND t.data_address BETWEEN s.address AND s.address + s.size - 1
	INNER JOIN fspgroup g ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
	INNER JOIN result_GenericExperimentMessage r ON r.pilot_id = g.pilot_id
	WHERE v.variant="$VARIANT"
		AND v.benchmark="$BENCHMARK"
	GROUP BY v.benchmark, v.variant, s.name, r.resulttype
	ORDER BY v.benchmark, v.variant, s.name, r.resulttype;
EOT
