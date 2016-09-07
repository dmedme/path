:
#!/bin/sh
# fdmenu; list of options for forms driver benchmark
# @(#) $Name$ $Id$
# Copyright (c) 1993 E2 Systems
# Essentially provided to give a high level index of what is there;
# the file Makefile identifies most of the components
#
set -x
PATH_AWK=${PATH_AWK:-nawk}
export PATH_AWK
if [ $# -lt 2 ]
then
   echo Supply a PID and the number of users to maintain
   exit 1
fi
pid=$1
sim_users=$2
#
# not used in this case, but preserved in order to satisfy argument
# count requirements
#
/usr/lib/sa/sadc 60 30 sad$pid &
sad_pid=$!
#start processing
#
sav_glob=`$PATH_AWK < runout$pid '/:/ && !/end_time/ { print $4}'`
set -- $sav_glob
bundle=$#
run_users=0
next_i=""
bundle=` expr $bundle + 1 `
i=0
while [ $i -lt $bundle ]
do
 next_i="$next_i 0"
 i=`expr $i + 1`
done
#
# We want to start one of each until they are gone
# We need to keep the next number on each.
#
#
# Exit the loop after an hour
#
trap "break 3" 15
sav_pid=$$
(sleep 1200; kill -15 $sav_pid) &
start_time=`tosecs`
ps -ef > ps_$start_time
#
# Loop, exited on receipt of signal
#
while :
do
    while [ $run_users -lt $sim_users ]
    do
        globusers=$sav_glob
        nnext=""
        j=1
#
# Loop - start 1 from each bundle
#
        while [ $j -lt $bundle ]
        do
#
# Get the next to do
#
            set -- $next_i
            i=$1
            shift
            next_i="$*"
#
# Get the number to do
#
            set -- $globusers
            nusers=$1
            shift
            globusers="$*" 
#
# If still some to do, do one
#
            if [ $i -lt $nusers ]
            then
                outfile=log$pid.$j.$i
                infile=echo$pid.$j.$i
                dumpfile=dump$pid.$j.$i
#
# Start the pty driver
#
#                ptydrive < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
 ptydrive -d < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
# stagger the start up
#
                i=`expr $i + 1`
                run_users=`expr $run_users + 1`
            else
#
# Otherwise, reset to the beginning of the list
#
                i=1
            fi
            nnext="$nnext $i"
            j=`expr $j + 1`
        done
        next_i=$nnext
    done
#
# How many have we got running
#
    sleep 120
    run_users=`ps -eda | grep -c ptydrive`
done
#
# Run for one hour, then shut it all down
#
# ********************************************************
# kill off still-running demo processes
#
# Stagger the death so that the Accounting Records do get written out.
# 
ps -e | egrep "ptydrive" | $PATH_AWK '!/grep/ {print "kill -15 " $1 " ; sleep 1;"}' | sh
wait
end_time=`tosecs`
ps -ef > ps_$end_time
#
echo `expr \( $end_time - $start_time \)` > elapsed$pid
echo $$ : start_time $start_time end_time $end_time elapsed `expr \( $end_time - $start_time \)` >> runout$pid
kill -9 $sad_pid
# merge the output files
cat log${pid}* | sort -n -t: > comout$pid
rm -f log${pid}*
#
fdreport.sh $pid $start_time $end_time
exit
