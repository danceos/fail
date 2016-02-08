#!/bin/bash
set -e

if [ ! $# -eq 3 ]; then
	echo "usage: $0 DATABASE VARIANT BENCHMARK" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
# add "-t" for more readable output
MYSQL="mysql -B --quick $DATABASE"

# get data
echo "getting faultspace data.."
$MYSQL <<EOT > "$VARIANT"_"$BENCHMARK"-raw.csv
	SELECT t.time1 - (SELECT MIN(t2.time1) FROM trace t2 WHERE t.variant_id = t2.variant_id) AS time1,
		   t.time2 - (SELECT MIN(t2.time1) FROM trace t2 WHERE t.variant_id = t2.variant_id) AS time2,
		   t.data_address, r.bitoffset, 1,
	CASE
	  WHEN r.resulttype = 'OK_MARKER' THEN '#FFFFFF'
	  WHEN r.resulttype = 'FAIL_MARKER' THEN '#EE0000'
	  WHEN r.resulttype = 'DETECTED_MARKER' THEN '#00FF00'
	  WHEN r.resulttype = 'GROUP0_MARKER' THEN '#EAEAEA'
	  WHEN r.resulttype = 'GROUP1_MARKER' THEN '#EBEBEB'
	  WHEN r.resulttype = 'GROUP2_MARKER' THEN '#ECECEC'
	  WHEN r.resulttype = 'GROUP3_MARKER' THEN '#EDEDED'
	  WHEN r.resulttype = 'TIMEOUT' THEN '#00EE00'
	  WHEN r.resulttype = 'TRAP' THEN '#00DD00'
	  WHEN r.resulttype = 'WRITE_TEXTSEGMENT' THEN '#0000AA'
	  WHEN r.resulttype = 'WRITE_OUTERSPACE' THEN '#0000BB'
	  WHEN r.resulttype = 'SDC' THEN '#FF0000'
	  WHEN r.resulttype = 'UNKNOWN' THEN '#000000'
	END AS color
	FROM trace t
	JOIN fspgroup g ON t.variant_id = g.variant_id AND t.data_address = g.data_address AND t.instr2 = g.instr2
	JOIN variant v ON v.id = t.variant_id
	JOIN result_GenericExperimentMessage r ON r.pilot_id=g.pilot_id
	WHERE
		  v.variant = '$VARIANT' AND v.benchmark = '$BENCHMARK'
	  AND t.accesstype = 'R';
EOT

# compact data
echo "compacting data.."
fsp.compact.sh "$VARIANT"_"$BENCHMARK"-raw.csv "$VARIANT"_"$BENCHMARK"-plot.csv

# plot data
echo "plotting.."
fsp.plot.py "$VARIANT"_"$BENCHMARK"-plot.csv
