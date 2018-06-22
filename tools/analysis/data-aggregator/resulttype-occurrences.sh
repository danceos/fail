#!/bin/bash

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
SELECT v.benchmark, v.variant, r.resulttype, SUM((t.time2-t.time1+1) * t.width) AS occurrences
FROM result_GenericExperimentMessage r
INNER JOIN fspgroup g
	ON g.pilot_id = r.pilot_id
	AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
INNER JOIN trace t
	ON g.instr2 = t.instr2
	AND g.data_address = t.data_address
	AND g.variant_id = t.variant_id
INNER JOIN variant v
	ON t.variant_id=v.id
WHERE $FILTER
GROUP BY v.id, r.resulttype
ORDER BY v.benchmark, v.variant, CONCAT(r.resulttype)
;
EOT
