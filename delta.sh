#!/usr/bin/env bash

set -e
set -u

function ceil {                                                             
    res=$(echo "($1 + 0.5)/1" | bc)                                         
    if [ $(echo "${res} < $1" | bc) -eq 1 ]; then                           
        res=$((${res}+1))                                                   
    fi                                                                      
    echo ${res}                                                             
}

B=4
DELTA=$(echo "1/2" | bc -l)                                            
PHI=$(echo "2^-14" | bc -l)                                            
EPSILON=$(echo "2^-15" | bc -l)                                    
UNIVERSE=$(echo "2^20" | bc -l)

FILE=datasets/Zipfian/uniform.dat

h=$(echo "l((2*(l(${UNIVERSE})/l(2)))/(${PHI}*${DELTA}))/l(${B})" | bc -l)
w=$(echo "${B}/${EPSILON}" | bc -l)

WIDTH=$(ceil ${w})                                                      
HEIGHT=$(ceil ${h}) 

for ((i=0; i < 100; i++));
do
	SEED1=$[ 1 + $[ RANDOM % 32768 ]]                                       
	SEED2=$[ 1 + $[ RANDOM % 32768 ]]                                       

	./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -p ${PHI} \
		-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --const   >> delta.const
	./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -p ${PHI} \
		-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --median  >> delta.median
	./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -p ${PHI} \
		-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --min     >> delta.min
	./precision_hh -m ${UNIVERSE} -1 ${SEED1} -2 ${SEED2} -p ${PHI} \
		-h ${HEIGHT} -w ${WIDTH} -f ${FILE} --cormode >> delta.cmh
done
