#!/bin/sh
# fdrun.sh - run a sizing
#
trap '' 1
if [ $# -lt 1 ]
then
    echo Provide a runout file id
    exit 1
fi
piid=$1
if [ ! -f runout$piid ]
then
    echo Provide a valid runout file id
    exit 1
fi
if [ $# -lt 2 ]
then
    seconds=900
else
    seconds=$2
fi
set -x
#
# the next 2 lines start sar
# input any other code to initiate performance monitors
# here !!!!!
#
/usr/lib/acct/accton
cat /dev/null >/usr/adm/pacct
/usr/lib/acct/accton /usr/adm/pacct
#/usr/lib/sa/sadc 30 2000 sad$piid &
#sad_piid=$!
#start processing
#
globusers=`nawk < runout$piid '!/end_time/ { if(NR>3) print $1}'`
set -- $globusers
bundle=$#
bundle=` expr $bundle + 1 `
j=1
echo "-------------------------------------------------------------"
start_time=`tosecs`
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
if [ $i -lt 10 ]
then
ptydrive -d -p < $infile $outfile $piid $j $i >$dumpfile.in 2>$dumpfile.out &
else
rerun.sh $outfile $piid $j $i $infile >$dumpfile 2>&1 &
fi
# stagger the start up
sleep 3
#
 i=`expr $i + 1`
done
 j=`expr $j + 1`
done
sleep $seconds
#kill -9 $sad_piid 
ps -e | nawk '/rerun.sh/ { print  $1 }' | xargs kill -9
ps -e | nawk '/ptydrive/ || /rerun.sh/ { print  $1 }' | xargs kill -15
wait
end_time=`tosecs`
echo `expr \( $end_time - $start_time \)` > elapsed$piid
echo $piid : start_time $start_time end_time $end_time elapsed `expr \( $end_time - $start_time \)` >> runout$piid
# merge the output files
if cat log${piid}* | sort -n -t: > comout$piid
then
    rm -f log${piid}*
fi
#
fdreport.sh $piid $start_time $end_time > /dev/null 2>&1  
mkdir save.$piid.$$
mv resout$piid timout$piid stats$piid comout$piid save.$piid.$$
cp runout$piid save.$piid.$$
#mv sad$piid sad$piid.used
