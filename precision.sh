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
		-s|--sketch)
			TYPE="sketch"
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

echo -n "# COMMIT: " >> ${OUT}.min
git log -1 --oneline >> ${OUT}.min
echo -n "# "         >> ${OUT}.min
date                 >> ${OUT}.min

echo -n "# COMMIT: " >> ${OUT}.median
git log -1 --oneline >> ${OUT}.median
echo -n "# "         >> ${OUT}.median
date                 >> ${OUT}.median

if [ "$TYPE" == "hh" ]; then
	echo -n "# COMMIT: " >> ${OUT}.const
	git log -1 --oneline >> ${OUT}.const
	echo -n "# "         >> ${OUT}.const
	date                 >> ${OUT}.const

	limit=2048
	p=2
	for ((i=1; p<limit; i++));
	do
		DELTA=0.25
		PHI=$(echo "1/${p}" | bc -l)
		
		e=$((p*2))
		for ((j=1; e<=limit; j++));
		do
			SEED1=$[ 1 + $[ RANDOM % 32768 ]]
			SEED2=$[ 1 + $[ RANDOM % 32768 ]]
			EPSILON=$(echo "1/${e}" | bc -l)
			./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --const  >> ${OUT}.const
			./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -e $(echo "$EPSILON^2" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --min    >> ${OUT}.min
			./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -e ${EPSILON}                -d ${DELTA} -p ${PHI} -f ${FILE} --median >> ${OUT}.median
			((e*=2))
		done
		((p*=2))
	done
else
	echo "Name,L1 Error,L2 Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.min
	echo "Name,L1 Error,L2 Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.median
	limit=16777216
	e=2;
	for ((i=1; e<=limit; i++));
	do
		for ((j=0; j<10; j++));
		do
			SEED1=$[ 1 + $[ RANDOM % 32768 ]]
			SEED2=$[ 1 + $[ RANDOM % 32768 ]]
			WIDTH=$(echo "2/(1/${e})" | bc -l)
			HEIGHT=1
			./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -f ${FILE} -w ${WIDTH} -h ${HEIGHT} --min    >> ${OUT}.min
			./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -f ${FILE} -w ${WIDTH} -h ${HEIGHT} --median >> ${OUT}.median
		done
		((e*=2))
	done
fi
