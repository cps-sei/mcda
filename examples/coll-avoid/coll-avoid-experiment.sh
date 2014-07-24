#!/bin/bash

if [ "$#" != "3" ]; then
    echo "Usage : $0 <num-nodes> <num-experiments> <out-file>"
    echo "Example : $0 3 10 foo.out"
    exit 1
fi

NUM_NODES=$1
NUM_EXPERIMENTS=$2
OUT_FILE=$3

g++ -Wall coll-avoid-experiment.cpp -o coll-avoid-experiment

#names of output directories
TMPDS=""

for ((i=0; i < $NUM_EXPERIMENTS; i++))
do
    TMPD=$(mktemp -d /tmp/coll-avoid.XXXXXXXX)
    echo "Experiment $i: $TMPD"
    TMPDS="$TMPDS $TMPD"
    echo "Experiment $i:" > $TMPD/out
    $MCDA_ROOT/examples/coll-avoid/coll-avoid-experiment $NUM_NODES dom-$i $TMPD 2>&1 >> $TMPD/out &
    sleep 1
done

#wait for experiments to finish, then concatenate outputs
wait
rm -f $OUT_FILE
echo 'Coll  AvgSpeed  Timeout' > $OUT_FILE
for i in $TMPDS; do
    cat $i/out
    cat $i/stats >> $OUT_FILE
done
