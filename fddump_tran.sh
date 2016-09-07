# fddump_tran.sh
#
# Translate script ids into dump file names for a given run
#
# Arguments:
# -  1 Scenario Identifier
# -  2 Translation Table
# -  3 Items to be translated
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
dump_from_script () {
pid=$1
tran=$2
scrips=$3
DUMP_LIST=`for i in $tran $scrips
do
echo $i
done | awk -F: '{ if (NF == 2)
s[$1] = $2
else
print "'$PATH_HOME/data/$PATH_IG/dump$pid'." s[$1] ".*"
}'`
export DUMP_LIST
return 0
}
#
# Build a bundle/script translation list for pid
# Arguments : 1 - Scenario Identifier
#
build_tran () {
pid=$1
TRAN_LIST=`$PATH_AWK 'BEGIN {x=1}
/:/ && ($5 != "end_time") { print $5 ":" x
x++ }' $PATH_HOME/scenes/runout$pid`
export TRAN_LIST
return 0
}
