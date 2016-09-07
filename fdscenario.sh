:
#!/bin/sh5
#!/bin/sh
# fdscenario.sh
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Run and Review a Scenario
#
# Read in the required functions
. $PATH_SOURCE/fdvars.sh
. $PATH_SOURCE/fdscenrun.sh
if [ ! -z "$PATH_IG" ]
then
    curr="($PATH_IG"
else
    curr="(None"
fi
if [ ! -z "$PATH_SCENE" ]
then
    curr="$curr/$PATH_SCENE)"
else
    curr="$curr/None)"
fi
pid=$1 

#
SCRIPT_LIST=`ls $PATH_HOME/scripts/$pid/f*.awk | sed  's=.*/f\\([RS][^/]*\\)\\.awk$=\\1='`
    
# SCRIPT_LIST=`ls $PATH_HOME/scripts/$pid/f*.awk | sed 's/.*fd\\(.*\\)\\.awk/\\1/'`
desc=`head -1  $PATH_HOME/scenes/runout$pid`
echo "$desc" > $PATH_HOME/scenes/runout$pid
echo "Run Parameters" >> $PATH_HOME/scenes/runout$pid
echo "==============" >> $PATH_HOME/scenes/runout$pid
for i in $SCRIPT_LIST
do
    echo $pid : users 1 $i 1 think $PATH_THINK cps $PATH_CPS seed $$ >> $PATH_HOME/scenes/runout$pid

done
scenario_run "$pid" $SCRIPT_LIST
exit
