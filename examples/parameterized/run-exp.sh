#!/bin/bash

#resource limits
ulimit -t 7200
ulimit -m 4194304
ulimit -v 4194304

#print usage and exit
function usage {
    echo "Usage: $0 exp tool"
    echo "   exp  = MUTEX-OK|MUTEX-BUG1|MUTEX-BUG2"
    echo "        | COLL-OK|COLL-BUG1|COLL-BUG2"
    echo "   tool = mcmt|cubicle"
    exit 1
}

#check usage
if [ "$#" != "2" ]; then
    usage
fi

#check for binary in path. abort if not found.
function ensure {
    X=$(which $1)
    if [ x"$X" == "x" ]; then
        echo "$1 not found, make sure it is on your path"
        exit 1
    fi
}

#make sure mcmt and cubicle are on path
ensure mcmt
ensure cubicle

#run experiment
function run_exp {
    EXP="$1"
    TOOL="$2"
    FILE="$3"
    OUT="$EXP.$TOOL.out"
    echo "running $TOOL on $EXP -- result will be in $OUT"
    if [ "$TOOL" == "mcmt" ]; then
        (time mcmt $FILE.in) &> $OUT
    elif [ "$TOOL" == "cubicle" ]; then
        (time cubicle $FILE.cub) &> $OUT
    else
        echo "ERROR: tool $TOOL not supported"
        usage
    fi
}

if [ "$1" == "MUTEX-OK" ]; then
    run_exp $1 $2 sync-mutex.ok
elif [ "$1" == "MUTEX-BUG1" ]; then
    run_exp $1 $2 sync-mutex.bug1
elif [ "$1" == "MUTEX-BUG2" ]; then
    run_exp $1 $2 sync-mutex.bug2
elif [ "$1" == "COLL-OK" ]; then
    run_exp $1 $2 sync-coll-avoid.ok
elif [ "$1" == "COLL-BUG1" ]; then
    run_exp $1 $2 sync-coll-avoid.bug1
elif [ "$1" == "COLL-BUG2" ]; then
    run_exp $1 $2 sync-coll-avoid.bug2
else
    usage
fi
