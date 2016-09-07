# fdscenrun.sh
# **************************************************************************
# scenario_run
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to actually run a scenario
# Parameters: 1 - ID
#             The rest - scripts to run
#
# The current Integration Group, PATH_IG
#
. $PATH_SOURCE/fdscenexec.sh
. $PATH_SOURCE/fddump_tran.sh
. $PATH_SOURCE/fdsysreview.sh
scenario_run () {
pid=$1
build_tran $pid
allscripts=`echo $TRAN_LIST | sed 's/:[0-9]*//g'`
if [ -z "$allscripts" ]
then
    echo "Empty Scenario\c"
    sleep 1
    return
fi
desc=`head -1 $PATH_HOME/scenes/runout$pid`
echo $desc > $PATH_HOME/scenes/runout$pid
echo "Run Parameters" >> $PATH_HOME/scenes/runout$pid
echo "==============" >> $PATH_HOME/scenes/runout$pid
scenario_exec $pid $allscripts
fdsysrep.sh $pid | pg -p '(Screen %d (h for help))' -s
#
# Ask if the user wants to review anything
#
sysreview $pid $desc
return 0
}
