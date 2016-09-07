:
#!/bin/sh5
# fdcleanup.sh - kill off still-running demo processes
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
if [ "$PATH_OS" = ULTRIX ]
then
ps -x| egrep "ptydrive" | grep -v egrep | awk '{print "kill  -15 " $1}' | sh
else
# System V behaviour
# SCO
ps -ef |  awk '/t[i]gger1 / {print $2}' | xargs kill -15
fi
