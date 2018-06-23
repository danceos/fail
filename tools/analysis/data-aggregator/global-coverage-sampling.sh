#!/bin/bash

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
SELECT benchmark, variant, method, resulttype, occurrences / n AS coverage,

         SQRT(
  occurrences / n * (1 - occurrences / n) / n) AS standard_error,
  1.96 * SQRT(
  occurrences / n * (1 - occurrences / n) / n) AS confidence95,
  2.326347874041 * SQRT(
  occurrences / n * (1 - occurrences / n) / n) AS confidence98,
  2.576 * SQRT(
  occurrences / n * (1 - occurrences / n) / n) AS confidence99,

  n

FROM
  (SELECT t.variant_id, v.benchmark, v.variant, g.fspmethod_id, m.method, CONCAT(r.resulttype) AS resulttype, SUM(g.weight) AS occurrences,
     (SELECT SUM(g2.weight)
      FROM fspgroup g2
      WHERE g2.variant_id = t.variant_id
        AND g2.fspmethod_id = m.id -- repeat subquery instead?
     ) AS n
   FROM variant v
   JOIN trace t ON v.id = t.variant_id
   JOIN fspgroup g ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
   JOIN fspmethod m ON g.fspmethod_id = m.id
   JOIN result_GenericExperimentMessage r ON r.pilot_id = g.pilot_id

   WHERE m.method = 'sampling'
   AND $FILTER

   GROUP BY t.variant_id, g.fspmethod_id, r.resulttype
  ) sub
GROUP BY variant_id, fspmethod_id, resulttype

UNION ALL

SELECT v.benchmark, v.variant, method, r.resulttype,
        SUM((t.time2-t.time1+1) * t.width)
                /
        (SELECT SUM(t.time2-t.time1+1)*t.width
        FROM result_GenericExperimentMessage r
        JOIN fspgroup g
          ON g.pilot_id = r.pilot_id
          AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
        JOIN trace t
          ON g.instr2 = t.instr2
          AND g.data_address = t.data_address
          AND g.variant_id = t.variant_id
        WHERE t.variant_id = v.id -- refers to parent query
        ) AS coverage,
  0,0,0,0,0
FROM result_GenericExperimentMessage r
JOIN fspgroup g
  ON g.pilot_id = r.pilot_id
JOIN fspmethod m
  ON m.id = g.fspmethod_id
  AND m.method = 'basic'
JOIN trace t
  ON g.instr2 = t.instr2
  AND g.data_address = t.data_address
  AND g.variant_id = t.variant_id
JOIN variant v
  ON t.variant_id = v.id
WHERE $FILTER
GROUP BY v.id, fspmethod_id, resulttype

ORDER BY benchmark, variant, method, resulttype
;
EOT
