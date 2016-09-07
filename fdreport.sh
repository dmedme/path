#!/bin/sh
# fdreport.sh - Report on the output from the run of the Forms Driver
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# This script expects the following input parameters
#
#  1.  - run identifier
#  2.  - (Optional) Start Time
#  3.  - (Optional) End Time
#  4.  - (Optional) Additional output identifier
#
# Report on the response times recorded in the output file
# The layout of the file is (colon separated):
#   - bundle identifier
#   - run identifier (PID)
#   - user identifier
#   - stamp sequence
#   - time stamp
#   - stamp type
#   - Further information
#
# Pick up the possible events from the A events
#
# Add details to the accumulators as the others (except for S, Z, A and F)
# are found
#
# Report by event type afterwards.
#
# This version ignores events that do not take place within the time window.
#
set -x
extra=""
if [ $# -lt 1 ]
then
    echo Provide a Report Identifier and Optionally start and end times
    echo "(in seconds since 1970)"
    exit 1
fi
PATH_AWK=${PATH_AWK:-nawk}
export PATH_AWK
pid=$1
if [ $# -lt 3 ]
then
    set -- ` $PATH_AWK '/end_time/ {print $4 " " $6}' runout$pid `
    start_time=$1
    end_time=$2
else
    start_time=$2
    end_time=$3
    if [ $# -gt 3 ]
    then
        extra=$4
    fi
fi
fdreport -b -s $start_time -e $end_time comout* | sed 's/   */	/g
s/ \([1-9][0-9]*\.[0-9]\)/	\1/g'  | tee timout$pid$extra.txt
#
#
pnam=
if [ -f /usr/adm/pacct ]
then
    pnam=/usr/adm/pacct
elif [ -f /var/adm/pacct ]
then
    pnam=/var/adm/pacct
fi
if [ -s "$pnam" ]
then
cat  ` ls -tr $pnam* ` | hisps -s $start_time -e $end_time >pacct.txt
sarprep -p ps_beg ps_end pacct.txt
mv resout.txt resout$pid$extra.txt
fi
#
# Produce the sar output
#
if [ -f sad ]
then
cat /dev/null > stats$pid$extra
for i in u b d y c w a q v m p g r k l
do
   sar -$i -f sad >> stats$pid$extra
done 
sarprep stats$pid$extra
rm sar_td.txt
mv sar_dsk.txt dsk_$pid$extra.txt
mv sar_cpu.txt cpu_$pid$extra.txt
mv sar_mem.txt mem_$pid$extra.txt
mv sar_scr.txt scr_$pid$extra.txt
mv sar_tty.txt tty_$pid$extra.txt
fi
#
# Produce the iostat output
#
if [ -f ioout ]
then
sarprep -t 30 -s $start_time -b ioout
mv ioout.txt ioout_$pid$extra.txt
mv iocpu.txt ioout_$pid$extra.txt
mv iotty.txt ioout_$pid$extra.txt
fi
exit
