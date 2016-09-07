# fdesel.sh	
# **********************************************************************
# Include this file in a script from which it is to be used.
#
# Function to allow a user to select from a list of echo files
# Arguments:
# 1 - The Selection Header
# 2 - The list of scripts
#
#
# Returns:
# List of echo files 
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
script_selecte () {
head=$1
shift
extra=$1
shift
if [ "$1" = "" -a "$extra" = "" ]
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
tail -1 $PATH_HOME/echo/$i.ech
echo /$i
done
else
for i in $*
do
echo \* $i -
tail -1 $PATH_HOME/echo/$i.ech
echo /$i
done
for i in \`ls -1 $PATH_HOME/echo/$PATH_SCENE/E*.ech 2>/dev/null | sed 's=.*/\\([^/]*\\)\\.ech$=\\1='\`
do
echo $i -
tail -1 $PATH_HOME/echo/$i.ech
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
