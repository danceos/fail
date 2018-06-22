#!/bin/bash

# TODO implement show_description

source $(dirname $0)/fail-analysis-common.inc.sh

$MYSQL << EOT
SELECT v.benchmark, v.variant, s.name, s.size, r.resulttype,
	SUM(t.width)
		/
	(SELECT SUM(t.width)
	FROM symbol s
	JOIN trace tw
		ON tw.variant_id = s.variant_id
		AND tw.data_address BETWEEN s.address AND s.address + s.size - 1
		AND tw.accesstype = 'W'
	JOIN trace t
		ON t.variant_id = tw.variant_id
		AND t.data_address = tw.data_address
		AND t.instr1 = tw.instr2 + 1
	JOIN fspgroup g
		ON t.variant_id = g.variant_id
		AND t.data_address = g.data_address
		AND t.instr2 = g.instr2
		AND g.fspmethod_id = (SELECT id FROM fspmethod WHERE method = 'basic')
	JOIN result_GenericExperimentMessage r
		ON r.pilot_id = g.pilot_id
	WHERE t.variant_id = v.id -- refers to parent query
	) AS coverage
FROM variant v
JOIN symbol s
	ON v.id = s.variant_id
JOIN trace tw
	ON tw.variant_id = v.id
	AND tw.data_address BETWEEN s.address AND s.address + s.size - 1
	AND tw.accesstype = 'W'
JOIN trace t
	ON t.variant_id = v.id
	AND t.data_address = tw.data_address
	AND t.instr1 = tw.instr2 + 1
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
