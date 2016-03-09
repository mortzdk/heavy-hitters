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
		*)
			# unknown option
			;;
	esac
	shift # past argument or value
done

echo -n "# COMMIT: " >> ${OUT}
git log -1 --oneline >> ${OUT}
echo -n "# " >> ${OUT}
date >> ${OUT}

limit=256
p=2
for ((i=1; p<limit; i++));
do
	DELTA=0.25
	PHI=$(echo "1/${p}" | bc -l)
	
	e=4
	for ((j=1; e<=limit; j++));
	do
		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		EPSILON=$(echo "1/${e}" | bc -l)
		./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --const >> ${OUT}.const
		./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --min >> ${OUT}.min
		./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e ${EPSILON} -d ${DELTA} -p ${PHI} -f ${FILE} --median >> ${OUT}.median
		((e*=2))
	done
	((p*=2))
done
