#!/bin/bash

INIT_PORT="19905"
declare -a NODEARGS

#get the number of nodes
NODENUM=$1
echo $NODENUM
shift 1

#get the node executable
NODECMD="$1"
echo $NODECMD
shift 1

#get the node arguments
for i in `seq 1 $NODENUM`; do
    NODEARGS[${i}]="$1"
    echo ${NODEARGS[${i}]}
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

cat $RAC

#start vrep
(cd $VREP_MCDA_ROOT ; ./vrep.sh $MCDA_ROOT/simulation/vrep/mcda.ttt&)
sleep 10

#restore old VREP remoteApiConnections.txt file
mv $RAC.saved.mcda-vrep $RAC

#start the nodes
PORT=$INIT_PORT
for i in `seq 1 $NODENUM`; do
    ($NODECMD ${NODEARGS[${i}]} --vrep-host 127.0.0.1 --vrep-port $PORT&)
    PORT=$(expr $PORT + 1)
done
