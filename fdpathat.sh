:
#!/bin/sh5
#!/bin/sh
# fdpathat.sh - Menu of options for path System Testing.
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# ***************************************************************************
# Parameters : None
#
# The directory structure is as follows
#
# The base for the tests is a directory, PATH_HOME.
#
# Below this are six sub-directories:
# - scripts
#   -  containing the generator programs
# - rules
#   -  containing the information for field value generation
#      that can automatically be included in the scripts
# - scenes
#   -  for scenario definitions
# - igs
#   -  for integration groups
# - data
#   -  where dump files end up.
# - echo
#   -  where echo files end up
#
# Optional se directory for sizing
#   A multi-user test canbe executed from any directory.
#   However tradition suggests an se directory should be
#   created for this purpose
#
# There are Integration Groups, Scenarios, and Scripts.
# - Integration Groups live in the igs directory, and are described
#   in makefiles
# - Scenario test runs are identified by the first lines of files named
#   runout$pid, and live in the data sub-directory
# - Scripts live in the scripts directory, are named as fd$name.awk,
#   and are described by the tail of the file
#
# Activity is related to nested scopes; there is a current scenario
# and integration group context.
#
# set -x
PATH_SOURCE=${PATH_SOURCE:-/e2soft/path}
export PATH_SOURCE
if [ -f ${HOME}/fdvars.sh ]
then
    . ${HOME}/fdvars.sh
fi
if [ ! -f $PATH_SOURCE/fdvars.sh ]
then
    echo "No PATH source files in designated PATH SOURCE directory"
    exit 1
fi
. $PATH_SOURCE/fdvars.sh
cd $PATH_HOME
stty intr \ susp \ quit \
trap "" 2 3
#
# ***************************************************************************
# Main program - process user requests until exit
#
while :
do
NOW=`date`
choice=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=MAIN:  Test Management at $NOW
PROMPT=Select Menu Option and Press RETURN
SEL_YES/COMM_YES/MENU
SCROLL
UNISCRIPT:    Manage Test Scripts/SCRIPT:
SCENARIO:     Manage Test Scenarios/SCENARIO:
INTER:        Manage Integration Groups/INTER:
HELP:         Help/HELP:
EXIT:         Exit/EXIT:

HEAD=HELP:  Test Management Help
PARENT=MAIN:
PROMPT=Press RETURN to return
SEL_YES/COMM_YES/MENU
SCROLL
UNISCRIPT:  Maintain Test Scripts/NULL:
    Anything to do with Tests Scripts individually/NULL:
SCENARIO: Manage Test Scenarios/NULL:
    Anything to do with Execution of Scenarios/NULL:
INTER:    Manage Integration Groups/NULL:
    Integration groups are self-contained collections of scenarios/NULL:
    with commands to restore the database to a known state at the/NULL:
    beginning./NULL:
HELP:     Help/NULL:
    Display this message/NULL: 

EOF
`
set -- $choice
case $1 in
EXIT:)
# clean.sh
    exit
;;
SCRIPT:)
    fdscript.sh
;;

SCENARIO:)
    fdsystem.sh
;;
INTER:)
    fdinter.sh
;;
*)
echo $choice
echo  "Logic Error: Invalid option"
exit
;;
esac
done
exit 
