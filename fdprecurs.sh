# ************************************************************************
# fdprecurs.sh - function to accept a list of integration group
# precursors
#
# Arguments :
#     1 - heading
#     Others - list of current precursors
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Returns : Validated list of precursor/type pairs
#
accept_precurs () {
head=$1
shift
PREC_LIST=$@
mess=""
while :
do
j=1
PREC_LIST=`(
echo HEAD=$head
echo MESS=$mess
echo PROMPT=Enter Group Precursors, and Press RETURN
echo SEL_NO/COMM_NO/SYSTEM
echo SCROLL
for i in $PREC_LIST
do
    echo Precursor:/$i
    j=\`expr \$j + 1\`
done
until [ \$j -gt 19 ]
do
    echo Precursor:
    j=\`expr \$j + 1\`
done
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
if [ -z "$PREC_LIST" ]
then
    return 1
fi
TAG_LIST=""
mess=""
for i in $PREC_LIST
do
if [ -f $PATH_HOME/igs/Mk.$i ]
then
#
# The Precursor is another integration group
#
    TAG_LIST="${TAG_LIST} $i:ig"
elif which "$i" >/dev/null 2>&1
then
    TAG_LIST="${TAG_LIST} $i:x"
else
mess="^GIntegration Group Precursor $i Must be Integration Group or Executable"
    echo "$mess"
fi
done
if [ -z "$mess" ]
then
    PREC_LIST="$TAG_LIST"
    export PREC_LIST
    return 0
fi
done
}
