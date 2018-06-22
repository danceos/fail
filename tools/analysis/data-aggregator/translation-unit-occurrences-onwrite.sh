#!/bin/bash

# TODO implement show_description

source $(dirname $0)/fail-analysis-common.inc.sh

# This implementation makes sure the write -- not the subsequent memory access
# that determines the result type -- happens in the context of the
# corresponding translation unit.
#
# Currently does not take into account right-margin writes, i.e. those
# that are not followed by *any* memory access.  This is mostly OK as
# import-trace creates synthetic 'write' events at the right margin for
# all but those memory accesses that are exactly on the right margin
# already.
$MYSQL << EOT
SELECT v.benchmark, v.variant, f.path, r.resulttype, SUM(t.width) AS occurrences
FROM variant v
JOIN dbg_filename f
	ON f.variant_id = v.id
JOIN dbg_mapping m
	ON f.variant_id = m.variant_id
	AND f.file_id = m.file_id
JOIN trace tw
	ON tw.variant_id = m.variant_id
	AND tw.instr2_absolute BETWEEN m.instr_absolute AND m.instr_absolute + m.line_range_size - 1
	AND tw.accesstype = 'W'
JOIN trace t
	ON t.variant_id = tw.variant_id
	AND t.instr1 = tw.instr2 + 1
	AND t.data_address = tw.data_address
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
