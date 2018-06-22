#!/bin/bash

show_description() {
cat >&2 <<EOT
This script calculates the per-symbol absolute result count (EAFC) for the
"inject-on-write" fault model, which assumes that writes to memory can get
corrupted, and completely disregards storage duration (e.g. time between write
and subsequent read).  The metric is calculated from the results of a normal
"inject-on-read" campaign, only using the FI results from memory accesses that
directly follow a "write" access.

Only works if write equivalence classes are also imported into the database
(i.e. import-trace is not run with --no-write-ecs).
EOT
}
source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SELECT v.benchmark, v.variant, s.name, s.size, r.resulttype, SUM(t.width) AS occurrences
FROM variant v
JOIN symbol s
	ON v.id = s.variant_id
JOIN trace tw
	ON tw.variant_id = v.id
	AND tw.data_address BETWEEN s.address AND s.address + s.size - 1
	AND tw.accesstype = 'W'
JOIN trace t
	ON t.variant_id = v.id
	AND t.instr1 = tw.instr2 + 1
	AND t.data_address = tw.data_address
JOIN fspgroup g
	ON t.variant_id = g.variant_id
	AND t.data_address = g.data_address
	AND t.instr2 = g.instr2
	AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
JOIN result_GenericExperimentMessage r
	ON r.pilot_id = g.pilot_id
WHERE $FILTER
GROUP BY v.id, s.name, r.resulttype
ORDER BY v.benchmark, v.variant, s.name, CONCAT(r.resulttype)
;
EOT
