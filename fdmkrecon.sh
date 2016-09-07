# fdmkrecon.sh
# ************************************************************************
# fdmkrecon.sh - function to rebuild an Integration Group Makefile
#
# Arguments :
#     1 - Integration Group
#     2 - heading
#     3 - list of current precursors
#     4 - extra flag for test_select (do we offer items not included?)
#     5 - dialogue id core
#     6 - dialogue id sequence
#
# Returns :
#     The updated sequence
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to reconstitute an Integration Group Make File
make_recon () {
pid="$1"
desc="$2"
precursors="$3"
extra="$4"
men="$5"
seq="$6"
test_select "$men$seq: Select Scenarios to run for Group $pid" "$extra"
if [ -z "$TEST_LIST" ]
then
    echo  "No scenarios selected\c"
    sleep 1
    return 1
fi
seq=`expr $seq + 1 `
set -- $TEST_LIST
if [ $# != 1 ]
then
#
# Indicate the order in which they must be executed in
#
ORDER_LIST=`(
echo HEAD="$men$seq: $pid $desc"
echo PROMPT=Indicate Preferred Order and Press RETURN
echo SEL_NO/COMM_NO/SYSTEM
echo SCROLL
j=10
for i in $TEST_LIST
do
    head -1 $PATH_HOME/scenes/runout$i | sed "s.[=/]. .g
s=.*\$=\$i - &/\$j="
j=\`expr \$j + 10\`
done
echo
) | natmenu 3<&0 4>&1 </dev/tty >/dev/tty`
else
ORDER_LIST=1
fi
#
# Now create the Integration Group. An Integration Group is implemented as
# a Makefile.
#
echo "#" $desc > $PATH_HOME/igs/Mk.$pid
echo "#====================">> $PATH_HOME/igs/Mk.$pid
rm -f $PATH_HOME/scenes/$pid/*
#
# Now need to get the Integration Groups in the Desired Order
# Get the look-up table loaded first, then the group identifiers,
# and output them in the look-up table order
#
(
for i in $ORDER_LIST
do
  echo 0:$i
done
for i in $TEST_LIST
do
    ln $PATH_HOME/scenes/runout$i $PATH_HOME/scenes/$pid >/dev/null 2>&1
    echo $i
done
) | $PATH_AWK -F: 'BEGIN {flag = 0}
/^0:/ && (flag == 0) { ord[NR] = substr($0,3)
ptr[NR] = NR
next}
flag == 0 { cnt = NR - 1
flag = 1
}
{ ind = NR - cnt
out[ind] = $1
}
END { 
for (i = 1; i < cnt; i++)
for (j = cnt; j > i; j--)
if  (ord[i] < ord[j])
{
    x=ord[i]
    y=ptr[i]
    ord[i]=ord[j]
    ptr[i]=ptr[j]
    ord[j]=x
    ptr[j]=y
}
#
# Now write out the Makefile.
#
i = ptr[1]
scen=out[i]
print "I_" pid ".out:S_" scen ".out"
print "\t@echo Integration Group " pid " Completed"
for (i = 2; i <= cnt; i++)
{
j=ptr[i]
next_scen=out[j]
print "S_" scen ".out:S_" next_scen ".out"
print "\tfdscenbat.sh " scen  " > S_" scen ".out 2>&1"
scen = next_scen
}
i = ptr[cnt]
scen=out[i]
print "S_" scen ".out: precursor.out"
print "\tfdscenbat.sh " scen  " > S_" scen ".out 2>&1"
}'  pid="$pid" - >>  $PATH_HOME/igs/Mk.$pid
(
for i in $precursors
do
   echo $i
done
) | sed 's=.*/==g' | $PATH_AWK -F: '{ precursor[NR] = $1
ptype[NR] = $2
}
END {
printf "precursor.out:"
for (i = 1; i < NR; i++)
     print "P_" precursor[i] ".out \\\\"
print "P_" precursor[i] ".out"
print "\t@echo \"All precursors up to date\""
for (i = 1; i <= NR; i++)
{
print "P_" precursor[i] ".out:"
if (ptype[i] == "ig" )
print "\tmake -f '$PATH_HOME'/igs/Mk." precursor[i]  " > P_" precursor[i] ".out 2>&1"
else
print "\t" precursor[i]  " > P_" precursor[i] ".out 2>&1"
}
}'  >>  $PATH_HOME/igs/Mk.$pid
#
# The following are provided for dialogue sequencing
#
seq=`expr $seq + 1 `
export seq
return 0
}
