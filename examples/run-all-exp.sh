#!/bin/bash

export PATH=$PATH:$(pwd)
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
BRFMT="ExpName:Nodes:Rounds:CPLING-Time:Status"
BRUNCH="./brunch --cpu 100000 --mem 100000 --format $BRFMT"
BRUNCHFAIL="./brunch --cpu 3 --mem 100000 --format $BRFMT"
TMPD=$(mktemp -d)

#array of experiments doomed to fail
declare -a DOOMED=( )

#check if an experiment is doomed
function is_doomed {
    for var in "${DOOMED[@]}"; do
        if [ "$var" == "$1" ]; then
            echo "y"
            return 0
        fi
    done
    echo "n"
    return 1
}

#update stats and output files
function update_results {
    cp $TMPD/*.stdout $TMPD/*.stderr $1
    if [ ! -e $1/stats ]; then
        cp $TMPD/stats $1
    else
        tail -n1 $TMPD/stats >> $1/stats
    fi
}

#update DOOMED array for two variables
function update_doomed_2 {
    local N="$1"
    local R="$2"
    local NV="$3"
    local RV="$4"

    #if already doomed, no need to do anything
    if [ $(is_doomed "$1") == "${N}:${R}" ]; then
        return
    fi

    local RES=$(tail -n1 $TMPD/stats | awk -F',' '{print $5}')
    if [ "x$RES" == "xUNK" ]; then
        for N1 in $NV; do
            for R1 in $RV; do
                if [ $N1 -gt $N ] && [ $R1 -ge $R ] ; then
                    DOOMED+=( "${N1}:${R1}" )
                fi
                if [ $N1 -ge $N ] && [ $R1 -gt $R ] ; then
                    DOOMED+=( "${N1}:${R1}" )
                fi
            done
        done
    fi
}

#update DOOMED array for three variables
function update_doomed_3 {
    local N="$1"
    local X="$2"
    local R="$3"
    local NV="$4"
    local XV="$5"
    local RV="$6"

    #if already doomed, no need to do anything
    if [ $(is_doomed "$1") == "${N}:${X}:${R}" ]; then
        return
    fi

    local RES=$(tail -n1 $TMPD/stats | awk -F',' '{print $5}')
    if [ "x$RES" == "xUNK" ]; then
        for N1 in $NV; do
            for X1 in $XV; do
                for R1 in $RV; do
                    if [ $N1 -gt $N ] && [ $X1 -ge $X ] && [ $R1 -ge $R ] ; then
                        DOOMED+=( "${N1}:${X1}:${R1}" )
                    fi
                    if [ $N1 -ge $N ] && [ $X1 -gt $X ] && [ $R1 -ge $R ] ; then
                        DOOMED+=( "${N1}:${X1}:${R1}" )
                    fi
                    if [ $N1 -ge $N ] && [ $X1 -ge $X ] && [ $R1 -gt $R ] ; then
                        DOOMED+=( "${N1}:${X1}:${R1}" )
                    fi
                done
            done
        done
    fi
}

#run brunch or brunch_fail
function run_brunch {
    if [ $(is_doomed "$1") == "y" ]; then
        $BRUNCHFAIL --out $TMPD "$2" -- ./run-exp.sh
    else
        $BRUNCH --out $TMPD "$2" -- ./run-exp.sh
    fi
}

#check if stats exist for an experiment already
function exists_stats {
    if [ ! -e $1/stats ]; then
        echo "n"
    else
        A=$(echo $2 | awk -F':' '{print $1}')
        B=$(echo $2 | awk -F':' '{print $3}')
        C=$(echo $2 | awk -F':' '{print $4}')
        D="$A,$B,$C,"
        if [ $(grep $D $1/stats | wc -l) == "1" ]; then
            cat $1/stats | grep $D > $TMPD/stats
            echo "y"
        else
            echo "n"
        fi
    fi
}

#create directory if it does not exist
function create_dir {
    if [ ! -e $1 ]; then 
        echo "creating directory $1 .."
        mkdir $1
    else
        echo "directory $1 exists .."
    fi
}

#coll experiments
function do_coll {
    local OUTD=brunch.out.coll.$1.$2
    local NV="2 4 6 8 10"
    local XV="4 7 10"
    local RV="5 10 15 20 30 40 50"
    create_dir $OUTD 
    for N in $NV; do
        for X in $XV; do
            for R in $RV; do
                TD="coll.$1.$X.$X:sync-coll-avoid.$1.dasl:$N:$R:--DX=$X--DY=$X$2"
                if [ $(exists_stats $OUTD "$TD") == "n" ]; then
                    echo "running $TD .."
                    run_brunch "${N}:${X}:${R}" "$TD"
                    update_results $OUTD
                else
                    echo "skipping $TD .."
                fi
                update_doomed_3 "$N" "$X" "$R" "$NV" "$XV" "$RV"
                rm -f $TMPD/*
            done
        done
    done
}

#coll-opt experiments
function do_coll_opt {
    local OUTD=brunch.out.coll.opt.$1.$2
    local NV="2 4 6 8 10"
    local XV="4 7 10"
    local RV="5 10 15 20 30 40 50"
    create_dir $OUTD 
    for N in $NV; do
        for X in $XV; do
            for R in $RV; do
                TD="coll.opt.$1.$X.$X:sync-coll-opt.$1.dasl:$N:$R:--DX=$X--DY=$X$2"
                if [ $(exists_stats $OUTD "$TD") == "n" ]; then
                    echo "running $TD .."
                    run_brunch "${N}:${X}:${R}" "$TD"
                    update_results $OUTD
                else
                    echo "skipping $TD .."
                fi
                update_doomed_3 "$N" "$X" "$R" "$NV" "$XV" "$RV"
                rm -f $TMPD/*
            done
        done
    done
}

#coll-3d experiments
function do_coll_3d {
    local OUTD=brunch.out.coll.3d.$1.$2
    local NV="4 7 10"
    local XV="4 7 10"
    local RV="30 60 90 120 150"
    create_dir $OUTD 
    for N in $NV; do
        for X in $XV; do
            for R in $RV; do
                TD="coll.3d.$1.$X.$X:sync-coll-3d.$1.dasl:$N:$R:--DX=$X--DY=$X$2"
                if [ $(exists_stats $OUTD "$TD") == "n" ]; then
                    echo "running $TD .."
                    run_brunch "${N}:${X}:${R}" "$TD"
                    update_results $OUTD
                else
                    echo "skipping $TD .."
                fi
                update_doomed_3 "$N" "$X" "$R" "$NV" "$XV" "$RV"
                rm -f $TMPD/*
            done
        done
    done
}

#coll-3d-opt experiments
function do_coll_3d_opt {
    local OUTD=brunch.out.coll.3d.opt.$1.$2
    local NV="2 4 6 8 10"
    local XV="4 7 10"
    local RV="5 10 15 20 30 40 50 60 80 100"
    create_dir $OUTD 
    for N in $NV; do
        for X in $XV; do
            for R in $RV; do
                TD="coll.3d.opt.$1.$X.$X:sync-coll-3d-opt.$1.dasl:$N:$R:--DX=$X--DY=$X$2"
                if [ $(exists_stats $OUTD "$TD") == "n" ]; then
                    echo "running $TD .."
                    run_brunch "${N}:${X}:${R}" "$TD"
                    update_results $OUTD
                else
                    echo "skipping $TD .."
                fi
                update_doomed_3 "$N" "$X" "$R" "$NV" "$XV" "$RV"
                rm -f $TMPD/*
            done
        done
    done
}

#mutex experiments
function do_mutex {
    local OUTD=brunch.out.mutex.$1.$2
    local NV="2 4 6 8 10"
    local RV="5 10 15 20 30 40 50 60 80 100"
    create_dir $OUTD 
    for N in $NV; do
        for R in $RV; do
            TD="mutex.$1:sync-mutex.$1.dasl:$N:$R:$2"
            if [ $(exists_stats $OUTD "$TD") == "n" ]; then
                echo "running $TD .."
                run_brunch "${N}:${R}" "$TD"
                update_results $OUTD
            else
                echo "skipping $TD .."
            fi
            update_doomed_2 "$N" "$R" "$NV" "$RV"
            rm -f $TMPD/*
        done
    done
}

#print usage and exit
function usage {
    echo "Usage : $0 <Application> <Type> <Sequentialization>"
    echo ""
    echo "Application ="
    echo "        do_coll : for 2-dimensional collision avoidance"
    echo " do_coll_3d_opt : for 3-dimensional collision avoidance"
    echo "       do_mutex : for mutual exclusion"
    echo ""
    echo "Type ="
    echo "        ok : for correct variant"
    echo "      bug1 : for first buggy variant"
    echo "      bug2 : for second buggy variant"
    echo "      Note : bug2 is not applicable for do_coll_3d_opt"
    echo ""
    echo "Sequentialization ="
    echo "   --seq-sem : to use SEQSEM"
    echo "   --seq-dbl : to use SEQDBL"
    echo ""
    echo "Results will be in brunch.out.Foo.Type.Sequentialization/stats"
    echo "where Foo will depend on Application"
    echo ""
    echo "For example: $0 do_mutex bug2 --seq-dbl"
    exit 1
}

if [ "$#" == "0" ]; then
    usage
    # do_coll ok --seq-sem
    # do_coll bug1 --seq-sem 
    # do_coll bug2 --seq-sem
    # # do_coll_opt ok --seq-sem
    # # do_coll_opt bug1 --seq-sem
    # # do_coll_opt bug2 --seq-sem
    # do_mutex ok --seq-sem
    # do_mutex bug1 --seq-sem
    # do_mutex bug2 --seq-sem
    # do_mutex ok --seq-dbl
    # do_mutex bug1 --seq-dbl
    # do_mutex bug2 --seq-dbl
else
    $*
fi
