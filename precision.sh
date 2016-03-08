#!/usr/bin/env bash

set -e
set -u

OUT=$1
EXTRA=${2:-}
IMPL="all"

while [[ $# > 1 ]]
do
	key="$2"

	case $key in
		-f|--file)
			FILE="$3"
			shift # past argument
			;;
		--const)
			IMPL="$key"
			shift # past argument
			;;
		--min)
			IMPL="$key"
			shift # past argument
			;;
		--median)
			IMPL="$key"
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

e=4
p=2
for ((i=1; e<8192; i++));
do
	((e*=2))
	SEED1=$[ 1 + $[ RANDOM % 32768 ]]
	SEED2=$[ 1 + $[ RANDOM % 32768 ]]
	EPSILON=$(echo "1/${e}" | bc -l)
	DELTA=0.25
	
	for ((j=1; p<e; j++));
	do
		PHI=$(echo "1/${p}" | bc -l)
		if [ "$IMPL"=="all" ] 
		then
			./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e ${EPSILON} -d ${DELTA} -p ${PHI} -f ${FILE} --const >> ${OUT}
			./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e ${EPSILON} -d ${DELTA} -p ${PHI} -f ${FILE} --min >> ${OUT}
			if (( $(echo $(echo "sqrt($EPSILON)" | bc)'<'$PHI | bc -l) )) 
			then
				./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e $(echo "sqrt($EPSILON)" | bc) -d ${DELTA} -p ${PHI} -f ${FILE} --median >> ${OUT}
			fi
		else
			./main -m 4294967295 -1 ${SEED1} -2 ${SEED2} -e ${EPSILON} -d ${DELTA} -p ${PHI} -f ${FILE} ${IMPL} >> ${OUT}
		fi
		((p*=2))
	done
done
