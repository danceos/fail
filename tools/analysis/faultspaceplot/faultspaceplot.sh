#!/bin/bash
set -e

if [ "$1" = -k ]; then
	KEEPCSV=yes
	shift
else
	KEEPCSV=no
fi

if [ ! $# -eq 3 ]; then
	echo "usage: $0 [ -k ] DATABASE VARIANT BENCHMARK" >&2
	echo "  -k      Keep compacted plot (and symbols) CSV" >&2
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
RAWCSV=$(mktemp)
echo "getting faultspace data.."
$MYSQL <<EOT > $RAWCSV
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
	  AND r.resulttype != 'OK_MARKER'
	  AND t.accesstype = 'R';
EOT

# sanity check
if [ $(wc -l < $RAWCSV) -le 1 ]; then
	rm "$RAWCSV"
	echo "no result data found for variant/benchmark $VARIANT/$BENCHMARK." >&2
	exit 1
fi

# compact data
echo "compacting data.."
COMPACTCSV=$(mktemp)
"$MYDIR"/fsp.compact.sh $RAWCSV $COMPACTCSV
rm $RAWCSV

# fetch symbols if available
SYMBOLCSV=
if table_exists symbol; then
	SYMBOLCSV=$(mktemp)
	echo "getting symbol information ..."
	$MYSQL <<EOT > $SYMBOLCSV
	SELECT s.address, s.size, s.name
	FROM variant v
	JOIN symbol s ON s.variant_id = v.id
	WHERE v.variant = '$VARIANT' AND v.benchmark = '$BENCHMARK'
	ORDER BY s.address
EOT
fi

# plot data
echo "plotting.."
if [ $KEEPCSV = yes ]; then
	KEPT="$VARIANT"_"$BENCHMARK"-plot.csv
	mv "$COMPACTCSV" "$KEPT"
	COMPACTCSV=$KEPT
fi
if [ ! -z "$SYMBOLCSV" -a $(wc -l < $SYMBOLCSV) -gt 1 ]; then
	if [ $KEEPCSV = yes ]; then
		KEPT="$VARIANT"_"$BENCHMARK"-symbols.csv
		mv "$SYMBOLCSV" "$KEPT"
		SYMBOLCSV=$KEPT
	fi
	"$MYDIR"/fsp.plot.py "$COMPACTCSV" "$SYMBOLCSV"
	[ $KEEPCSV = no ] && rm "$SYMBOLCSV"
else
	"$MYDIR"/fsp.plot.py "$COMPACTCSV"
fi
[ $KEEPCSV = no ] && rm "$COMPACTCSV"
