#!/usr/bin/env bash

# Remember to run `make clean && make USERFLAGS=-DSPACE` before running this 
# script.

set -e
set -u

EXTRA=${1:-}

while [[ $# > 0 ]]
do
	key="$1"

	case $key in
		-f|--file)
			FILE="$2"
			shift # past argument
			;;
		-s|--sketch)
			TYPE="sketch"
			shift # past argument
			;;
		-m|--universe)
			UNIVERSE=$2
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

function ceil {                                                             
    res=$(echo "($1 + 0.5)/1" | bc)                                         
    if [ $(echo "${res} < $1" | bc) -eq 1 ]; then                           
        res=$((${res}+1))                                                   
    fi                                                                      
    echo ${res}                                                             
}

if [ "$TYPE" == "hh" ]; then
	limit=$(echo "2^15" | bc)
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
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --const   2>> space/space.hh.${i}.const
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --min     2>> space/space.hh.${i}.min
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --median  2>> space/space.hh.${i}.median
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --cormode 2>> space/space.hh.${i}.cmh
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --kmin    2>> space/space.hh.${i}.kmin
		./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -w ${WIDTH} \
			-h ${HEIGHT} -p ${PHI} -f ${FILE} --kmedian 2>> space/space.hh.${i}.kmedian
	done
else
	B=4
	DELTA=$(echo "1/(2^18)" | bc -l)                                            
	UNIVERSE=$(echo "2^26" | bc)

	FILE=datasets/Zipfian/sketch_zipf_1.0alpha.dat

	limit=$(echo "2^20" | bc)

	echo "Name,L1 Error,L2 Error,Epsilon,Delta,Width,Depth,M,L1,L2"
	for ((e=2; e <= limit; e*=2));
	do
		EPSILON=$(echo "1/${e}" | bc -l)                                    
		SEED1=$[ 1 + $[ RANDOM % 32768 ]]                                       
		SEED2=$[ 1 + $[ RANDOM % 32768 ]]                                       

		h=$(echo "l(1/${DELTA})/l(${B})" | bc -l)
		w=$(echo "${B}/${EPSILON}" | bc -l)

		WIDTH=$(ceil ${w})                                                   
		HEIGHT=$(ceil ${h}) 

		# Theoretical space
	#	./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
	#		-d ${DELTA} -e ${EPSILON} -f ${FILE} --median  2>> space.theory.median
	#	./precision_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
	#		-d ${DELTA} -e ${EPSILON} -f ${FILE} --min     2>> space.theory.min

		# Equal space
		./error_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
			-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --median  2>> space.exact.min
		./error_sketch -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} \
			-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --min     2>> space.exact.median
	done
fi
