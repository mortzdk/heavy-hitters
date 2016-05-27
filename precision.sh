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
: "${UNIVERSE:=2147483647}"

echo -n "# COMMIT: " >> ${OUT}.min
git log -1 --oneline >> ${OUT}.min
echo -n "# "         >> ${OUT}.min
date                 >> ${OUT}.min

echo -n "# COMMIT: " >> ${OUT}.median
git log -1 --oneline >> ${OUT}.median
echo -n "# "         >> ${OUT}.median
date                 >> ${OUT}.median

if [ "$TYPE" == "hh" ]; then

	function ceil {
		res=$(echo "($1 + 0.5)/1" | bc)
		if [ $(echo "${res} < $1" | bc) -eq 1 ]; then
			res=$((${res}+1))
		fi
		echo ${res}
	}

	echo -n "# COMMIT: " >> ${OUT}.const
	git log -1 --oneline >> ${OUT}.const
	echo -n "# "         >> ${OUT}.const
	date                 >> ${OUT}.const

	echo -n "# COMMIT: " >> ${OUT}.cormode
	git log -1 --oneline >> ${OUT}.cormode
	echo -n "# "         >> ${OUT}.cormode
	date                 >> ${OUT}.cormode

	echo -n "# COMMIT: " >> ${OUT}.kmin
	git log -1 --oneline >> ${OUT}.kmin
	echo -n "# "         >> ${OUT}.kmin
	date                 >> ${OUT}.kmin

	echo -n "# COMMIT: " >> ${OUT}.kmedian
	git log -1 --oneline >> ${OUT}.kmedian
	echo -n "# "         >> ${OUT}.kmedian
	date                 >> ${OUT}.kmedian

	limit=$(echo "2^12" | bc)
	for ((i=2; i<=limit; i*=2));
	do
		B=4
		DELTA=0.25
		PHI=$(echo "1/${i}" | bc -l)
		EPSILON=$(echo "1/(${i}*2)" | bc -l)

		h=$(echo "l((2*(l(${UNIVERSE})/l(2)))/(${PHI}*${DELTA}))/l(${B})" | bc -l)
		w=$(echo "${B}/${EPSILON}" | bc -l)

		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		WIDTH=$(ceil ${w})
		HEIGHT=$(ceil ${h})

		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --const   >> ${OUT}.const
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --min     >> ${OUT}.min
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --median  >> ${OUT}.median
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --cormode >> ${OUT}.cormode
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --kmin     >> ${OUT}.kmin
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -d ${DELTA} -f ${FILE} --kmedian  >> ${OUT}.kmedian
	done
else
	echo "Name,L1 Error,L2 Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.min
	echo "Name,L1 Error,L2 Error,Epsilon,Delta,Width,Depth,M,L1,L2" >> ${OUT}.median
	limit=$(echo "2^20" | bc)
	e=2;
	for ((i=1; e<=limit; i++));
	do
		for ((j=0; j<10; j++));
		do
			B=4
			DELTA=0.25
			EPSILON=$(echo "1/(${e})" | bc -l)

			h=$(echo "scale=10; l(1/${DELTA})/l(${B})" | bc -l)
			w=$(echo "${B}/${EPSILON}" | bc -l)

			SEED1=$[ 1 + $[ RANDOM % 32768 ]]
			SEED2=$[ 1 + $[ RANDOM % 32768 ]]
			WIDTH=$(ceil ${w})
			HEIGHT=$(ceil ${h})

			./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
				-f ${FILE} -w ${WIDTH} -h ${HEIGHT} --min    >> ${OUT}.min
			./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
				-f ${FILE} -w ${WIDTH} -h ${HEIGHT} --median >> ${OUT}.median
		done
		((e*=2))
	done
fi
