#!/bin/sh
#!/bin/sh5
# fdscript.sh
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Handle the script facilities. Called from fdpathat.sh
# Read in the required functions
# set -x
. $PATH_SOURCE/fdvars.sh
. $PATH_SOURCE/fdscrisel.sh
. $PATH_SOURCE/fdconvert.sh
. $PATH_SOURCE/fdharninter.sh
. $PATH_SOURCE/fdchoice.sh
. $PATH_SOURCE/fdevent.sh
while :
do
if [ ! -z "$PATH_IG" ]
then
    curr="($PATH_IG"
else
    curr="(None"
fi
if [ ! -z "$PATH_SCENE" ]
then
    curr="$curr/$PATH_SCENE)"
else
    curr="$curr/None)"
fi
if [ -z "$PATH_INIT" ]
then
    id=
    timeout=
    look=
else
    set `echo $PATH_INIT | sed 's/::.*$//
s/:/ /g'`
    if [ $# -lt 3 ]
    then
        id=
        timeout=
        look=
    else
        id=$1
        shift
        timeout=$1
        shift
        look="$*"
    fi
fi
choice=`natmenu 3<<EOF 4>&1 </dev/tty >/dev/tty
HEAD=UNISCRIPT:  Test Script Management $curr
PROMPT=Select Menu Option and Press RETURN
SEL_YES/COMM_YES/MENU
SCROLL
UNICREATE:     Create a New Test Script/UNICREATE:
UNIRUN:        Interactively rerun a Test Script/UNIRERUN:
UNIEDIT:       Edit Test Scripts/UNIEDIT:
UNISHELL:      Escape to Shell/UNISHELL:
UNIREVIEW:     Review Script Event Reports/UNIREVIEW:
UNISAVE:       Save Scripts to User Directory/UNISAVE:
UNIDELETE:     Delete Test Scripts/UNIDELETE:
UNICOMMAND:    Change the Initial Startup Command/UNICOMMAND:
UNIEVENT:      Declare an Initial Event/UNIEVENT:
UNIECHO:       Manage Echo Files/UNIECHO:
UNIHELP:       Help/UNIHELP:
EXIT:          Exit/EXIT:

HEAD=UNIHELP:  UNISCRIPT Management Help
PARENT=UNISCRIPT:
PROMPT=Press RETURN to return
SEL_YES/COMM_YES/MENU
SCROLL
UNICREATE:     Create a New Test Script/NULL:
    Create a brand new test script with keystroke capture/NULL:
UNIRUN:        Rerun a Test Script/NULL:
    Run a test script under conditions which allow you to revise/NULL:
    it interactively/NULL:
UNIEDIT:       Edit Test Script Programs/NULL:
    Select a number of test scripts to edit/NULL:
UNISHELL:      Temporary escape to The Shell/NULL: 
UNIREVIEW:     Review Event Reports from a regenerated Script File/NULL:
UNISAVE:       Save scripts to your own User Directory 
UNIDELETE:     Delete Test Script Programs/NULL:
    Select a number of test scripts to purge/NULL:
UNICOMMAND:    Declare an initial Script Command/NULL:
    If script execution is to start with something other than a shell,/NULL:
    declare this fact here./NULL:
UNIEVENT:      Declare an Initial Event/NULL:
    If you want to search for something brought up by the initial command,/NULL:
    declare this fact here./NULL:
UNIECHO:       Select ECHO file management sub menu/NULL:
UNIHELP:       Help/NULL:
    Display this message/NULL: 

HEAD=UNICREATE: Create a new System Test Script
PARENT=UNISCRIPT:
PROMPT=Give your script a mnemonic name and Press RETURN
COMMAND=UNICREATE:
SEL_NO/COMM_NO/SYSTEM
SCROLL
Name :
Description :

HEAD=UNICOMMAND: Declare an initial Script Command
PARENT=UNISCRIPT:
PROMPT=Enter your initial command (NOT a shell) and Press RETURN
COMMAND=UNICOMMAND:
SEL_NO/COMM_NO/SYSTEM
SCROLL
Command :/$PATH_COMMAND

HEAD=UNIEVENT: Declare an Initial Event $PATH_INIT
PROMPT=Enter your initial event and Press RETURN
COMMAND=UNIEVENT:
SEL_NO/COMM_NO/SYSTEM
SCROLL
Event ID (2 chars)           :/$id
Timeout  (Positive Integer)  :/$timeout
Look For (Regular Expression):/$look

EOF
`
set -- $choice
case $1 in
EXIT:)
break
;;
UNICREATE:)
#
# Process a script file creation
# - Check that a name has been supplied
# - run harninter on it
#
    shift
    if [ ! -z "$1" ]
    then
        test=$PATH_HOME/scripts/$PATH_SCENE/$1
        shift
        desc=$@
        harninter $test
        fdconvert "S" $test
        echo "# " $desc >> $PATH_HOME/scripts/$PATH_SCENE/fS$NAME.awk
        rm -f $test
    else
        echo  "No script name supplied\c"
        sleep 1
    fi
;;
UNIRERUN:)
#
# Rerun a previously created script in such a way that failures cause the
# transfer of control to the user.
# - Offer the user a list of scripts to pick
# - Process them one at a time
#   -  Run the script (semiauto)
#   -  Ask the user if regeneration is required.
#   -  If yes, put it back where it came from. The processing would
#      be the same as at the end of harninter.sh
#
script_list=`ls -1 $PATH_HOME/scripts/$PATH_SCENE/fR*.awk $PATH_HOME/scripts/$PATH_SCENE/fS*.awk 2>/dev/null | sed  's=.*/f\\([^/]*\\)\\.awk$=\\1='`
    
    script_select "UNIRUN: Pick scripts to Rerun" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        fname=$PATH_HOME/scripts/$PATH_SCENE/f$i.awk
        j=`echo $i | sed 's/.//'`
        Rname=$PATH_HOME/scripts/$PATH_SCENE/fR$j.awk
        desc=`tail -1 $fname`
        PATH_COMMAND=`sed -n '1s/#//
1p
1q' < $fname`
        export PATH_COMMAND
        PATH_INIT=`sed -n '2s/#//
2p
2q' < $fname`
        export PATH_INIT
        if fdsetup.sh $fname $pid 1 $PATH_CPS $PATH_THINK 1 1 1 1 1 1
        then
semiauto $PATH_HOME/scripts/$PATH_SCENE/$j echo$pid.1.0
            get_yes_or_no "UNIREGENERATE: Do you wish to regenerate?"
            if [ $CHOICE = "YES:" ]
            then
                fdconvert "R" $PATH_HOME/scripts/$PATH_SCENE/$j 
                echo $desc >> $Rname
rname=$PATH_HOME/data/f$i.rep
rm -f $rname
echo "Script $Rname" `date` >$rname
$PATH_AWK -f $PATH_SOURCE/awkextract.awk $Rname | $PATH_AWK -f $PATH_SOURCE/awksort.awk | $PATH_AWK -f $PATH_SOURCE/awkprint.awk >> $rname
sleep 5
echo  $desc>>$rname
clear
pg -s $rname
            fi
rm -f $PATH_HOME/scripts/$PATH_SCENE/$j 
Ename=$PATH_HOME/echo/E$j.ech 
mv -f echo$pid.1.0 $Ename
        else
            echo  "Script errors - must be edited\c"
            sleep 5
        fi
    done
    else
        echo  "No script supplied\c"
        sleep 1
    fi
;;
UNIREVIEW:)
#
# Run pg on event reports 
#
script_list=`ls -1 $PATH_HOME/data/f*.rep 2>/dev/null | sed  's=.*/f\\([^/]*\\)\\.rep$=\\1='`
    script_selecta "UNIREVIEW: Pick Event Reports to Review" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
   clear
        pg $PATH_HOME/data/f$i.rep
            get_yes_or_no "PRINT: Do you wish to print $i?"
            if [ $CHOICE = "YES:" ]
            then
        lpr $PATH_HOME/data/f$i.rep
fi
    done
    else
        echo  "No Report selected\c"
        sleep 1
    fi
;;
UNIEDIT:)
#
# Run editor $PATH_EDITOR on script files
#
script_list=`ls -1 $PATH_HOME/scripts/$PATH_SCENE/fS*.awk $PATH_HOME/scripts/$PATH_SCENE/fR*.awk 2>/dev/null | sed  's=.*/f\\([^/]*\\)\\.awk$=\\1='`
    script_select "UNIEDIT: Pick scripts to edit" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        $PATH_EDITOR $PATH_HOME/scripts/$PATH_SCENE/f$i.awk
    done
    else
        echo  "No script supplied\c"
        sleep 1
    fi
;;
UNISHELL:)
clear
echo "Insert Shell Command and Press Return - Type exit to return to PATH" 
csh -
;;
UNIDELETE:)
#
# Delete script files
#
script_list=`ls -1 $PATH_HOME/scripts/$PATH_SCENE/fS*.awk $PATH_HOME/scripts/$PATH_SCENE/fR*.awk 2>/dev/null | sed 's=.*/f\\([^/]*\\)\\.awk$=\\1='`
    
    script_select "DELETE: Pick scripts to delete" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        rm -f $PATH_HOME/scripts/$PATH_SCENE/f$i.awk
    done
    else
        echo  "No script supplied\c"
        sleep 1
    fi
;;
UNISAVE:)
#
# Move script files to User directory
#
script_list=`ls -1 $PATH_HOME/scripts/$PATH_SCENE/fS*.awk $PATH_HOME/scripts/$PATH_SCENE/fR*.awk 2>/dev/null | sed  's=.*/f\\([^/]*\\)\\.awk$=\\1='`
    
    script_select "UNISAVE: Pick scripts to save" "" "$script_list"
    if [ ! -z "$SCRIPT_LIST" ]
    then
    for i in $SCRIPT_LIST
    do
        cp $PATH_HOME/scripts/$PATH_SCENE/f$i.awk $PATH_USER
            get_yes_or_no "UNISAVE: Delete the script $PATH_HOME/scripts/$PATH_SCENE/f$i.awk?"
            if [ $CHOICE = "YES:" ]
            then
        rm -f $PATH_HOME/scripts/$PATH_SCENE/f$i.awk
fi
    done
    else
        echo  "No script supplied\c"
        sleep 1
    fi
;;
UNICOMMAND:)
    shift
    PATH_COMMAND=$@
    export PATH_COMMAND
;;
UNIEVENT:)
    shift
    if [ $# -ge 3 ]
    then
        args=`echo $* | sed 's/:/./g'`
        set -- $args
        id=$1
        shift
        timeout=$1
        shift
        look=$1
        shift
        PATH_INIT="$id:$timeout:$look::Initial Event"
    else
        PATH_INIT=""
    fi
    export PATH_INIT
;;
UNIECHO:)
fdecho.sh
;;
esac
done
exit
