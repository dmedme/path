# fdvars.sh - Global variables for PATH
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# ***************************************************************************
#
# Function to allow editing with edt whilst
# preserving the inode number
#
edt_wrap () {
for i in $*
do
    rm -f $i.bak
    edt $i
    if [ -f $i.bak ]
    then
        cp $i.bak $i.sav
        cp $i $i.bak
        mv $i.bak $i
    fi
done
return
}
PATH_TERM=`tty`
export PATH_TERM
PATH_IG=${PATH_IG:-}
PATH_SCENE=${PATH_SCENE:-}
# Establish directory where PATH is run
PATH_HOME=${PATH_HOME:-/u3/e2/2000}
# O/S type for establishing executables directory (PATH_SOURCE) and text editor
# (PATH_EDITOR)
PATH_OS=${PATH_OS:-AIX}
PATH_SOURCE=${PATH_SOURCE:-/u3/e2/2000/path}
PATH_EDITOR=${PATH_EDITOR:-vi}
# path and name of ./rules directory
PATH_RULE_BASE=$PATH_HOME/rules
# Terminal type
#TERM=${TERM:-at386}
#TERM=${TERM:-at386}
#TERM=${TERM:-dme300}
pid=$$
export TERM PATH_RULE_BASE PATH_HOME PATH_EDITOR PATH_IG PATH_SCENE PATH_SOURCE usr_id pid
# path and name of directory for saved scripts
PATH_USER=${PATH_USER:-}
# Think/Wait time
PATH_THINK=${PATH_THINK:-5}
# Characters per second typing speed
PATH_CPS=${PATH_CPS:-3}
# Application Redraw String
PATH_AWK=nawk
PATH_REDRAW=${PATH_REDRAW:-}
# Initial Command
PATH_COMMAND=${PATH_COMMAND:-}
# Initial Event
PATH_INIT=${PATH_INIT:-}
PATH_DIALOGUE=${PATH_DIALOGUE:-"MANAGEMENT:"}
export PATH_THINK PATH_CPS PATH_OS PATH_REDRAW PATH_USER PATH_COMMAND PATH_INIT PATH_DIALOGUE PATH_AWK
case $PATH in
*$PATH_SOURCE*)
     ;;
*)
#    ORACLE_HOME=/u01/oracle/product/7.2.3
#    ORACLE_SID=cfx7
    PATH=$PATH_SOURCE:$PATH_SOURCE/../e2common:$PATH_SOURCE/../perfdb:$PATH
    export PATH
    ;;
esac
