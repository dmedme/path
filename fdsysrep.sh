:
# Report by event type afterwards.
#!/bin/sh5
# fdsysrep; Report on the output from the run of the System Test Driver
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited, 1993
#
# This script expects the following input parameters
#
#  1.  - run identifier
#
# Report on the response times recorded in the output file
# The layout of the file is (colon separated):
#   - run identifier (PID)
#   - bundle identifier
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
#
PATH_AWK=${PATH_AWK:-nawk}
export PATH_AWK
pid="$1"
now=`date`
echo Start of Data | $PATH_AWK 'BEGIN {run_parm = 1
tr_mx=0
HZ=100
event_cnt = 0
cat_cnt = 0
fail_cnt = 0
page_cnt=0
line_cnt=100
pid_cnt=0
last_pid=-1
}
/:/ && (run_parm == 1) {
if ($5 == "end_time")
{
    next
}
else
{
    pid = $1
    if (last_pid != pid)
    {
        last_pid=pid
        bot[last_pid] = tr_mx
    }
    run_cnt[tr_mx] = 0
    tot_elapsed[tr_mx] = 0
    seed[tr_mx] = $NF
    tr[tr_mx] = $5
    tr_mx++
}
next
}
/^Start of Data$/ {
run_parm = 0
FS=":"
last_pid= -1
if (ig != "")
    ig=" Integration Group: " ig
next
}
$6 == "Z" {
pid = $1
if (pid != last_pid)
{
    line_cnt=100
    offset=bot[pid]
}
atim = substr($5,1,9) + 0
tran_ind = $2 + offset -1
if (line_cnt >60)
{
page_cnt++
print ""
print rep_time "                                          Page: " page_cnt
print "RUN TRANSACTION FAILURES FOR SCENARIO " pid ig
print "===================================================================="
printf "%-15.15s %-8.8s %-40.40s %-13.13s\n","Script","Event ID","Event Description","Screen Output"
printf "%-15.15s %-8.8s %-40.40s %-13.13s\n","======","========","=================","============="
line_cnt=5
last_pid = pid
}
printf "%-15.15s %-8.8s %-40.40s %-13.13s\n",tr[tran_ind],$7,$NF,"dump" pid "." $2 "." $3
line_cnt++
fail_cnt ++
}
END {
#
# Do not assume that the awk supports functions
#
if (line_cnt >60)
{
page_cnt++
print ""
print rep_time "                                          Page: " page_cnt
print "RUN TRANSACTION FAILURES FOR SCENARIO " pid ig
print "===================================================================="
printf "%-15.15s %-8.8s %-40.40s %-13.13s\n","Script","Event ID","Event Description","Screen Output"
printf "%-15.15s %-8.8s %-40.40s %-13.13s\n","======","========","=================","============="
line_cnt=5
}
    print ""
    if (fail_cnt == 0)
       print "Total success; no failures!"
    else
       print fail_cnt " Scripts Failed"
}' ig=$PATH_IG rep_time="$now" $PATH_HOME/scenes/runout$pid - $PATH_HOME/data/$PATH_IG/comout$pid | tee $PATH_HOME/data/$PATH_IG/timout$pid
sleep 2 
exit
