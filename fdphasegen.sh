#!/bin/sh
# fdstepgen.sh - autogen everything that moves
id=$1
for i in "" _1 _2 _3 _4 _5
do
    if [ -f runout$id$i ]
    then
        fdechogen.sh $id$i
    fi
done
wait
exit
