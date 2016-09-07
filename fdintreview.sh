# *************************************************************************
# intreview
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to look at test run output
# Parameters: 1 - A description for the header
# Integration Group context in PATH_IG
#
. $PATH_SOURCE/fdsysreview.sh
. $PATH_SOURCE/fdtestsel.sh
intreview () {
description=$@
while :
do
opt=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=INTREVIEW: Review Outputs for $PATH_IG $description
PROMPT=Select option and Press RETURN
SEL_YES/COMM_NO/MENU
SCROLL
INTOUT:     Review the Integration Group Outputs/INTOUT:
INTBROWSE:  Review the Run Summary Report/INTBROWSE:
INTPRINT:   Print the Run Summary Report/INTPRINT:
INTSCEN:    Select Scenario Output for Detailed Review/INTSCEN:
EXIT:       Finish/EXIT:

EOF
` 
#
# Review all the dump files
#
case $opt in
EXIT:)
    break
;;
INTPRINT:)
    lpr $PATH_HOME/data/$PATH_IG/timout*
;;
INTBROWSE:)
  cat $PATH_HOME/data/$PATH_IG/timout* | sed 's///' |   pg -p '(Screen %d (h for help))' -s
;;
INTOUT:)
 echo Browse Integration Group Output with pg | pg -p '(Screen %d (h for help))' -s - $PATH_HOME/data/$PATH_IG/*.out
   ;;
INTSCEN:)
    test_select "INTSCEN: Select components of Group $PATH_IG for Detailed Review" ""
    for pid in $TEST_LIST
    do
        desc=`head -1 $PATH_HOME/scenes/runout$pid`
        sysreview $pid $desc
    done
;;
esac
done
return 0
}
