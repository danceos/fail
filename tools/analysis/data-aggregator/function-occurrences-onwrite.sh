#!/bin/bash

show_description() {
cat >&2 <<EOT
This script calculates the per-function absolute result count (EAFC) for the
"inject-on-write" fault model, which assumes that writes to memory can get
corrupted, and completely disregards storage duration (e.g. time between write
and subsequent read).  The metric is calculated from the results of a normal
"inject-on-read" campaign, only using the FI results from memory accesses that
directly follow a "write" access.

Only works if write equivalence classes are also imported into the database
(i.e. import-trace is not run with --no-write-ecs).
EOT
}
MUST_FILTER=1
source $(dirname $0)/fail-analysis-common.inc.sh

# This implementation makes sure the write -- not the subsequent memory access
# that determines the result type -- happens in the context of the
# corresponding function.
#
# Currently does not take into account right-margin writes, i.e. those
# that are not followed by *any* memory access.  This is mostly OK as
# import-trace creates synthetic 'write' events at the right margin for
# all but those memory accesses that are exactly on the right margin
# already.
$MYSQL << EOT
SELECT v.benchmark, v.variant, s.name, s.address, r.resulttype, SUM(t.width) AS occurrences
FROM variant v
JOIN symbol s
	ON s.variant_id = v.id
JOIN trace tw
	ON tw.variant_id = s.variant_id
	AND tw.instr2_absolute BETWEEN s.address AND s.address + s.size - 1
	AND tw.accesstype = 'W'
JOIN trace t
	ON t.variant_id = s.variant_id
	AND t.data_address = tw.data_address
	AND tw.instr2 + 1 = t.instr1
JOIN fspgroup g
	ON g.variant_id = t.variant_id
	AND g.data_address = t.data_address
	AND g.instr2 = t.instr2
	AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
JOIN result_GenericExperimentMessage r
	ON r.pilot_id = g.pilot_id
WHERE $FILTER
GROUP BY s.variant_id, s.address, r.resulttype
ORDER BY s.address, CONCAT(r.resulttype)
;
EOT
