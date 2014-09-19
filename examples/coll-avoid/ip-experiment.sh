#!/bin/bash

if [ "$#" != "4" ]; then
    echo "Usage : $0 <num-nodes> <num-experiments> <dasl-file> <daslc-args>"
    echo "        num-experiments = number of experiments"
    echo "        each experiment will do 5 runs in parallel"
    echo "Example : $0 3 10 intersection.dasl \"--DMETHOD 0\" (to use conservative method)"
    echo "Example : $0 3 10 intersection.dasl \"--DMETHOD 1\" (to use disc method)"
    exit 1
fi

NUM_NODES=$1
NUM_EXPERIMENTS=$2
DASL_FILE=$3
DASLC_ARGS="$4"

DASL_BASE=$(basename $DASL_FILE .dasl)
CPP_FILE="$DASL_BASE.cpp"
g++ -Wall ip-driver.cpp -o ip-driver
daslc --simulation $DASLC_ARGS --nodes $NUM_NODES --madara --out $CPP_FILE $DASL_FILE
g++ -I$ACE_ROOT -I$MADARA_ROOT/include -o $DASL_BASE $CPP_FILE \
$MADARA_ROOT/libMADARA.so $ACE_ROOT/lib/libACE.so

#output directories of all experiment runs are under experiment-logs/
LOG_DIR="experiment-logs"
rm -rf $LOG_DIR
mkdir $LOG_DIR

#names of output directories
EXPRDS=""

for i in `seq 1 $NUM_EXPERIMENTS`; do
    for j in `seq 1 5`; do
	mkdir $LOG_DIR/experiment-$i-$j
	EXPRD="$LOG_DIR/experiment-$i-$j"
	echo "Experiment $i Run $j: $EXPRD"
  	EXPRDS="$EXPRDS $EXPRD"
  	$MCDA_ROOT/examples/coll-avoid/ip-driver ./$DASL_BASE $MOC $NUM_NODES dom-$j $EXPRD &
    done
    wait
done
