#!/bin/sh
# fdmenu; list of options for forms driver benchmark
# @(#) $Name$ $Id$
# Copyright (c) 1993 E2 Systems
# Essentially provided to give a high level index of what is there;
# the file Makefile identifies most of the components
# if [ -f ${HOME}/fdvars.sh ]
# then
# . ${HOME}/fdvars.sh
# fi
if [ $# -lt 1 ]
then
    echo Enter runout file ID
    exit 1
fi
pid=$1
if [ ! -f runout$pid ] 
then
echo "Create a runout file as does not exist; defaults are in square brackets thus[10]"
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
    echo "Enter para_1, any parameter for use within the script [1]:\c"
    read para_1
    if [ -z "$para_1" ]
    then
        para_1="1"
    fi
    echo "Enter para_2, any parameter for use within the script [1]:\c"
    read para_2
    if [ -z "$para_2" ]
    then
        para_2="1"
    fi
    echo "Enter para_3, any parameter for use within the script [1]:\c"
    read para_3
    if [ -z "$para_3" ]
    then
        para_3="1"
    fi
    echo "If happy with above press return, if not type n and return \c"
    read resp
    if [ -z "$resp" ]
    then
     echo $nusers $tran $ntrans $think $cps $runtype $para_1 $para_2 $para_3 >> runout$pid
     bundle=`expr $bundle + 1`
     fi
   done
fi
(
#
# Skip the first three lines
#
read i
read i
read i
bundle=1
while :
do
    read nusers tran ntrans think cps runtype para_1 para_2 para_3
    if [ "$ntrans" = "start_time" -o "$ntrans" = "" ]
    then
        break
    fi
# Notify user and wait for acknowledgement
echo  Generating script $tran for $nusers users $ntrans transactions
    fdsetup.sh $tran $pid $bundle $cps $think $nusers $ntrans $runtype $para_1 $para_2 $para_3
     bundle=`expr $bundle + 1`
done
) < runout$pid
#
wait
echo Echo regeneration complete for $pid
