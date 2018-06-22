#!/bin/bash

MUST_FILTER=1
source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SELECT v.benchmark, v.variant, s.name, s.address, r.resulttype, SUM(t.time2-t.time1+1) AS occurrences
FROM variant v
JOIN symbol s
  ON s.variant_id = v.id
JOIN trace t
  ON t.variant_id = s.variant_id AND t.instr2_absolute BETWEEN s.address AND s.address + s.size - 1
JOIN fspgroup g
  ON g.variant_id = t.variant_id AND g.data_address = t.data_address AND g.instr2 = t.instr2
 AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
JOIN result_GenericExperimentMessage r
  ON r.pilot_id = g.pilot_id
WHERE $FILTER
GROUP BY s.variant_id, s.address, r.resulttype
ORDER BY s.address, CONCAT(r.resulttype)
;
EOT
