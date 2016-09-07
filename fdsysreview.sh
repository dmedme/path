# fdsysreview.sh
# *************************************************************************
# sysreview
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to look at test run output
# Parameters: 1 - The test run identifier
#             The rest are a description for the header
# Integration Group context in PATH_IG
#
. $PATH_SOURCE/fddump_tran.sh
. $PATH_SOURCE/fdscrisel.sh
sysreview () {
pid=$1
shift
description=$@
while :
do
if [ ! -z "$PATH_IG" ]
then
    curr="($PATH_IG)"
else
    curr="(None)"
fi
opt=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=SENREVIEW: Review Outputs for $pid $description $curr
PROMPT=Select option and Press RETURN
SEL_YES/COMM_NO/MENU
SCROLL
SENREPORT:  Review the Run Summary Report/SENREPORT:
SENPRINT:   Print the Run Summary Report/SENPRINT:
SENALL:     Select Screen Output for Review (All sessions)/SENALL:
SENFAIL:    Select Screen Output for Review (Failed sessions)/SENFAIL:
EXIT:       Finish/EXIT:

EOF
` 
if [ "$opt" = "EXIT:" ]
then
     break
elif [ "$opt" = "SENALL:" ]
then
#
# Review all the dump files
#
build_tran $pid
allscripts=`echo $TRAN_LIST | sed 's/:[0-9]*//g'`
script_select "SENALL: Select Screen Files to Review" "" $allscripts
    if [ -z "$SCRIPT_LIST" ]
    then
        echo  "No scripts selected\c"
        sleep 1
    else
#
# Get a list of dump files
#
dump_from_script "$pid" "$TRAN_LIST" "$SCRIPT_LIST"
        echo Browse Selected Scripts with pg | pg -p '(Screen %d (h for help))' -s - $DUMP_LIST
    fi
elif [ "$opt" = "SENFAIL:" ]
then
failures=`$PATH_AWK -F: '$6 == "Z" {
print $2}' $PATH_HOME/data/$PATH_IG/comout$pid`
    if [ -z "$failures" ]
    then
        echo  "No scripts failed!\c"
        sleep 1
    else
build_tran $pid
script_list=`for i in $TRAN_LIST $failures
do
echo $i
done | $PATH_AWK -F: '{ if (NF == 2)
s[$2] = $1
else
print  s[$1]
}'`

script_select "SENFAIL: Select Failed Transaction Screen Dumps to Review" "" $script_list
    if [ -z "$SCRIPT_LIST" ]
    then
        echo  "No scripts selected\c"
        sleep 1
    else
dump_from_script "$pid" "$TRAN_LIST" "$SCRIPT_LIST"
        echo Browse Selected Failed Screen Output with pg |pg -p '(Screen %d (h for help))' -s - $DUMP_LIST
    fi
    fi
elif [ "$opt" = "SENPRINT:" ]
then
    lpr $PATH_HOME/data/$PATH_IG/timout$pid
else
 cat $PATH_HOME/data/$PATH_IG/timout$pid | sed 's///' | pg -p '(Screen %d (h for help))' -s
fi
done
return 0
}
