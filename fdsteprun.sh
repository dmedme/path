#!/bin/sh
#
# fdsteprun.sh - run a sizing
# 
do_phase() {
this_pid=$1
globusers=`nawk < runout$this_pid '!/end_time/ { if(NR>3) print $1}'`
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
outfile=log$this_pid.$j.$i
infile=echo$this_pid.$j.$i
dumpfile=dump$this_pid.$j.$i
#
# Start the pty driver
#
if [ $i -lt 2 ]
then
ptydrive -d -p < $infile $outfile $this_pid $j $i >$dumpfile.in 2>$dumpfile.out &
else
ptydrive < $infile $outfile $this_pid $j $i >$dumpfile 2>&1 &
fi
# stagger the start up
sleep 3
#
 i=`expr $i + 1`
done
 j=`expr $j + 1`
done
return
}
# **********************************************************
# Main Program start here
#
TERM=vt220
export TERM
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
# ***********************************************************************
# Start performance monitors
#
#/usr/lib/acct/accton
#cat /dev/null >/usr/adm/pacct
#/usr/lib/acct/accton /usr/adm/pacct
# netstat -i 60 >netout$piid &
# net_piid=$!
save_dir=save.$piid.$$
mkdir $save_dir
/usr/lib/sa/sadc 30 2000 $save_dir/sad &
sad_piid=$!
vmstat 30 >$save_dir/vmout &
vm_piid=$!
iostat 30 >$save_dir/ioout &
io_piid=$!
ps -ef >$save_dir/ps_beg
#rsh -n db_server /var/e2/cspath/mon.sh &
#rsh_piid=$!
#start processing
#
cat /dev/null >elapsed$piid
for phase in "" _1 _2 _3 _4 _5 _6
do
    if [ -f runout$piid$phase ]
    then 
        do_phase $piid$phase
        tosecs >>elapsed$piid
        sleep $seconds
        tosecs >>elapsed$piid
    fi
done
kill -9 $sad_piid $io_piid $vm_piid
ps -e | awk '/ptydrive/ { print  $1 }' | xargs kill -15
wait
#
# merge the output files
#
if cat log${piid}* | sort -n -t: > $save_dir/comout
then
    rm -f log${piid}*
fi
#
# Produce the report pack
#
cp runout$piid* dump$piid*.in dump$piid*.out elapsed$piid $save_dir
cd $save_dir
ps -ef >ps_end
for phase in "" _1 _2 _3 _4 _5
do
    read start_time || break
    read end_time || break
    fdreport.sh $piid $start_time $end_time $phase
    if [ -f sad ]
    then
        mv sad sad.used
    fi
    if [ -f ioout ]
    then
        mv ioout ioout.used
    fi
    if [ -f vmout ]
    then
        mv vmout vmout.used
    fi
done <elapsed$piid
exit
