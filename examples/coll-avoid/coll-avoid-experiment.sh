#!/bin/bash

if [ "$#" != "5" ]; then
    echo "Usage : $0 <num-nodes> <num-experiments> <dasl-file> <daslc-args> <out-file>"
    echo "        num-experiments = number of experiments"
    echo "        each experiment will do 5 runs in parallel"
    echo 'Example : $0 3 10 coll-avoid.dasl "--DUSE_DISC_METHOD 1" foo.out (to use disc method)'
    echo 'Example : $0 3 10 coll-avoid.dasl "--DUSE_DISC_METHOD 0" foo.out (to use conservative method)'
    exit 1
fi

NUM_NODES=$1
NUM_EXPERIMENTS=$2
DASL_FILE=$3
DASLC_ARGS="$4"
OUT_FILE=$5

DASL_BASE=$(basename $DASL_FILE.dasl)
CPP_FILE="$DASL_BASE.cpp"
g++ -Wall coll-avoid-experiment.cpp -o coll-avoid-experiment
daslc $DASLC_ARGS --nodes $NUM_NODES --madara --out $CPP_FILE $DASL_FILE
g++ -I$ACE_ROOT -I$MADARA_ROOT/include -o $DASL_BASE $CPP_FILE \
$MADARA_ROOT/libMADARA.so $ACE_ROOT/lib/libACE.so

#names of output directories
TMPDS=""

for ((i=0; i < $NUM_EXPERIMENTS; i++))
do
    for j in `seq 1 5`; do
        TMPD=$(mktemp -d /tmp/coll-avoid.XXXXXXXX)
        echo "Experiment $i Run $j: $TMPD"
        TMPDS="$TMPDS $TMPD"
        echo "Experiment $i Run $j:" > $TMPD/out
        $MCDA_ROOT/examples/coll-avoid/coll-avoid-experiment ./$DASL_BASE $NUM_NODES dom-$j $TMPD 2>&1 >> $TMPD/out &
    done
    wait
done

#concatenate outputs
sleep 1
rm -f $OUT_FILE
echo 'Coll  AvgSpeed  Timeout' > $OUT_FILE
for i in $TMPDS; do
    cat $i/out
    cat $i/stats >> $OUT_FILE
done

#compute and print stats
COLL_AVG=$(tail -n +2 $OUT_FILE | awk '{sum+=$1} END { print sum/NR}')
COLL_SD=$(tail -n +2 $OUT_FILE | awk '{print $1}' | awk '{x[NR]=$0; s+=$0; n++} END{a=s/n; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/n); print sd}')
COLL_RE=$(echo "$COLL_SD / $COLL_AVG" | bc -l)
SPD_AVG=$(tail -n +2 $OUT_FILE | awk '{sum+=$2} END { print sum/NR}')
SPD_SD=$(tail -n +2 $OUT_FILE | awk '{print $2}' | awk '{x[NR]=$0; s+=$0; n++} END{a=s/n; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/n); print sd}')
SPD_RE=$(echo "$SPD_SD / $SPD_AVG" | bc -l)
echo "Collision : Avg = $COLL_AVG StDev = $COLL_SD RE = $COLL_RE" >> $OUT_FILE
echo "Speed : Avg = $SPD_AVG StDev = $SPD_SD RE = $SPD_RE" >> $OUT_FILE
