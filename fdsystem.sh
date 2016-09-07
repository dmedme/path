:
#!/bin/sh5
#!/bin/sh
# fdsystem.sh
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Handle the scenario facilities. Called from fdpathat.sh
#
# Read in the required functions
. $PATH_SOURCE/fdvars.sh
. $PATH_SOURCE/fdscrisel.sh
. $PATH_SOURCE/fdtestsel.sh
. $PATH_SOURCE/fdsysreview.sh
. $PATH_SOURCE/fddump_tran.sh
. $PATH_SOURCE/fdscenrun.sh
. $PATH_SOURCE/fdchoice.sh
new_pid=$$
while :
do
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
choice=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=SCENARIO: Administer Test Scenarios $curr
PROMPT=Select Menu Option and Press RETURN
SEL_YES/COMM_YES/MENU
SCROLL
SENCREATE:     Create a New System Test Scenario/SENCREATE1:
SENAMEND:      Change the Constituent Tests for a Scenario/SENAMEND:
SENCURR:       Change Current Scenario/SENCURR:
SENRUN:        Run a Previously Defined Scenario/SENRUN:
SENREVIEW:     Review the Results of a Prior Test/SENREVIEW:
SENPURGE:      Remove Prior Run Data/SENPURGE:
UNISCRIPT:     Maintain Scripts/UNISCRIPT:
SENDELETE:     Delete Scenario Altogether/SENDELETE:
EXIT:          Exit/EXIT:

HEAD=SENCREATE1: Create a new System Test Scenario
PARENT=SCRIPT:
PROMPT=Give your scenario an identifier and description
COMMAND=SENCREATE:
SEL_NO/COMM_NO/SYSTEM
SCROLL
Name :/$new_pid
Description :/

EOF
`
set -- $choice
case $1 in
EXIT:)
break
;;
UNISCRIPT:)
    fdscript.sh
;;
SENCREATE:)
#
# Run a chosen set of scripts as a scenario.
# -  invoke the script generation
# -  run the tests
# -  run and display the report
#
# Offer the user further choices:
# -  Review Summary Output
# -  Review dump files
# -  Exit
#
#
shift
PATH_SCENE=""
export PATH_SCENE
if [  -z "$1" ]
then
    echo  "No test reference supplied\c"
    sleep 1
else
pid=$1 
export pid
new_pid=`expr $new_pid + 1`
shift
desc=$@
script_select "SENCREATE2: Select Scripts to run for test $pid" "Y" ""
if [ -z "$SCRIPT_LIST" ]
then
    echo  "No scripts selected\c"
    sleep 1
else
mkdir $PATH_HOME/scripts/$pid
echo "$desc" > $PATH_HOME/scenes/runout$pid
echo "Run Parameters" >> $PATH_HOME/scenes/runout$pid
echo "==============" >> $PATH_HOME/scenes/runout$pid
for i in $SCRIPT_LIST
do
    ln $PATH_HOME/scripts/f$i.awk $PATH_HOME/scripts/$pid
    echo $pid : users 1 $i 1 think $PATH_THINK cps $PATH_CPS seed 1 >> $PATH_HOME/scenes/runout$pid

done
echo $pid : start_time 0 end_time 0 elapsed 0 >> $PATH_HOME/scenes/runout$pid
get_yes_or_no "SENCREATE3: Do you want to run the test now?"
if [ "$CHOICE" = "YES:" ]
then
scenario_run "$pid" "$desc" $SCRIPT_LIST
fi
fi
fi
PATH_SCENE="$pid"
export PATH_SCENE
;;
SENREVIEW:)
#
# Have another look at some results
#
    test_select "SENREVIEW: Select Scenarios to Review" ""
    if [ -z "$TEST_LIST" ]
    then
        echo  "No scenarios selected\c"
        sleep 1
    else
        for pid in $TEST_LIST
        do
            desc=`head -1 $PATH_HOME/scenes/runout$pid`
            sysreview $pid $desc
        done
    fi
;;
SENAMEND:)
#
# Alter the scripts that are part of a test
#
    test_select "SENAMEND1: Select Scenarios to Amend" ""
    if [ -z "$TEST_LIST" ]
    then
        echo  "No scenarios selected\c"
        sleep 1
    else
        for pid in $TEST_LIST
        do
        desc=`head -1 $PATH_HOME/scenes/runout$pid`
script_list=`ls -1 $PATH_HOME/scripts/$pid/fS*.awk  $PATH_HOME/scripts/$pid/fR*.awk 2>>/dev/null | sed  's=.*/f\\([^/]*\\)\\.awk$=\\1='`
script_select "SENAMEND2: Select Scripts to run for test $pid" "Y" "$script_list"
            if [ -z "$SCRIPT_LIST" ]
            then
                echo  "No scripts selected\c"
                sleep 1
            else
                rm -f $PATH_HOME/scripts/$pid/*
                echo "$desc" > $PATH_HOME/scenes/runout$pid
                echo "Run Parameters" >> $PATH_HOME/scenes/runout$pid
                echo "==============" >> $PATH_HOME/scenes/runout$pid
                for i in $SCRIPT_LIST
                do
                    ln $PATH_HOME/scripts/f$i.awk $PATH_HOME/scripts/$pid
echo $pid : users 1 $i 1 think $PATH_THINK cps $PATH_CPS seed 1 >> $PATH_HOME/scenes/runout$pid
    
                done
echo $pid : start_time 0 end_time 0 elapsed 0 >> $PATH_HOME/scenes/runout$pid
                get_yes_or_no "SENAMEND3: Do you want to run the test now?"
                if [ "$CHOICE" = "YES:" ]
                then
                    scenario_run "$pid" "$desc" $SCRIPT_LIST
                fi
            fi
        done
    fi
;;
SENRUN:)
#
# Run an existing scenario
#
    test_select "SENRUN: Select Scenarios to Rerun" ""
    if [ -z "$TEST_LIST" ]
    then
        echo  "No scenarios selected\c"
        sleep 1
    else
        for pid in $TEST_LIST
        do
            fdscenario.sh "$pid"
        done
    fi
;;
SENDELETE:)
#
# Delete an existing scenario
#
    test_select "SENDELETE: Select Scenarios to Delete" ""
    if [ -z "$TEST_LIST" ]
    then
        echo  "No scenarios selected\c"
        sleep 1
    else
        for pid in $TEST_LIST
        do
rm -rf $PATH_HOME/scripts/$pid $PATH_HOME/data/*/*out$pid $PATH_HOME/data/*out$pid $PATH_HOME/scenes/runout$pid $PATH_HOME/data/*$pid.* $PATH_HOME/data/*/*$pid.*
        done
    fi
    PATH_SCENE=""
    export PATH_SCENE
;;
SENPURGE:)
#
# Remove the detritus from a scenario
#
    test_select "SENPURGE: Select Scenarios to Purge" ""
    if [ -z "$TEST_LIST" ]
    then
        echo  "No scenarios selected\c"
        sleep 1
    else
        for pid in $TEST_LIST
        do
rm -f $PATH_HOME/data/*out$pid $PATH_HOME/data/*$pid.* $PATH_HOME/data/*/*out$pid $PATH_HOME/data/*/*$pid.*
        done
    fi
;;
SENCURR:)
#
# Select the current Scenario
#
    test_select "SENCURR: Select Current Scenarios" ""
    if [ -z "$TEST_LIST" ]
    then
        PATH_SCENE=""
    else
        set -- $TEST_LIST
        PATH_SCENE="$1"
    fi
    export PATH_SCENE
;;
esac
done
exit
