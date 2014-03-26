#!/bin/bash

#resource limits
CORES=$(cat /proc/cpuinfo | grep processor | wc -l)
ulimit -t $(echo "$CORES * 3600" | bc)
ulimit -m $(echo "16 * 1024 * 1024" | bc) #4194304
ulimit -v $(echo "16 * 1024 * 1024" | bc) #4194304

#check usage
if [ "$#" != "1" ]; then
    echo "Usage: $0 A:B:C:D:E"
    echo "   A = experiment name"
    echo "   B = DASL filename"
    echo "   C = number of nodes"
    echo "   D = number of rounds"
    echo "   E = daslc arguments"
    exit 1
fi

#check for binary in path. abort if not found.
function ensure {
    X=$(which $1)
    if [ x"$X" == "x" ]; then
        echo "$1 not found, make sure it is on your path"
        exit 1
    fi
}

#make sure daslc is on path
ensure daslc

#parse the argument
EXPN=$(echo "$1" | awk -F ':' '{print $1}')
echo "BRUNCH_STAT ExpName $EXPN"
DFILE=$(echo "$1" | awk -F ':' '{print $2}')
echo "BRUNCH_STAT DaslFile $DFILE"
NODES=$(echo "$1" | awk -F ':' '{print $3}')
echo "BRUNCH_STAT Nodes $NODES"
ROUNDS=$(echo "$1" | awk -F ':' '{print $4}')
echo "BRUNCH_STAT Rounds $ROUNDS"
DARGS=$(echo "$1" | awk -F ':' '{print $5}' | sed 's/--/ --/g')

TMPF=$(mktemp) && rm -f $TMPF
daslc $DFILE --node $NODES --rounds $ROUNDS $DARGS --seq-out $TMPF.c
#/usr/bin/time -f "BRUNCH_STAT CBMC-Time %e" cbmc $TMPF.c
/usr/bin/time -f "BRUNCH_STAT CPLING-Time %e" ./cbmc-plingeling.sh $TMPF.c &> $TMPF.out
cat $TMPF.out

#get result
if [ $(grep "VERIFICATION SUCCESSFUL" $TMPF.out | wc -l) == "1" ]; then
    echo "BRUNCH_STAT Status OK"
elif [ $(grep "VERIFICATION FAILED" $TMPF.out | wc -l) == "1" ]; then
    echo "BRUNCH_STAT Status BUG"
elif [ $(grep "s SATISFIABLE" $TMPF.out | wc -l) == "1" ]; then
    echo "BRUNCH_STAT Status BUG"
elif [ $(grep "s UNSATISFIABLE" $TMPF.out | wc -l) == "1" ]; then
    echo "BRUNCH_STAT Status OK"
else
    echo "BRUNCH_STAT Status UNK"
fi

#cleanup CNF and output file
CNFF=$(grep "CNF file = " $TMPF.out | awk '{print $4}')
echo "cleaning up $CNFF"
rm -f $CNFF
echo "cleaning up $TMPF.c $TMPF.out"
rm -f $TMPF.c $TMPF.out
