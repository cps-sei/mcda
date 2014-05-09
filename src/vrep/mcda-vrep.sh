#!/bin/bash

INIT_PORT="19905"

function usage {
    echo "Usage : $0 <node-num> <out-dir> <node-exec> <node-1-args> ... <node-N-args>"
    echo "        where N = node-num"
}

#get the number of nodes
NODENUM=$1
#echo $NODENUM

if [ "$#" != $(expr $NODENUM + 3) ]; then
    usage
    exit 1
fi

#get the output directory
shift 1
OUTDIR="$1"
#echo $OUTDIR
shift 1

#create the output directory and get its realpath
rm -fr $OUTDIR; mkdir $OUTDIR
OUTDIR=$(realpath $OUTDIR)

#get the node executable
NODECMD="$1"
#echo $NODECMD
shift 1


#get the node arguments
declare -a NODEARGS
for i in `seq 1 $NODENUM`; do
    NODEARGS[${i}]="$1"
    #echo ${NODEARGS[${i}]}
    shift 1
done

#save old and create new VREP remoteApiConnections.txt file
RAC=$VREP_MCDA_ROOT/remoteApiConnections.txt
if [ -e $RAC ]; then
    rm -f $RAC.saved.mcda-vrep
    mv $RAC $RAC.saved.mcda-vrep
fi
touch $RAC
PORT=$INIT_PORT
for i in `seq 1 $NODENUM`; do 
    echo "portIndex${i}_port                 = $PORT" >> $RAC
    echo "portIndex${i}_debug                = false" >> $RAC
    echo "portIndex${i}_syncSimTrigger       = true" >> $RAC
    echo "" >> $RAC
    PORT=$(expr $PORT + 1)
done

#cat $RAC

#save the VREP system/settings.dat
SDF=$VREP_MCDA_ROOT/system/settings.dat
cp $SDF $SDF.saved.mcda-vrep

#start vrep
echo "starting VREP .. output is in $OUTDIR/vrep.out ..."
(cd $VREP_MCDA_ROOT ; ./vrep.sh $MCDA_ROOT/src/vrep/mcda.ttt &> $OUTDIR/vrep.out &)
sleep 10

#restore old VREP remoteApiConnections.txt file
mv $RAC.saved.mcda-vrep $RAC

#start the nodes
PORT=$INIT_PORT
for i in `seq 1 $NODENUM`; do
    echo "starting node $i .. output is in $OUTDIR/node-$i.out"
    ($NODECMD ${NODEARGS[${i}]} --vrep-host 127.0.0.1 --vrep-port $PORT &> $OUTDIR/node-$i.out &)
    PORT=$(expr $PORT + 1)
done

printf "press enter terminate the simulation ..."
read X

#kill nodes and VREP
killall $NODECMD vrep vrep.sh

#restore the VREP system/settings.dat
cp $SDF.saved.mcda-vrep $SDF

#all done
exit 0
