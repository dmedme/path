# fdfilsel.sh
# Include this file in a script from which the file_select
# function is to be executed.
# **********************************************************************
# Function to allow a user to select from a list of files
# Arguments:
# 1 - The File Selection Header
# 2 - The File Selection String
#
# Returns:
# List of files in FILE_LIST
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
file_select () {
FILE_LIST=`(
echo HEAD=$1
echo PROMPT=Select Files, and Press RETURN
echo SEL_YES/COMM_NO/SYSTEM
echo SCROLL
ls -lt $2 2>&1 | sed '/\// s=\(.* \)\([^ ]*/\)\([^/]*\)$=\1\3/\2\3=
/\//!{
s=.* \([^ ]*\)$=&/\1=
}'
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
if [ "$FILE_LIST" = " " -o "$FILE_LIST" = "EXIT:" ]
then
    FILE_LIST=""
fi
export FILE_LIST
return
}
