#!/bin/bash
export PATH=$PATH:../tools/daig/src/daslc:/home/chaki/stuff/cbmc/cbmc-4.7
TMPF1=$(mktemp)
TMPF2=$(mktemp)

#ensures that the first argument is on the path
function ensure {
    X=$(which $1 | wc -l)
    if [ "$X" == "0" ]; then
        echo "ERROR: $1 not found on PATH"
        exit 1
    fi
}

#ensure key executables exist
ensure daslc
ensure cbmc

#takes three arguments -- an experiment name, a C file name and an
#expected result. runs cbmc on C file, compares result with expected,
#and prints message
function run_cbmc {
    cbmc $2 &> $TMPF2
    
    #compute model checking result
    MCRES=$(grep "^VERIFICATION SUCCESSFUL" $TMPF2 | wc -l)
    if [ "$MCRES" == "1" ]; then
        MCRES="SUCCESS"
    else
        MCRES=$(grep "^VERIFICATION FAILED" $TMPF2 | wc -l)
        if [ "$MCRES" == "1" ]; then
            MCRES="FAILURE"
        else
            MCRES="UNKNOWN"
        fi
    fi

    if [ "$3" == "$MCRES" ]; then
        printf "%-35s : PASSED\n" $1
    else
        printf "%-35s : FAILED - expected $3 got $MCRES\n" $1
    fi
}

#all the testcases
TESTS=( \
    "sync-coll-avoid.ok.dasl 2 4 SUCCESS" \
    "sync-coll-avoid.bug1.dasl 2 4 FAILURE" \
    "sync-coll-avoid.bug2.dasl 2 4 FAILURE" \
    "sync-mutex.ok.dasl 2 4 SUCCESS" \
    "sync-mutex.bug1.dasl 2 4 FAILURE" \
    "sync-mutex.bug2.dasl 2 4 FAILURE" \
    "sync-coll-opt.ok.dasl 2 4 SUCCESS" \
    "sync-coll-opt.bug1.dasl 2 4 FAILURE" \
    "sync-coll-opt.bug2.dasl 2 4 FAILURE" \
    )

#test parser
function test_parser {
    echo "==== testing daslc parsing and printing"
    for j in "${TESTS[@]}"; do
        i=$(echo $j | awk '{print $1}')
        daslc $i --print > $TMPF1
        daslc $TMPF1 --print > $TMPF2
        DIFF=$(diff $TMPF1 $TMPF2 | wc -l)
        if [ "$DIFF" == "0" ]; then
            printf "%-30s : PASSED\n" $(basename $i)
        else
            printf "%-30s : FAILED\n" $(basename $i)
        fi
    done
}

#test verification with array-based sequentialization
function test_verif_array {
    echo "==== testing verifier with array sequentialization"
    for i in "${TESTS[@]}"; do
        CMDS=($i)
        daslc ${CMDS[0]} --nodes ${CMDS[1]} --rounds ${CMDS[2]} --seq-out $TMPF1.c &> /dev/null
        run_cbmc "${CMDS[0]}" $TMPF1.c "${CMDS[3]}"
    done
}

#test verification with no-array-based sequentialization
function test_verif_no_array {
    echo "==== testing verifier with no-array sequentialization"
    for i in "${TESTS[@]}"; do
        CMDS=($i)
        daslc ${CMDS[0]} --nodes ${CMDS[1]} --rounds ${CMDS[2]} --seq-no-array --seq-out $TMPF1.c &> /dev/null
        run_cbmc "${CMDS[0]}" $TMPF1.c "${CMDS[3]}"
    done
}

#test verification with semantics-based sequentialization
function test_verif_sem {
    echo "==== testing verifier with semantics sequentialization"
    for i in "${TESTS[@]}"; do
        CMDS=($i)
        daslc ${CMDS[0]} --nodes ${CMDS[1]} --rounds ${CMDS[2]} --seq-sem --seq-out $TMPF1.c &> /dev/null
        run_cbmc "${CMDS[0]}" $TMPF1.c "${CMDS[3]}"
    done
}

#run a single regression test for verification. this function takes
#four arguments: the name of the DASL file, the number of nodes, the
#number of rounds of sequentialization, and the expected result. this
#does double-buffer-based sequentialization
function verify_dbl {
    daslc $1 --seq=$2 --rounds=$3 --seq-dbl --out=$TMPF1.c &> /dev/null
    run_cbmc "${1}-dbl" $TMPF1.c "$4"
}

#test verification with double-buffer-based sequentialization
function test_verif_dbl {
    echo "==== testing verifier with double-buffer sequentialization"
    for i in "${TESTS[@]}"; do
        CMDS=($i)
        daslc ${CMDS[0]} --nodes ${CMDS[1]} --rounds ${CMDS[2]} --seq-dbl --seq-out $TMPF1.c &> /dev/null
        run_cbmc "${CMDS[0]}" $TMPF1.c "${CMDS[3]}"
    done
}

#print usage and exit
function usage {
    echo "Usage : $0 <option>"
    echo "Options:"
    echo "     -a: all tests"
    echo "     -p: parser tests"
    echo "    -va: verification tests with array-sequentialization"
    echo "   -vna: verification tests with no-array-sequentialization"
    echo "    -vs: verification tests with semantics-sequentialization"
    echo "    -vd: verification tests with double-buffer-sequentialization"
    exit 1
}

if [ "$#" == "0" ]; then
    usage
elif [ "$1" == "-a" ]; then
    test_parser
    test_verif_array
    test_verif_no_array
    test_verif_sem
    test_verif_dbl
elif [ "$1" == "-p" ]; then
    test_parser
elif [ "$1" == "-va" ]; then
    test_verif_array
elif [ "$1" == "-vna" ]; then
    test_verif_no_array
elif [ "$1" == "-vs" ]; then
    test_verif_sem
elif [ "$1" == "-vd" ]; then
    test_verif_dbl
else
    usage
fi
