# fdig_select.sh
# **********************************************************************
# Function to allow a user to select from a list of integration groups
# Arguments:
# 1 - The Selection Header
#
# The Current PATH_SCENE 
#
# Returns:
# Integration Groups in IG_LIST
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
ig_select () {
    head=$1
    set -- $PATH_HOME/igs/Mk.*
    if [ "$1" = "$PATH_HOME/igs/Mk.*" ]
    then
        IG_LIST=""
    else
        IG_LIST=`(
echo HEAD=$head
echo PROMPT=Select Integration Groups, and Press RETURN
echo SEL_YES/COMM_NO/SYSTEM
echo SCROLL
for j in \` ls $PATH_HOME/igs/Mk.* | sed 's/.*Mk\.//' \`
do
echo $j -
head -1 $PATH_HOME/igs/Mk.$j
echo /$j
done | sed 'N
s.[/=#]. .g
N
s=\n= =g'
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
        if [ "$IG_LIST" = " " -o "$IG_LIST" = "EXIT:" ]
        then
            IG_LIST=""
        fi
    fi
export IG_LIST
return 0
}
