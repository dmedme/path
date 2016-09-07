:
#!/bin/sh5
#!/bin/sh
# fdecho.sh	
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Handle the ECHO sub menu - called from fdscript.sh 
# Read in the required functions
# set -x
. $PATH_SOURCE/fdesel.sh
. $PATH_SOURCE/fdscrisel.sh
if [ "$PATH_OS" = "ULTRIX" ]
then
    PATH_AWK=awk
elif [ "$PATH_OS" = "AIX" ]
then
    PATH_AWK=nawk
elif [ "$PATH_OS" = "OSF" ]
then
    PATH_AWK=gawk
else
    PATH_AWK=nawk
fi
export PATH_AWK
#
while :
do
choice=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=ECHO:  Echo File Management $curr
PARENT=SCRIPT:
PROMPT=Select Menu Option and Press RETURN
SEL_YES/COMM_YES/MENU
SCROLL
ECHOCREATE:    Create New Echo files/ECHOCREATE:
ECHORUN:       Run an Echo File/ECHORUN:
ECHOEDIT:      Edit Echo Files/ECHOEDIT:
ECHOSAVE:      Save Echo Files to User Directory/ECHOSAVE:
ECHODELETE:    Delete Echo Files/ECHODELETE:
ECHOHELP:      Help/ECHOHELP:
EXIT:          Exit/EXIT:

HEAD=ECHOHELP:  ECHO File Management Help
PARENT=ECHO:
PROMPT=Press RETURN to return
SEL_YES/COMM_YES/MENU
SCROLL
ECHOCREATE:     Create an Echo file/NULL:
    Create an echo file from an existing script file with the option
    of running it interactively/NULL: 
ECHORUN:      Run an echo file interactively and produce an event report/NULL:
ECHOEDIT:     Edit echo files/NULL:
ECHOSAVE:     Save echo files/NULL:
    Select a number of echo files to save to the PATH_USER directory
    with the option of deleting the files from the echo directory/NULL:
ECHODELETE:     Delete Echo Files/NULL:
    Select a number of echo files to remove/NULL:
ECHOHELP:       Help/NULL:
    Display this message/NULL: 

EOF
`
set -- $choice
case $1 in
EXIT:)
break
;;
ECHOCREATE:)
# Select script files to create and retain echo files
#
    script_select "ECHOCREATE: Pick scripts to create echo files " "Y" ""
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
   echvar=`echo $i |sed 's/[SR]/E/'`
Ename=$PATH_HOME/echo/$echvar.ech
cps=$PATH_CPS
think=$PATH_THINK
        $PATH_AWK -f $PATH_HOME/scripts/$PATH_SCENE/f$i.awk cps=$cps think=$think</dev/null >$Ename
echo $desc >>$Ename
            get_yes_or_no "ECHOCREATE: Do you wish to run the file E$i now?"
            if [ $CHOICE = "YES:" ]
            then
$PATH_SOURCE/ptydrive -d -e\# -r  -t /dev/null 1 1 1 > 1 <$Ename
fi
    done
    else
        echo  "No script supplied\c"
        sleep 1
    fi
;;
ECHORUN:)
#
# Run the echo files and produce event report
#
script_list=`ls $PATH_HOME/echo/$PATH_SCENE/E*.ech | sed  's=.*/\\(E[^/]*\\)\\.ech$=\\1='`
    script_selecte "ECHORUN: Pick echo files to run" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
log=$PATH_HOME/scripts/$PATH_SCENE/$i 
$PATH_SOURCE/ptydrive -d -e\# -r  -t /dev/null 1 1 1 > $log <$PATH_HOME/echo/$PATH_SCENE/$i.ech
echvar=`echo $i.awk |sed 's/E/R/'`
Rname=$PATH_HOME/scripts/$PATH_SCENE/f$echvar 
                $PATH_AWK -f $PATH_SOURCE/fdconvert.awk $log >$Rname
                echo $desc >> $Rname
repvar=`echo $i.rep |sed 's/E/S/'`
rname=$PATH_HOME/data/f$repvar
echo "Echo file $repvar" `date` >$rname
$PATH_AWK -f $PATH_SOURCE/awkextract.awk $Rname | $PATH_AWK -f $PATH_SOURCE/awksort.awk | $PATH_AWK -f $PATH_SOURCE/awkprint.awk >> $rname
sleep 5
clear
pg -s $rname
rm -f $Rname  $log
    done
    else
        echo  "No echo file selected\c"
        sleep 1
    fi
;;
ECHOEDIT:)
#
# Run editor $PATH_EDITOR on echo files
#
script_list=`ls $PATH_HOME/echo/$PATH_SCENE/E*.ech | sed  's=.*/\\(E[^/]*\\)\\.ech$=\\1='`
    script_selecte "ECHORUN: Pick echo files to edit" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        $PATH_EDITOR $PATH_HOME/echo/$PATH_SCENE/$i.ech
    done
    else
        echo  "No echo file selected\c"
        sleep 1
    fi
;;
ECHODELETE:)
#
# Remove selected echo files 
#
    
script_list=`ls $PATH_HOME/echo/$PATH_SCENE/E*.ech | sed  's=.*/\\(E[^/]*\\)\\.ech$=\\1='`
    script_selecte "ECHORUN: Pick echo files to delete" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
# echo "Are you sure (Y/N)?"
#read choice
# case $choice in
# Y)
# echo
# echo "Removing echo file $i.ech! "
        rm -f $PATH_HOME/echo/$PATH_SCENE/$i.ech
# ;;
# *)
# echo "echo files not removed"
# sleep 1
# break
# ;;
# esac
done
    else
        echo  "No echo file selected\c"
        sleep 1
    fi
;;
ECHOSAVE:)
#
# Copy echo files to PATH_USER directory
#
script_list=`ls $PATH_HOME/echo/$PATH_SCENE/E*.ech | sed  's=.*/\\(E[^/]*\\)\\.ech$=\\1='`
    script_selecte "ECHOSAVE: Pick echo files to save" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        cp $PATH_HOME/echo/$PATH_SCENE/$i.ech $PATH_USER
            get_yes_or_no "ECHOSAVE: Delete the file $PATH_HOME/echo/$PATH_SCENE/$i.ech?"
            if [ $CHOICE = "YES:" ]
            then

        rm -f $PATH_HOME/echo/$PATH_SCENE/$i.ech
fi
 done

    else
        echo  "No echo file supplied\c"
        sleep 1
    fi
;;
esac
done
