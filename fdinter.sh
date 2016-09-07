:
#!/bin/sh5
#!/bin/sh
# fdinter.sh
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Handle the Integration Group facilities. Called from fdpathat.sh
#
# Read in the required functions
. $PATH_SOURCE/fdvars.sh
. $PATH_SOURCE/fdig_select.sh
. $PATH_SOURCE/fdtestsel.sh
. $PATH_SOURCE/fdintreview.sh
. $PATH_SOURCE/fddump_tran.sh
. $PATH_SOURCE/fdintrun.sh
. $PATH_SOURCE/fdprecurs.sh
. $PATH_SOURCE/fdmkrecon.sh
. $PATH_SOURCE/fdchoice.sh
new_pid=$$
while :
do
if [ ! -z "$PATH_IG" ]
then
    curr="($PATH_IG)"
else
    curr="(None)"
fi
choice=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=INTER: Administer Integration Groups of Tests $curr
PROMPT=Select Menu Option and Press RETURN
SEL_YES/COMM_YES/MENU
SCROLL
INTCREATE:  Create a New Integration Group/INTCREATE1:
INTAMEND:   Change the Constituent Elements of an Integration Group/INTAMEND:
INTCURR:    Change Current Integration Group/INTCURR:
INTRUN:     Execute a Previously Defined Integration Group/INTRUN:
INTREVIEW:  Review the output of a Previously Run Integration Group/INTREVIEW:
INTPURGE:   Remove Prior Run Data/INTPURGE:
SCENARIO:   Maintain System Test Scenarios/SCENARIO:
INTDELETE:  Delete Integration Group Altogether/INTDELETE:
EXIT:       Exit/EXIT:

HEAD=INTCREATE1: Create a new Integration Group
PARENT=SCRIPT:
PROMPT=Give your Integration Group an identifier and description
COMMAND=INTCREATE:
SEL_NO/COMM_NO/SYSTEM
SCROLL
Name        :/$new_pid
Description :

EOF
`
set -- $choice
case $1 in
EXIT:)
break
;;
SCENARIO:)
    fdsystem.sh
;;
INTCREATE:)
shift
PATH_IG=""
export PATH_IG
#
# The Integration Group Identifier
#
    if [  -z "$1" ]
    then
        echo  "No Integration Group Reference supplied\c"
        sleep 1
        continue
    fi
    pid=$1 
    export pid
    new_pid=`expr $new_pid + 1`
    shift
    desc=$@
#
# The Integration Group Precursors
#
    if accept_precurs "INTCREATE2: $pid - $desc"
    then
        :
    else
        echo  "No Integration Group Precursor supplied\c"
        sleep 1
        continue
    fi
    PATH_IG=""
    export PATH_IG
    mkdir $PATH_HOME/data/$pid
    mkdir $PATH_HOME/scenes/$pid
    if make_recon "$pid" "$desc" "$PREC_LIST" "" "INTCREATE" 3
    then
        PATH_IG=$pid
        export PATH_IG
        get_yes_or_no "INTCREATE$seq: Do you want to run the group now?"
        if [ "$CHOICE" = "YES:" ]
        then
            int_run $desc
        fi
    fi
;;
INTREVIEW:)
#
# Have another look at some results
#
    ig_select "INTREVIEW: Select Integration Tests to Review"
    if [ -z "$IG_LIST" ]
    then
        echo  "No Integration Groups selected\c"
        sleep 1
    else
        for PATH_IG in $IG_LIST
        do
            export PATH_IG
            desc=`sed -n '/========/q
s/^# //
p' < $PATH_HOME/igs/Mk.$PATH_IG`
            intreview $desc
        done
    fi
;;
INTAMEND:)
#
# Alter the scripts that are part of a test
#
    ig_select "INTAMEND1: Select Integration Groups to Amend"
    if [ -z "$IG_LIST" ]
    then
        echo  "No Integration Groups selected\c"
        sleep 1
    else
        for PATH_IG in $IG_LIST
        do
            export PATH_IG
desc=`(
echo HEAD=INTAMEND2: Amend Integration Group Description
echo PROMPT=Revise Description and press RETURN
echo SEL_NO/COMM_NO/SYSTEM
echo SCROLL
sed -n '/========/q
s/^# //
p' < $PATH_HOME/igs/Mk.$PATH_IG | $PATH_AWK 'BEGIN { l = 0 ; x = ""}
{ if ((l + length) < 38)
{
    if (l == 0)
        print "Description :/" $0
    else
    {
        print "Description :/" x " " $0
    }
}
else
{
    x = x " " $0
    for (i = 39; i > 0 && substr(x,i,1) != " "; i--);
    print "Description :/" substr(x,1,i)
    l = length(x) - i
    x = substr(x,i+1,l)
}
}
END { if (l != 0)
        print "Description :/" x
}'
echo "Description :"
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
            prec_list=`sed -n < $PATH_HOME/igs/Mk.$PATH_IG '/^precursor.out/ {
s/.*://
:top
s/^ *P_//
s/\.out\([\\\\]*\)/\\1/
tall
:all
s/\\\\$//
p
tmore
bend
:more
n
btop
:end
q
}'`
accept_precurs "INTAMEND3: Amend Precursors for $PATH_IG - $desc" "$prec_list"
            if make_recon "$PATH_IG" "$desc" "$PREC_LIST" "Y" "INTAMEND" 4
            then
            get_yes_or_no "INTAMEND$seq: Do you want to run group $PATH_IG now?"
                if [ "$CHOICE" = "YES:" ]
                then
                    int_run "$desc"
                fi
            fi
        done
    fi
;;
INTRUN:)
#
# Run an existing integration group
#
    ig_select "INTRUN: Select Integration Groups to Run"
    if [ -z "$IG_LIST" ]
    then
        echo  "No Integration Groups selected\c"
        sleep 1
    else
        for PATH_IG in $IG_LIST
        do
            export PATH_IG
            desc=`sed -n '/========/q
s/^#//
p' < $PATH_HOME/igs/Mk.$PATH_IG`
            int_run "$desc"
        done
    fi
;;
INTDELETE:)
#
# Delete an existing Integration Group
#
    ig_select "INTDELETE: Select Integration Groups to Delete"
    if [ -z "$IG_LIST" ]
    then
        echo  "No Integration Groups selected\c"
        sleep 1
    else
        for pid in $IG_LIST
        do
    rm -rf $PATH_HOME/igs/Mk.$pid $PATH_HOME/data/$pid $PATH_HOME/scenes/$pid
        done
    fi
    PATH_IG=""
    export PATH_IG
;;
INTPURGE:)
#
# Remove the detritus from an integration group
#
    ig_select "INTPURGE: Select Integration Groups to Purge"
    if [ -z "$IG_LIST" ]
    then
        echo  "No Integration Groups selected\c"
        sleep 1
    else
        for pid in $IG_LIST
        do
            rm -f $PATH_HOME/data/$pid/*
        done
    fi
;;
INTCURR:)
#
# Select the Current Integration Group
#
    ig_select "INTCURR: Select Current Integration Group"
    if [ -z "$IG_LIST" ]
    then
        PATH_IG=""
    else
        set -- $IG_LIST
        PATH_IG="$1"
    fi
    export PATH_IG
;;
esac
done
exit
