:
#!/bin/sh
# fdmenu.sh - list of options for forms driver benchmark
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#TAILOR ensure using correct shell
#
# set -x
# TERM=dme386
# export TERM
# TERMINFO=/home/opt/e2/path
# export TERMINFO
#. fdvars.sh
pid=$$
#
# not used in this case, but preserved in order to satisfy argument
# count requirements
#
case $PATH in
/opt/e2/path)
   :
;;
*)
   PATH=/opt/e2/path:$PATH export PATH
;;
esac
now=`date`
usr_id=trial/trial
export usr_id
clear
while [ : ]
do
cat << EOF
                       Forms Driver Facilities 
  ___________________________________________________________________

   1. HELP! How to use fdmenu.sh    2. Edit files
   3. Define a scenario             4. List and display files
   5. Execute a scenario            6. Display test results
   7. Delete or dump test result    8. Not in use
         to a sub-directory
   9. Not in use                    10. Exit 
  ____________________________________________________________________
EOF
echo "Enter your choice \c"
read choice
case $choice in
1)
echo --------------------------------------------------------------------
        echo 'HELP - How to use fdmenu.sh ' 
echo --------------------------------------------------------------------
     echo '1)  Displays this screen'
     echo '2)  Edit any file in current directory'
     echo '3)  Define a scenario by creating bundles.  Each bundle consists'
     echo '    a script to which parameters will be applied.  These parameters'
     echo '    control the number of users, number of transactions, etc.'
     echo '4)  List and display files in current directory'
     echo '5)  Execute a scenario using ptydrive'
     echo '6)  Display test results for scenario'
     echo '7)  Delete test results or move them to a sub-directory'
     echo '     test<pid>'
     echo '8)  Available for tailored use'
     echo '9)  Available for tailored use'
     echo '10) Exit from fdmenu.sh'
echo --------------------------------------------------------------------
echo "press return to display main menu \c"
read choice
;;
2)
# Edit scripts
while [ : ]
do
echo --------------------------------------------------------------------
echo "Scripts available in directory:"
ls -C *awk
#TAILOR Line above asssumes all scripts have .awk extension
echo " Input script to edit or press return \c"
read script
if [ -z "$script" ]
then
break
fi
vi $script
#TAILOR set above line to call your standard (or desired) editor
done
;;
3)
pid=`expr $pid + 1`
echo "Set up the forms driver; defaults are in square brackets thus[10]"
echo
echo "There must be three junk lines before the transactions" > runout$pid
echo "Run Parameters" >> runout$pid
echo "==============" >> runout$pid
    bundle=1
         echo "Accepting Details for Bundle Number " $bundle
         while :
         do
         echo "Enter the transaction for this bundle; return to quit the loop."
         echo "Scripts available in directory:"
echo --------------------------------------------------------------------
         ls -C *awk
    read tran
    if [ -z "$tran" ]
    then
        break 
    fi
    echo "Enter the number of characters per second typing rate [10]:\c"
    read cps
    if [ -z "$cps" ]
    then
        cps=10
 #TAILOR change default
    fi
    echo "Enter the operator's think time (between commits) [10]:\c"
    read think
    if [ -z "$think" ]
    then
        think=10
 #TAILOR change default
    fi
    echo "Enter the number of simulated users [10]:\c"
    read nusers
    if [ -z "$nusers" ]
    then
        nusers=10
 #TAILOR change default
    fi
    echo "Enter the number of transactions each will do [100]:\c"
    read ntrans
    if [ -z "$ntrans" ]
    then
        ntrans=100
 #TAILOR change default
    fi
    echo The runtype is set to m to indicate multi-user
    echo It could however be used for other purposes if desired

    echo "Enter the runtype value [m]:\c"
    read runtype
    if [ -z "$runtype" ]
    then
        runtype="m"
 #TAILOR change default
    fi
    echo "Enter para_1, any parameter for use within the script []:\c"
    read para_1
    if [ -z "$para_1" ]
    then
        para_1=""
    fi
    echo "Enter para_2, any parameter for use within the script []:\c"
    read para_2
    if [ -z "$para_2" ]
    then
        para_2=""
    fi
    echo "If happy with above press return, if not type n and return \c"
    read resp
    if [ -z "$resp" ]
    then
    fdsetup.sh $tran $pid $bundle $cps $think $nusers $ntrans $usr_id $runtype $para_1 $para_2 &
     echo $pid : users $nusers $tran $ntrans think $think cps $cps runtype $runtype para_1 $para_1 para_2 $para_2 >> runout$pid
     bundle=`expr $bundle + 1`
     fi
   done
echo Please wait for all the setups to finish
#
# Notify user and wait for acknowledgement
wait
echo Rerun this scenario by specifying pid = $pid
;;
4)
echo 'Input file extension or string in filename to display,'
echo '* = all, echo = all echo files, etc.'
echo 'dump = all dump files, etc \c'
while [ : ]
do
read choice
if [ -z "$choice" ]
then
break
fi
echo ------------------------------------
ls -C *$choice* 
#TAILOR ensure correct ls command options fo your system
echo ------------------------------------
echo "To display a file via pg type the filename, or press return to exit"
read choice
if [ -z "$choice" ]
then
break
fi
pg $choice
done
;;
5)
echo "Ready to run the Forms Driver script."
echo "-------------------------------------------------------------"
echo "Available echo files:"
ls -C echo*
echo "-------------------------------------------------------------"
echo "Input the pid to rerun \c"
read piid
while [ : ]
do
echo "-------------------------------------------------------------"
echo "Input number of seconds before termination \c"
read seccnt
if [ -n "$seccnt" ]
then
break
fi
done
#start processing
#
echo "Do you want to run sar? \c"
read sar_run
case $sar_run in
Y|y)
rm -f sad$piid
/usr/lib/sa/sadc 10 1000 sad$piid &
sad_pid=$!
;;
*)
sad_pid=
;;
esac
globusers=`awk < runout$piid '/:/ && !/end_time/ { print $4}'`
set -- $globusers
bundle=$#
bundle=` expr $bundle + 1 `
j=1
echo "-------------------------------------------------------------"
echo "Enter -d option for ptydrive"
echo "Causes the driver to log output from the driven application"
echo "to standard error.  This option will increase the processing"
echo "load.  Type -d and press return: \c"
read opt
echo "-------------------------------------------------------------"
echo "Starting up the ptydrives - please wait"
echo "-------------------------------------------------------------"
while [ $j -lt $bundle ]
do
i=0
set -- $globusers
nusers=$1
shift
globusers="$*" 
while [ $i -lt $nusers ]
do
outfile=log$piid.$j.$i
infile=echo$piid.$j.$i
dumpfile=dump$piid.$j.$i
#
# Start the pty driver
#
ptydrive $opt < $infile $outfile $piid $j $i >$dumpfile 2>&1 &
# stagger the start up
#
 i=`expr $i + 1`
done
 j=`expr $j + 1`
done
start_time=`tosecs`
sleep 5 
echo "----------------------------`date`-------------------"
if [ -z "$opt" ]
then
ps -adef | egrep "ptydrive log$piid" | egrep -v egrep
else
ps -adef | egrep "ptydrive $opt log$piid" | egrep -v egrep
fi
echo "---------------------------------------------------------------------------"
echo 'All streams now running; please wait'
#
while [ $seccnt -gt 0 ]
do
echo "going to sleep for duration   zzzzzzzzzz!"
sleep $seccnt
echo "----------------------------`date`-------------------"
if [ -z "$opt" ]
then
ps -adef | egrep "ptydrive log$piid" | egrep -v egrep
else
ps -adef | egrep "ptydrive $opt log$piid" | egrep -v egrep
fi
echo "---------------------------------------------------------------------------"
echo "test time has runout, Would you like some more?"
while [ : ]
do
echo "Enter number of further seconds (or 0) and press return \c"
read seccnt
if [ -n "$seccnt" ]
then
break
fi
done
done
#
# kill off still-running demo processes
#
# Stagger the death so that the Accounting Records do get written out.
# the search string includes "log$piid" to ensure that only relevant
# ptydrives are killed
ps -ade | egrep "ptydrive.*log$piid" | awk '!/grep/ {print "kill -15 " $1 " ; sleep 1;"}' | sh
if [ $sad_pid != "" ]
then
kill -9 $sad_pid
fi
wait
end_time=`tosecs`
echo `expr \( $end_time - $start_time \)` > elapsed$piid
echo $piid : start_time $start_time end_time $end_time elapsed `expr \( $end_time - $start_time \)` >> runout$piid
# kill -9 $sad_piid 
# merge the output files
cat log${piid}* | sort -n -t: > comout$piid
rm -f log${piid}*
#
#fdreport.sh $piid $start_time $end_time
fdreport.sh $piid $start_time $end_time > /dev/null 2>&1  
;;
6)
echo "Display test results"
echo ------------------------------------
echo Available tests
echo
ls -C comout*
echo "Type pid number \c"
read piid
if [ -z $piid ]
then
break
fi
cat timout$piid resout$piid runout$piid stats$piid > cat$piid
#NIGEL needs extending
pg cat$piid
# rm cat$piid
;;
7)
echo "Delete or dump test results to sub-directory"
echo --------------------------------------------------------------------
echo Available tests
echo
ls -C comout*
echo "Type pid number \c"
read piid
if [ -z "$piid" ]
then
break
fi
while [ : ]
do
echo "D=dump to sub-directory   erase = delete"
echo "Type D or erase \c" 
read choice
if [ $choice = "D" ]
then
mkdir test$piid
mv *$piid* test$piid
#TAILOR change directory name for storing test results
echo "Test data for $piid now in directory test$piid"     
echo --------------------------------------------------------------------
break
fi
if [ $choice = "erase" ]
then
rm *$piid*
echo "test results for $piid erased"
break
fi
done
;;
8)
#TAILOR Option available for your own use
;;
9)
#TAILOR Option available for your own use
;;
10)
# Exit to o/s
echo "Session started - $now"
echo "Session ending  - `date`"
break
;;
*)
echo "Invalid option; try again"
;;
esac
done
exit
