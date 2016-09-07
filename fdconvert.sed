#
# @(#)fdconvert.sed	1.1 93/05/20 22:33:54 93/05/20 22:33:54
# Copyright (c) E2 Systems Limited 1993
#
# fdconvert.sed - This program gets round the limitations in the
# Ultrix 4.2 terminfo capability by joining up the missing function
# keys as single elements. Beware. Implies that all the missing function
# keys are of the form  ^[.*~
#
#  Look for an escape
//{
# change the delimiter if present to the 'all one key' delimiter
s/'/`/g
tsome
# if no delimiter, separate the escape from everything else on the line
s//' `/g
:some
# Have we got the tilde?
/~/{
# Similar logic to that for the escape
s/'".*"'//
tclear
:clear
s/~'/~`/g
tthere
s/~/~` '/g
:there
#
# Finished with this one
bend
}
#
# If we come here, we haven't got the tilde. Get another line and try again
N
bsome
:end
}

