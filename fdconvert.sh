# ***************************************************************************
# fdconvert.sh
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to generate a generator program from a script
# Parameters : 1 - Prefix for the file name
#              2 - Name of the log file to generate from
#
fdconvert () {
    prefix="$1"
    logfile="$2"
    if [ -z "$logfile" ]
    then
        return 1
    fi
    NAME=`echo $logfile | sed 's=.*/=='`
    export NAME
    target=$PATH_HOME/scripts/$PATH_SCENE/f$prefix$NAME.awk
    rm -f $target
    (
      echo "#$PATH_COMMAND"
      echo "#$PATH_INIT"
    ) | cat - $PATH_SOURCE/preamble > $target
if [ "$PATH_OS" = ULTRIX ]
then
     awk -f $PATH_SOURCE/fdconvert.awk path_init="$PATH_INIT" $logfile |
        sed -f $PATH_SOURCE/fdconvert.sed  >> $target
else
# SCO SUNOS
    $PATH_AWK -f $PATH_SOURCE/fdconvert.awk path_init="$PATH_INIT" $logfile >> $target
fi
    $PATH_EDITOR $target
    return 0
}
