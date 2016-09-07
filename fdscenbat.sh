:
#!/bin/sh5
#!/bin/sh
# fdscenbat.sh
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Run a Scenario in Batch
#
# Read in the required functions
. $PATH_SOURCE/fdvars.sh
. $PATH_SOURCE/fdscenexec.sh
. $PATH_SOURCE/fddump_tran.sh
pid=$1 

#
SCRIPT_LIST=`ls $PATH_HOME/scripts/$pid/f*.awk | sed  's=.*/f\\([RS][^/]*\\)\\.awk$=\\1='`
    
# SCRIPT_LIST=`ls $PATH_HOME/scripts/$pid/fd*.awk | sed 's/.*fd\\(.*\\)\\.awk/\\1/'`
if [ -z "$SCRIPT_LIST" ]
then
    echo "Empty Scenario\c"
    sleep 1
    exit
fi
desc=`head -1  $PATH_HOME/scenes/runout$pid`
echo $desc > $PATH_HOME/scenes/runout$pid
echo "Run Parameters" >> $PATH_HOME/scenes/runout$pid
echo "==============" >> $PATH_HOME/scenes/runout$pid
scenario_exec $pid $SCRIPT_LIST
exit
