:
# valfetch.sh - feed on standard out the requisite number of codes
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Arguments: 1 - file to do
#            2 - bundle
#            3 - thread (conventionally g)
#            4 - ntrans (the number of codes)
#
# Processing:
# - Get the number of records in the file
# - Work out where to start reading ((bundle + 1) * thread * ntrans) % recs 
# - Loop; repeatedly scan the file until one of the following is true:
#   - All records requested
#   - All records in the file have been done
#
#set -x
if [ -z "$PATH_AWK" ]
then
if [ "$PATH_OS" = ULTRIX -o "$PATH_OS" = AIX ]
then
  PATH_AWK=awk
elif [ "$PATH_OS" = OSF  ]
then
  PATH_AWK=gawk
else
  PATH_AWK=awk
fi
if [ $# -lt 4 ]
then
    exit 1
fi
fi
file=$1
bundle=$2
thread=$3
ntrans=$4
nrecs=`$PATH_AWK 'END { print NR}' $file`
start=`expr \` expr \\\` expr $bundle \\\` \\* $thread \\* $ntrans \` % $nrecs` 
can_do=`expr $start + $ntrans`
#
# Now do it
#
if [ $can_do -le $nrecs ]
then
    $PATH_AWK 'NR > start && NR <= can_do { print }' can_do=$can_do start=$start $file
else
    $PATH_AWK 'NR > start { print }' start=$start $file
    if [ $ntrans -gt $nrecs ]
    then
        $PATH_AWK 'NR <= start { print }' start=$start $file 
    else
        end=`expr $ntrans - $nrecs + $start`
        $PATH_AWK 'NR <= end { print }' end=$end $file
    fi
fi
exit
