# **********************************************************************
# Include this file in a script from which it is to be used.
#
# Function to allow a user to select from a list of scripts for amending
# a scenario
# Arguments:
# 1 - The Selection Header
# 2 - The list of scripts
#
# Current Scenario (PATH_SCENE)
#
# Returns:
# List of script ids in SCRIPT_LIST
# Copyright (c) E2 Systems Limited 1993
#
script_selectam () {
head=$1
shift
extra=$1
shift
if [ "$1" = "" ]
then
SCRIPT_LIST=""
else
if [ ! -z "$PATH_SCENE" ]
then
    head="$head ($PATH_SCENE)"
fi
SCRIPT_LIST=`(
echo HEAD=$head
echo PROMPT=Select Scripts, and Press RETURN
echo SEL_YES/COMM_NO/SYSTEM
echo SCROLL
{
    if [ -z "$extra" ]
    then
        sel_str=
    else
        sel_str=\*
    fi
    for i in $*
    do
        echo  "$i" -
        tail -1 $PATH_HOME/scripts/$pid/f$i.awk
        echo "/$sel_str$i"
    done
    if [ ! -z "$extra" ]
    then
    for i in \`ls -1 $PATH_HOME/scripts/$pid/f*.awk | sed 's=//*f\\([^/.]*\\)\\.awk$=\\1='\`
    do
        while :
        do
            for j in $*
            do
                if [ "$i" = "$j" ]
                then
                    break 2
                fi
            done
            echo "$i" -
            tail -1 $PATH_HOME/scripts/$pid/f$i.awk
            echo "/$i"
            break
        done
    done
    fi
} | sed 'N
s.[/=#]. .g
N
s=\n= =g'
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
if [ "$SCRIPT_LIST" = " " -o "$SCRIPT_LIST" = "EXIT:" ]
then
    SCRIPT_LIST=""
fi
fi
export SCRIPT_LIST
return 0
}
