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


limit=128
p=8
for ((i=1; p<limit; i++));
do
	RUNS=10
	DELTA=0.25
	PHI=$(echo "1/${p}" | bc -l)
	
	e=$((p*2))
	for ((j=1; e<=limit; j++));
	do
		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		EPSILON=$(echo "1/${e}" | bc -l)

		echo -n "# COMMIT: " >> ${OUT}.${p}.${e}.const
		git log -1 --oneline >> ${OUT}.${p}.${e}.const
		echo -n "# "         >> ${OUT}.${p}.${e}.const
		date                 >> ${OUT}.${p}.${e}.const

		echo -n "# COMMIT: " >> ${OUT}.${p}.${e}.min
		git log -1 --oneline >> ${OUT}.${p}.${e}.min
		echo -n "# "         >> ${OUT}.${p}.${e}.min
		date                 >> ${OUT}.${p}.${e}.min

		echo -n "# COMMIT: " >> ${OUT}.${p}.${e}.median
		git log -1 --oneline >> ${OUT}.${p}.${e}.median
		echo -n "# "         >> ${OUT}.${p}.${e}.median
		date                 >> ${OUT}.${p}.${e}.median

		./measure -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --const  -r ${RUNS} -o ${OUT}.${p}.${e}.const
		./measure -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --min    -r ${RUNS} -o ${OUT}.${p}.${e}.min
		./measure -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e ${EPSILON}                -d ${DELTA} -p ${PHI} -f ${FILE} --median -r ${RUNS} -o ${OUT}.${p}.${e}.median
		((e*=2))
	done
	((p*=2))
done
