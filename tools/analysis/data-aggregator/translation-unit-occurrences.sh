#!/bin/bash

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SELECT v.benchmark, v.variant, f.path, r.resulttype, SUM(t.time2-t.time1+1) AS occurrences
FROM variant v
JOIN dbg_filename f
	ON f.variant_id = v.id
JOIN dbg_mapping m
	ON f.variant_id = m.variant_id
	AND f.file_id = m.file_id
JOIN trace t
	ON t.variant_id = m.variant_id
	AND t.instr2_absolute BETWEEN m.instr_absolute AND m.instr_absolute + m.line_range_size - 1
JOIN fspgroup g
	ON g.variant_id = t.variant_id
	AND g.data_address = t.data_address
	AND g.instr2 = t.instr2
	AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
JOIN result_GenericExperimentMessage r
	ON r.pilot_id = g.pilot_id
WHERE $FILTER
GROUP BY v.id, f.file_id, r.resulttype
ORDER BY v.benchmark, v.variant, f.path, CONCAT(r.resulttype)
;
EOT
