# fdscenexec.sh
# **************************************************************************
# scenario_exec
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to actually execute a set of scenario scripts.
# Parameters: 1 - ID
#             The rest - scripts to run
#
# The current Integration Group, PATH_IG
#
# This script can be run interactively, or in batch 
#
scenario_exec () {
pid=$1
shift
script_list=$@
if [ -z "$script_list" ]
then
    echo "No scripts to Run\c"
    sleep 1
    return
fi
think=$PATH_THINK
cps=$PATH_CPS
seed=$$
nusers=1
ntrans=1
    bundle=1
    for tran in $script_list
    do
     fname=$PATH_HOME/scripts/$pid/f$tran.awk 
     if [ $bundle = 1 ]
     then
     PATH_COMMAND=`sed -n '1s/#//
1p
1q' < $fname`
     export PATH_COMMAND
     PATH_INIT=`sed -n '2s/#//
2p
2q' < $fname`
     export PATH_INIT
     fi
     fdsetup.sh $fname $pid $bundle $cps $think $nusers $ntrans $usr_id $seed $server_1 $server_2 &
     echo $pid : users $nusers $tran $ntrans think $think cps $cps seed $seed >> $PATH_HOME/scenes/runout$pid
     bundle=`expr $bundle + 1`
   done
echo  "Waiting for all the setups to finish. It will bleep at you.\c"
#
# Notify user and wait for acknowledgement
wait
echo  " \c"
j=1
START_TIME=`tosecs`
while [ $j -lt $bundle ]
do
i=0
while [ $i -lt $nusers ]
do
outfile=log$pid.$j.$i
infile=echo$pid.$j.$i
dumpfile=$PATH_HOME/data/$PATH_IG/dump$pid.$j.$i
#
# Start the pty driver
#
if [ -z "$PATH_COMMAND" ]
then
    if [ -z "$PATH_INIT" ]
    then
        ptydrive -d < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
    else
    ptydrive -i "$PATH_INIT" -d < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
    fi
else
    if [ -z "$PATH_INIT" ]
    then
ptydrive -x "$PATH_COMMAND" -d < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
    else
ptydrive -i "$PATH_INIT" -x "$PATH_COMMAND" -d < $infile $outfile $pid $j $i >$dumpfile 2>&1 &
    fi
fi
sleep  5
 i=`expr $i + 1`
done
 j=`expr $j + 1`
done
echo  ' Please wait\c'
wait
END_TIME=`tosecs`
echo $pid : start_time $START_TIME end_time $END_TIME elapsed `expr \( $END_TIME - $START_TIME \)` >> $PATH_HOME/scenes/runout$pid
cat log${pid}* | sort -n -t: > $PATH_HOME/data/$PATH_IG/comout$pid
rm -f log${pid}* echo${pid}*
fdcleanup.sh
export START_TIME END_TIME
return 0
}
