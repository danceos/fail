#!/bin/bash
set -e

if [ ! $# -eq 3 ]; then
	echo "usage: $0 DATABASE VARIANT BENCHMARK" >&2
	exit 1
fi

DATABASE=$1
VARIANT=$2
BENCHMARK=$3
MYSQL="mysql -B --quick $DATABASE"

MYDIR=$(dirname $0)

function table_exists()
{
	N=$(echo "SHOW TABLES LIKE '$1'" | $MYSQL $DATABASE | wc -l)
	[ $N -gt 0 ]
	return
}

# get data
echo "getting faultspace data.."
$MYSQL <<EOT > "$VARIANT"_"$BENCHMARK"-raw.csv
	SELECT t.time1 - (SELECT MIN(t2.time1) FROM trace t2 WHERE t.variant_id = t2.variant_id) AS time1,
		   t.time2 - (SELECT MIN(t2.time1) FROM trace t2 WHERE t.variant_id = t2.variant_id) AS time2,
		   t.data_address, r.bitoffset, r.injection_width,
	CASE
	  WHEN r.resulttype = 'OK_MARKER' THEN '#FFFFFF'
	  WHEN r.resulttype = 'FAIL_MARKER' THEN '#EE0000'
	  WHEN r.resulttype = 'DETECTED_MARKER' THEN '#00FF00'
	  WHEN r.resulttype = 'GROUP0_MARKER' THEN '#EAEAEA'
	  WHEN r.resulttype = 'GROUP1_MARKER' THEN '#EBEBEB'
	  WHEN r.resulttype = 'GROUP2_MARKER' THEN '#ECECEC'
	  WHEN r.resulttype = 'GROUP3_MARKER' THEN '#EDEDED'
	  WHEN r.resulttype = 'TIMEOUT' THEN '#CCCC00'
	  WHEN r.resulttype = 'TRAP' THEN '#00CCCC'
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
"$MYDIR"/fsp.compact.sh "$VARIANT"_"$BENCHMARK"-raw.csv "$VARIANT"_"$BENCHMARK"-plot.csv

# fetch symbols if available
if table_exists symbol; then
	echo "getting symbol information ..."
	$MYSQL <<EOT > "$VARIANT"_"$BENCHMARK"-symbols.csv
	SELECT s.address, s.size, s.name
	FROM variant v
	JOIN symbol s ON s.variant_id = v.id
	WHERE v.variant = '$VARIANT' AND v.benchmark = '$BENCHMARK'
	ORDER BY s.address
EOT
fi

# plot data
echo "plotting.."
if [ -e "$VARIANT"_"$BENCHMARK"-symbols.csv -a $(wc -l < "$VARIANT"_"$BENCHMARK"-symbols.csv) -gt 1 ]; then
	"$MYDIR"/fsp.plot.py "$VARIANT"_"$BENCHMARK"-plot.csv "$VARIANT"_"$BENCHMARK"-symbols.csv
else
	"$MYDIR"/fsp.plot.py "$VARIANT"_"$BENCHMARK"-plot.csv
fi
