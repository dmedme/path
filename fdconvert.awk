#
# fdconvert.awk; takes the output of harninter.sh and converts it
# to a script generator file
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Copyright (c) 1993 E2 Systems Limited.
#
# Scan for event, duration and comment details 
# in_comment is a flag for whether we are:
# - looking for a comment ( = 0)
# - processing data hidden in a comment ( = 1)
# - looking for the end of a comment ( = 2)
#
# We pass the PATH_INIT value in so that the first event can be discarded
# if necessary
#
BEGIN {in_comment = 0
loop_var=0
}
# This assumes that the first three lines are the event definition
# its invocation and the match comment. Why are the former not in comments?
path_init != "" && NR < 3 { next }
/^#$/ {
if (in_comment == 1)
{
# - processing data hidden in a comment ( = 1)
   in_comment = 0
   next
}
else
if (in_comment == 2)
{
# - looking for the end of a comment ( = 2)
   in_comment = 0
}
else
{
# processing a comment
x = getline
if ($0 == "REMARKS")
{
    in_comment = 2
    print "print \"#" $0 "\"";
    next
}
else
{
in_comment = 1;
if (substr($0,1,1) == "{")
{
# Processing the start of a loop
loopb= substr($0,2) ";"
loopc="k" loop_var "++)"
loop_start = "for (k" loop_var " = 0; k" loop_var " < "
print loop_start loopb, loopc
print "{"
loop_var++
}
else
if (substr($0,1,1) == "}")
{
#    End of Loop
j = substr($0,2)+0;
for (i = 0; i < j; i++)
print "}"
}
else
if (substr($0,1,1) == "\\")
{
# Event #
print "print \"\\" $0 "\\\"";
x = getline
print "print \"\\" $0 "\\\"";
}
else
if (substr($0,1,2) == "NR")
{
# New Rule Processing
#
print "# " $0
    in_comment = 2
}
else
if (substr($0,1,2) == "ER")
{
# Existing Rule Processing
#
print "# " $0
    in_comment = 2
}
else
if ($1 == "Finished")
{
# End of script
#
{print "print \"# " $0 "\"";}
    in_comment = 2
}
else
if ($1 == "Event")
{
# A not very interesting remark
#
{print "print \"# " $0 "\"";}
    in_comment = 2
}
else
{
# Shouldn't get here
print "ERROR - " x " - " $0
}
}
next
}
}
#
# Default action; print this out. Stuff the output where it contains
# double quotes and back-slashes
#
{
i = 1
j=length
printf "print \""
for (i = 1; i <= j; i++)
{
    x = substr($0,i,1)
    if (x == "\\")
        printf "\\\\"
    else
    if (x == "\"")
        printf "\\\""
    else
        printf "%s",x
    }
        printf "\"\n"
}
# finish off
END {
print "}"
}
