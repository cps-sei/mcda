#!/bin/bash

if [ "$#" != "4" ]; then
    echo "Usage : $0 <num-nodes> <num-experiments> <num-runs> <out-file>"
    echo "Example : $0 3 5 10 foo.out"
    exit 1
fi

NUM_NODES=$1
NUM_EXPERIMENTS=$2
NUM_RUNS=$3
OUT_FILE=$4

g++ -Wall coll-avoid-experiment.cpp -o coll-avoid-experiment

for ((i=0; i < $NUM_EXPERIMENTS; i++))
do
    echo "Experiment $i:"
    $MCDA_ROOT/examples/coll-avoid/coll-avoid-experiment $NUM_NODES $NUM_RUNS $OUT_FILE
done
