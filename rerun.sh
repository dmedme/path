#!/bin/sh
# rerun.sh - Repeatedly execute ptydrive when it dies
#
if [ $# != 5 ]
then
    echo Provide a logfile run_id bundle thread and input file for ptydrive
    exit
fi
logfile=$1
pid=$2
bundle=$3
g=$4
infile=$5
cat /dev/null >$logfile.sav
while :
do
ptydrive $logfile $pid $bundle $g < $infile
cat $logfile >>$logfile.sav
done
