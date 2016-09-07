# fdharninter.sh
# ************************************************************************
# Functions that run ptydrive interactively.
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# harninter
#
# Function for capturing the commented bones of a script session
# Parameters : 1 - Name of log file
# 
# ptydrive switches
# -p is save the feed to the pty;
# -d is output the stuff from the pty;
# -b is single step
# -s is see through mode (don't interpret terminal independent stuff)
# -e is allow escaped comments in the output stream; these will not
#    quite be in the right place
# -t is log in terminal-independent mode
# -r is the redraw string
# -f is the force event plus timeout
# Whatever you do, don't hit ^Z with BSD csh-style job control!
#
# stty options differ markedly from UNIX to UNIX. Therefore, these are
# managed by ptydrive, with conditional compilation.
#
harninter () {
logfile="$1"
if [ -z "$logfile" ]
then
    return 1
fi
if [ -z "$PATH_COMMAND" ]
then
    if [ -z "$PATH_INIT" ]
    then
         ptydrive -f "$PATH_FORCE" -u `tty` -d -s -e \# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile
    else
        ptydrive -f "$PATH_FORCE" -u `tty` -i "$PATH_INIT" -d -s -e \# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile
    fi
else
    if [ -z "$PATH_INIT" ]
    then
    ptydrive -f "$PATH_FORCE" -u `tty` -x "$PATH_COMMAND" -d -s -e \# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile
    else
ptydrive -f "$PATH_FORCE" -u `tty` -i "$PATH_INIT" -x "$PATH_COMMAND" -d -s -e \# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile
    fi
fi
cp $logfile fred
return 0
}
#
# ***********************************************************************
# semiauto
#
# Function for verifying a script
# Parameters : 1 - Name of log file
#              2 - Name of script file
#
semiauto () {
logfile="$1"
if [ -z "$logfile" ]
then
    return 1
fi
scriptfile="$2"
if [ -z "$scriptfile" ]
then
    return 1
fi
if [ -z "$PATH_COMMAND" ]
then
    if [ -z "$PATH_INIT" ]
    then
ptydrive $PATH_EXTRA -u `tty` -d -e\# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile <$scriptfile
    else
ptydrive $PATH_EXTRA -u `tty` -d -i "$PATH_INIT" -e\# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile <$scriptfile
    fi
else
    if [ -z "$PATH_INIT" ]
    then
ptydrive $PATH_EXTRA -u `tty` -x "$PATH_COMMAND" -d -e\# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile <$scriptfile
    else
ptydrive $PATH_EXTRA -u `tty` -x "$PATH_COMMAND" -i "$PATH_INIT" -d -e\# -r "$PATH_REDRAW" -t /dev/null 1 1 1 > $logfile <$scriptfile
    fi
fi
cp $logfile fred
return
}
