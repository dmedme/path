# fdevent.sh
# **********************************************************************
# Include this file in a script from which it is to be used.
#
# Function to allow a user to select from a list of event reports
# Arguments:
# 1 - The Selection Header
# 2 - The list of scripts
#
#
# Returns:
# List of event ids in SCRIPT_LIST
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
script_selecta () {
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
if [ -z "$extra" ]
then
for i in $*
do
echo $i -
tail -1 $PATH_HOME/data/f$i.rep
echo /$i
done
else
for i in $*
do
echo \* $i -
tail -1 $PATH_HOME/data/f$i.rep
echo /$i
done
for i in \`ls $PATH_HOME/data/f*.rep | sed 's/.*f\\(.*\\)\\.awk/\\1/'\`
do
echo $i -
tail -1 $PATH_HOME/data/f$i.rep
echo /$i
done
fi | sed 'N
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
