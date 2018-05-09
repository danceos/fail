#!/bin/bash

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
SELECT v.benchmark, v.variant,
	MAX(t.time2)-MIN(t.time1)+1 AS duration,
	MAX(t.instr2)-MIN(t.instr1)+1 AS dyn_instr
FROM trace t
JOIN variant v
	ON t.variant_id = v.id
WHERE $FILTER
GROUP BY v.id
ORDER BY v.benchmark, v.variant
EOT
