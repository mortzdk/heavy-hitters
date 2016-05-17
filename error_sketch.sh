#!/usr/bin/env bash

set -e
set -u

OUT=$1
EXTRA=${2:-}

while [[ $# > 1 ]]
do
	key="$2"

	case $key in
		-f|--file)
			FILE="$3"
			shift # past argument
			;;
		-h|--height)
			HEIGHT=$3
			shift # past argument
			;;
		-m|--universe)
			UNIVERSE=$3
			shift # past argument
			;;
		*)
			# unknown option
			;;
	esac
	shift # past argument or value
done

# Run heavy hitter if sketch is not defined
: "${TYPE:="hh"}"
: "${UNIVERSE:=4294967295}"
: "${HEIGHT:=1}"

echo -n "# COMMIT: " >> ${OUT}.min
git log -1 --oneline >> ${OUT}.min
echo -n "# "         >> ${OUT}.min
date                 >> ${OUT}.min

echo -n "# COMMIT: " >> ${OUT}.median
git log -1 --oneline >> ${OUT}.median
echo -n "# "         >> ${OUT}.median
date                 >> ${OUT}.median

echo "Name,Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.min
echo "Name,Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.median
limit=16777216
e=2;
for ((i=1; e<=limit; i++));
do
	for ((j=0; j<10; j++));
	do
		B=4
		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		WIDTH=$(echo "${B}/(1/${e})" | bc -l)
		./error_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -f ${FILE} \
			-w ${WIDTH} -h ${HEIGHT} --min    >> ${OUT}.min
		./error_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -f ${FILE} \
			-w ${WIDTH} -h ${HEIGHT} --median >> ${OUT}.median
	done
	((e*=2))
done
