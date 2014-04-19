#!/bin/bash

#check arguments
if [ "$#" != "1" ]; then
    echo "Usage : $0 X"
    echo "    X = package name. will be created as X.tgz"
    exit 1
fi

PN="$1"
TMPD=$(mktemp -d)
mkdir $TMPD/$PN
cp -a README.txt ../../LICENSE.txt run-exp.sh \
sync-coll-avoid.bug1.cub  sync-coll-avoid.bug2.cub  sync-coll-avoid.ok.cub  \
sync-mutex.bug1.cub  sync-mutex.bug2.cub  sync-mutex.ok.cub \
sync-coll-avoid.bug1.in   sync-coll-avoid.bug2.in   sync-coll-avoid.ok.in \
sync-mutex.bug1.in   sync-mutex.bug2.in   sync-mutex.ok.in $TMPD/$PN
(cd $TMPD; tar cvfz $PN.tgz $PN)
echo "package $TMPD/$PN.tgz created"
