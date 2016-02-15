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
SELECT v.benchmark, v.variant, f.path, r.resulttype, SUM(t.time2-t.time1+1) AS occurrences
FROM variant v
JOIN dbg_filename f
  ON f.variant_id = v.id
JOIN dbg_mapping m
  ON f.variant_id = m.variant_id AND f.file_id = m.file_id
JOIN trace t
  ON t.variant_id = m.variant_id AND t.instr2_absolute BETWEEN m.instr_absolute AND m.instr_absolute + m.line_range_size - 1
JOIN fspgroup g
  ON g.variant_id = t.variant_id AND g.data_address = t.data_address AND g.instr2 = t.instr2
JOIN result_GenericExperimentMessage r
  ON r.pilot_id = g.pilot_id
WHERE v.variant="$VARIANT"
  AND v.benchmark="$BENCHMARK"
GROUP BY v.id, f.file_id, r.resulttype
ORDER BY v.benchmark, v.variant, f.path, r.resulttype
;
EOT
