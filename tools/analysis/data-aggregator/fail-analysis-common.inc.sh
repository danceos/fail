set -e

if [ "$1" = -t ]; then
	FORMAT=-t
	shift
else
	FORMAT=-B
fi

if [ \( ! -z "${MUST_FILTER+x}" -a $# -ne 3 \) -o \( -z "${MUST_FILTER+x}" -a \( $# -lt 1 -o $# -gt 3 \) \) ]; then
	if [ "$(type -t show_description)" = function ]; then
		show_description
		echo '' >&2
	fi
	if [ ! -z "${MUST_FILTER+x}" ]; then
		echo "usage: $0 [ -t ] DATABASE BENCHMARK VARIANT" >&2
	else
		echo "usage: $0 [ -t ] DATABASE [ BENCHMARK [ VARIANT ] ]" >&2
	fi
	echo "  -t      Display output in table format (tab-separated CSV otherwise)" >&2
	exit 1
fi

DATABASE=$1
BENCHMARK=$2
VARIANT=$3
MYSQL="mysql $FORMAT $DATABASE"

# don't filter anything by default
FILTER=1
if [ -n "$BENCHMARK" ]; then
	FILTER="v.benchmark = '$BENCHMARK'"
	if [ -n "$VARIANT" ]; then
		FILTER="$FILTER AND v.variant = '$VARIANT'"
	fi
fi
FILTER="($FILTER)"
