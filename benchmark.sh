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

# Universe
: "${UNIVERSE:=2147483647}"

# HEIGHT and WIDTH is those satisfying the Count Min Sketch guarantees
if [ "$TYPE" == "hh" ]; then

	phis=(0.0001, 0.005, 0.001, 0.05, 0.01)
	for i in "${phis[@]}"
	do
		echo -n "# COMMIT: " >> ${OUT}.${PHI}.const
		git log -1 --oneline >> ${OUT}.${PHI}.const
		echo -n "# "         >> ${OUT}.${PHI}.const
		date                 >> ${OUT}.${PHI}.const

		echo -n "# COMMIT: " >> ${OUT}.${PHI}.min
		git log -1 --oneline >> ${OUT}.${PHI}.min
		echo -n "# "         >> ${OUT}.${PHI}.min
		date                 >> ${OUT}.${PHI}.min

		echo -n "# COMMIT: " >> ${OUT}.${PHI}.median
		git log -1 --oneline >> ${OUT}.${PHI}.median
		echo -n "# "         >> ${OUT}.${PHI}.median
		date                 >> ${OUT}.${PHI}.median

		echo -n "# COMMIT: " >> ${OUT}.${PHI}.cmh
		git log -1 --oneline >> ${OUT}.${PHI}.cmh
		echo -n "# "         >> ${OUT}.${PHI}.cmh
		date                 >> ${OUT}.${PHI}.cmh

		EPSILON=0.0005
		DELTA=0.25
		PHI=${i}

		h=$(echo "l((2*(l(${UNIVERSE})/l(2)))/(${PHI}*${DELTA}))/l(2)" | bc -l)
		w=$(echo "2/(1/${EPSILON})" | bc -l)

		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		WIDTH=$(echo "(${w}+0.5)/1" | bc)
		HEIGHT=$(echo "(${h}+0.5)/1" | bc)


		./benchmark_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --const  -r ${RUNS} \
			-o ${OUT}.${PHI}.const
		./benchmark_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --min    -r ${RUNS} \
			-o ${OUT}.${PHI}.min
		./benchmark_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --median -r ${RUNS} \
			-o ${OUT}.${PHI}.median
		./benchmark_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --cormode -r ${RUNS} \
			-o ${OUT}.${PHI}.cmh
	done
else
	limit=16777216
	e=32
	for ((i=1; e<limit; i++));
	do
		SEED1=$[ 1 + $[ RANDOM % 32768 ]]
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]
		WIDTH=$(echo "2/(1/${e})" | bc -l)
		HEIGHT=4
		RUNS=10

		echo -n "# COMMIT: " >> ${OUT}.${e}.min
		git log -1 --oneline >> ${OUT}.${e}.min
		echo -n "# "         >> ${OUT}.${e}.min
		date                 >> ${OUT}.${e}.min

		echo -n "# COMMIT: " >> ${OUT}.${e}.median
		git log -1 --oneline >> ${OUT}.${e}.median
		echo -n "# "         >> ${OUT}.${e}.median
		date                 >> ${OUT}.${e}.median

		./benchmark_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
			-w ${WIDTH} -h ${HEIGHT} -f ${FILE} --min \
			-r ${RUNS} -o ${OUT}.${e}.min
		./benchmark_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
			-w ${WIDTH} -h ${HEIGHT} -f ${FILE} --median \
			-r ${RUNS} -o ${OUT}.${e}.median


		((e*=2))
	done
fi
