#!/bin/bash

show_description() {
cat >&2 <<EOT
This script calculates the global absolute result count (EAFC) for the
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

# Currently does not take into account right-margin writes, i.e. those
# that are not followed by *any* memory access.  This is mostly OK as
# import-trace creates synthetic 'write' events at the right margin for
# all but those memory accesses that are exactly on the right margin
# already.
$MYSQL << EOT
SET sql_mode = 'NO_UNSIGNED_SUBTRACTION';
SELECT v.benchmark, v.variant, r.resulttype, SUM(t.width) AS occurrences
FROM result_GenericExperimentMessage r
INNER JOIN fspgroup g
	ON g.pilot_id = r.pilot_id
	AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
INNER JOIN trace t
	ON g.instr2 = t.instr2
	AND g.data_address = t.data_address
	AND g.variant_id = t.variant_id
INNER JOIN trace tw
	ON t.instr1 = tw.instr2 + 1
	AND t.data_address = tw.data_address
	AND t.variant_id = tw.variant_id
	AND tw.accesstype = 'W'
INNER JOIN variant v
	ON t.variant_id = v.id
WHERE $FILTER
GROUP BY v.id, r.resulttype
ORDER BY v.benchmark, v.variant, CONCAT(r.resulttype)
;
EOT
