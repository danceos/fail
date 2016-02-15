#!/bin/bash
set -e

if [ "$1" = -t ]; then
	FORMAT=-t
	shift
else
	FORMAT=-B
fi

if [ ! $# -eq 3 ]; then
	echo "usage: $0 [ -t ] DATABASE VARIANT BENCHMARK" >&2
	echo "  -t      Display output in table format (tab-separated CSV otherwise)" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
MYSQL="mysql $FORMAT $DATABASE"

$MYSQL << EOT
SELECT s.name, s.address, r.resulttype, SUM(t.time2-t.time1+1) AS occurrences
FROM variant v
JOIN symbol s
  ON s.variant_id = v.id
JOIN trace t
  ON t.variant_id = s.variant_id AND t.instr2_absolute BETWEEN s.address AND s.address + s.size - 1
JOIN fspgroup g
  ON g.variant_id = t.variant_id AND g.data_address = t.data_address AND g.instr2 = t.instr2
JOIN result_GenericExperimentMessage r
  ON r.pilot_id = g.pilot_id
WHERE v.variant="$VARIANT"
  AND v.benchmark="$BENCHMARK"
GROUP BY s.variant_id, s.address, r.resulttype
ORDER BY s.address, r.resulttype
;
EOT
