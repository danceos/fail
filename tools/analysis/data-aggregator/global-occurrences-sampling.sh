#!/bin/bash

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
-- quantile function: http://en.wikipedia.org/wiki/Normal_distribution#Quantile_function
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
SELECT benchmark, variant, method, resulttype, occurrences,

         SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS standard_error,
  1.96 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence95,
  2.326347874041 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence98,
  2.576 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence99,
/*  2.807 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence995,
  3.090 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence998,
  3.291 * SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea AS confidence999,

         SQRT(
  occurrences_r / fullarea * (1 - occurrences_r / fullarea) / n
  ) * fullarea / occurrences AS RSE,

occurrences_r, fullarea, */ n

FROM
(
SELECT variant_id, benchmark, variant, method, resulttype,
  SUM(occurrences *
      -- extrapolation factor
      (SELECT SUM(t2.time2-t2.time1+1) -- complete area
       FROM trace t2
       WHERE t2.accesstype = sub.accesstype AND t2.variant_id = sub.variant_id)
      /
      (SELECT SUM(g.weight) FROM trace t2 -- sampled area for this resulttype + accesstype
       JOIN fspgroup g ON t2.variant_id = g.variant_id AND t2.data_address = g.data_address AND t2.instr2 = g.instr2
       WHERE g.fspmethod_id = sub.fspmethod_id AND t2.accesstype = sub.accesstype AND t2.variant_id = sub.variant_id)
     ) AS occurrences,

  SUM(occurrences *
      IF(sub.accesstype = 'R',

      -- extrapolation factor (same as above)
      (SELECT SUM(t2.time2-t2.time1+1)
       FROM trace t2
       WHERE t2.accesstype = sub.accesstype AND t2.variant_id = sub.variant_id)
      /
      (SELECT SUM(g.weight) FROM trace t2
       JOIN fspgroup g ON t2.variant_id = g.variant_id AND t2.data_address = g.data_address AND t2.instr2 = g.instr2
       WHERE g.fspmethod_id = sub.fspmethod_id AND t2.accesstype = sub.accesstype AND t2.variant_id = sub.variant_id)

      , 0)
     ) AS occurrences_r,

  (SELECT SUM(g.weight)    -- * 8
   FROM fspgroup g
   JOIN trace t
   ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
   WHERE g.variant_id = sub.variant_id AND g.fspmethod_id = sub.fspmethod_id AND t.accesstype = 'R'
  ) AS n, -- #experiments for Reads
   
  (SELECT SUM(t2.time2-t2.time1+1)  -- * 8 -- 8 experiments per byte, must be removed for burst experiments
   FROM trace t2
   WHERE t2.accesstype = 'R' AND t2.variant_id = sub.variant_id
  ) AS fullarea -- R area to extrapolate to

FROM
  (SELECT t.variant_id, v.benchmark, v.variant, g.fspmethod_id, m.method, r.resulttype, t.accesstype, SUM(g.weight) AS occurrences
   FROM variant v
   JOIN trace t ON v.id = t.variant_id
   JOIN fspgroup g ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
   JOIN fspmethod m ON g.fspmethod_id = m.id
   JOIN result_GenericExperimentMessage r ON r.pilot_id = g.pilot_id

   WHERE m.method = 'sampling'
   AND $FILTER

   GROUP BY t.variant_id, g.fspmethod_id, r.resulttype, t.accesstype
  ) sub
-- WHERE resulttype IN ('TRAP', 'SDC', 'TIMEOUT')
-- GROUP BY variant_id, fspmethod_id --   , resulttype
GROUP BY variant_id, fspmethod_id, resulttype
) sub2

UNION ALL

-- ------------------------------------ basic ------------------------------------

SELECT benchmark, variant, method, resulttype, SUM(t.time2-t.time1+1) AS occurrences,
 0 AS standard_error,
 0 AS confidence95,
 0 AS confidence98,
 0 AS confidence99,
/*
 0 AS confidence995,
 0 AS confidence998,
 0 AS confidence999,
 0 AS RSE,
 0 AS occurrences_r,
(SELECT SUM(t2.time2-t2.time1+1) *8 FROM trace t2 WHERE t2.variant_id = t.variant_id) AS fullarea,
*/
(SELECT COUNT(*) /* *8 */ FROM fsppilot p WHERE p.variant_id = t.variant_id AND p.fspmethod_id = g.fspmethod_id) AS n
FROM trace t
JOIN variant v
  ON t.variant_id = v.id
JOIN fspgroup g
  ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
 AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
JOIN fspmethod m
  ON m.id = g.fspmethod_id
JOIN result_GenericExperimentMessage r
  ON r.pilot_id = g.pilot_id
WHERE $FILTER
GROUP BY t.variant_id, g.fspmethod_id, r.resulttype

ORDER BY benchmark, variant, method, CONCAT(resulttype)
;
EOT
