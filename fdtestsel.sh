# fdtestsel.sh
# **********************************************************************
# Function to allow a user to select from a list of system tests
# Arguments:
# 1 - The Selection Header
#
# The Current Integration Group, PATH_IG
#
# Returns:
# System test definition files in TEST_LIST
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
test_select () {
head=$1
extra=$2
set -- $PATH_HOME/scenes/runout*
if [ "$1" = "$PATH_HOME/scenes/runout*" ]
then
TEST_LIST=""
else
TEST_LIST=`(
echo HEAD=$head
echo PROMPT=Select Tests, and Press RETURN
echo SEL_YES/COMM_NO/SYSTEM
echo SCROLL
{
if [ -z "$extra" ]
then
sel_str=
else
sel_str=\*
fi
if [ ! -z "$PATH_IG" ]
then
for j in \` sed -n < $PATH_HOME/igs/Mk.$PATH_IG '/^[^.]*\.out:[S]_/ {
s/^[^.]*\.out:[S]_\([^.]*\)\.out$/\1/
p
}' \`
do
   if [ -f  $PATH_HOME/scenes/$PATH_IG/runout$j ]
   then
       echo  "$j" -
       head -1 $PATH_HOME/scenes/$PATH_IG/runout$j
       echo "/$sel_str$j"
   fi
done
fi
if [ ! -z "$extra" -o -z "$PATH_IG" ]
then
    for j in \` ls $PATH_HOME/scenes/runout* | sed 's/.*runout//' \`
    do
    if [ -z "$PATH_IG" -o ! -f  $PATH_HOME/scenes/$PATH_IG/runout$j ]
    then
        echo "$j" -
        head -1 $PATH_HOME/scenes/runout$j
        echo /"$j"
    fi
    done
fi
} | sed 'N
s.[/=#]. .g
N
s=\n= =g'
echo
) | $PATH_AWK '
# BEGIN does not work here on SUN
$0 ~ /\/\*/ && x > -1 { l[x++] = $0; next}
{if (x > -1)
{
 for (i = x - 1; i > -1; i--)
    print l[i]
x = 0
}
print $0
}
END { if (x > -1)
{
 for (i = x - 1; i > -1; i--)
    print l[i]
}
}' | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
if [ "$TEST_LIST" = " " -o "$TEST_LIST" = "EXIT:" ]
then
    TEST_LIST=""
fi
fi
export TEST_LIST
return 0
}
