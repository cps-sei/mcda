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

DASL_BASE=$(basename $DASL_FILE .dasl)
CPP_FILE="$DASL_BASE.cpp"
g++ -Wall coll-avoid-experiment.cpp -o coll-avoid-experiment
daslc $DASLC_ARGS --nodes $NUM_NODES --madara --out $CPP_FILE $DASL_FILE
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
	mkdir $LOG_DIR/expr-$i-$j
	EXPRD="$LOG_DIR/expr-$i-$j"
	echo "Experiment $i Run $j: $EXPRD"
	EXPRDS="$EXPRDS $EXPRD"
	$MCDA_ROOT/examples/coll-avoid/coll-avoid-experiment ./$DASL_BASE $NUM_NODES dom-$j $EXPRD &
    done
    wait
done

#concatenate outputs
sleep 1
rm -f $OUT_FILE
echo 'Collision_Rate  Success_Rate  Norm_Completion_Time' > $OUT_FILE
for i in $EXPRDS; do
    #cat $i/out
    cat $i/stats >> $OUT_FILE
done

#compute and print stats
#collision rate
COLL_AVG=$(tail -n +2 $OUT_FILE | awk '{sum+=$1} END { print sum/NR}')
COLL_SD=$(tail -n +2 $OUT_FILE | awk '{print $1}' | awk '{x[NR]=$0; s+=$0; n++} END{a=s/n; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/n); print sd}')
if [ "$COLL_AVG" == "0" ]; then
	COLL_RE="N/A"
else
	COLL_RE=$(echo "$COLL_SD / $COLL_AVG" | bc -l)
fi
#success rate
SUCC_AVG=$(tail -n +2 $OUT_FILE | awk '{sum+=$2} END { print sum/NR}')
SUCC_SD=$(tail -n +2 $OUT_FILE | awk '{print $2}' | awk '{x[NR]=$0; s+=$0; n++} END{a=s/n; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/n); print sd}')
if [ "$SUCC_AVG" == "0" ]; then
        SUCC_RE="N/A"
else
        SUCC_RE=$(echo "$SUCC_SD / $SUCC_AVG" | bc -l)
fi
#completion time
TIME_AVG=$(tail -n +2 $OUT_FILE | awk '{sum+=$3} END { print sum/NR}')
TIME_SD=$(tail -n +2 $OUT_FILE | awk '{print $3}' | awk '{x[NR]=$0; s+=$0; n++} END{a=s/n; for (i in x){ss += (x[i]-a)^2} sd = sqrt(ss/n); print sd}')
if [ "$TIME_AVG" == "0" ]; then
        TIME_RE="N/A"
else
        TIME_RE=$(echo "$TIME_SD / $TIME_AVG" | bc -l)
fi
echo "Collision Rate : Avg = $COLL_AVG StDev = $COLL_SD RE = $COLL_RE" >> $OUT_FILE
echo "Success Rate : Avg = $SUCC_AVG StDev = $SUCC_SD RE = $SUCC_RE" >> $OUT_FILE
echo "Normalized Completion Time : Avg = $TIME_AVG StDev = $TIME_SD RE = $TIME_RE" >> $OUT_FILE
