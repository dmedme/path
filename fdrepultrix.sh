#!/bin/sh5
# fdrepultrix.sh - Report on the output from the run of the Forms Driver
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# This script expects the following input parameters
#
#  1.  - run identifier
#  2.  - start time
#  3.  - end time
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
pid=$1
start_time=$2
end_time=$3
awk 'BEGIN {run_parm = 1
start_time = start_time + 0
end_time = end_time + 0
tr_mx=1
HZ=100
event_cnt = 0
cat_cnt = 0
}
run_parm == 1 && /:/ {
if ($5 == "end_time")
{
    run_parm = 0
    FS=":"
}
else
{
    run_cnt[tr_mx] = 0
    tot_elapsed[tr_mx] = 0
    seed[tr_mx] = $NF
    tr[tr_mx] = $5
    tr_mx++
}
next
}
# Start event ...
$6 == "S" {
start = substr($5,1,9) + 0
if (start < start_time)
start = start_time
next
}
# Setup event ...
$6 == "A" {
for (i = 0; i< cat_cnt; i++)
{
if (lab[i] == $2 $7 )
    next
}
cat_cnt++
lab[i] = $2 $7
desc[lab[i]] = tr[$2] ":" $NF
cnt[lab[i]] = 0
tot[lab[i]]  = 0
tot2[lab[i]]  = 0
max[lab[i]] = 0
min[lab[i]] = 999999
next
}
# Finish event ...
$6 == "F" { 
end = substr($5,1,9) + 0
if (end > end_time)
    end = end_time
run_cnt[$2]++
run = end - start
tot_elapsed[$2] += run
next
}
$6 == "Z" {
atim = substr($5,1,9) + 0
if ( atim < end_time)
{
print "Abort Event Detected; details follow...................."
print $0
}
next }
{
etim = substr($5,1,9) + 0
if (etim < start_time || etim > end_time)
    next
# An interesting event; collect some details
#
# If a pass-mark is defined, can look at that as well
#
  event_cnt++
  cnt[$2 $6]++
  sa = $2 $6 "," cnt[$2 $6]
  saval[sa] = $7
  tot[$2 $6] = tot[$2 $6] + $7
  tot2[$2 $6] = tot2[$2 $6] + $7 * $7
  if ( $7 > max[$2 $6] )
      max[$2 $6] = $7
  if ( $7 < min[$2 $6] )
      min[$2 $6] = $7
}
END {
if (event_cnt == 0)
    print "No results!"
else
{
print "Transaction Stream Elapsed Times/Seconds"
print "============================="
printf "%-30.30s %-13.13s %5.5s %15.15s\n","Transaction","Seed","Count","Avge Elapsed"
for (i=1;i < tr_mx;i++)
{
if (run_cnt[i] != 0)
{
av = tot_elapsed[i]/run_cnt[i]
}
else
av=0
printf "%-30.30s %-13.13s %5.5g %15.5g\n",tr[i],seed[i],run_cnt[i],av
}
print "Response Time Summary/Seconds"
print "============================="
printf "%-44.44s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s\n","Event Description","Count","Avge","SD","Min","10%","20%","30%","40%","50%","60%","70%","80%","90%","Max"
for (i = 0; i < cat_cnt; i++)
{
if (cnt[lab[i]] > 0)
{
l=lab[i]
d = desc[l]
c = cnt[l]
mn = int( 100 * min[l]/HZ+.5)/100
mx = int( 100 * max[l]/HZ+.5)/100
av = int( 100 * tot[l]/cnt[l]/HZ+.5)/100
sd = int( 100 * sqrt(tot2[l] - tot[l]/cnt[l]*tot[l])/cnt[l]/HZ+.5)/100
for (j=1; j < c; j++)
   for (k=c; k > j; k--)
   {
     sa1 = l "," j
     sa2 = l "," k
     if (saval[sa1] > saval[sa2])
     {
        x = saval[sa2] 
        saval[sa2] = saval[sa1]
        saval[sa1] = x
     }
   }
   sa = l "," int(.1*c)
   pc = saval[sa]/HZ
pc1 = int(pc * 100)/100
   sa = l "," int(.2*c)
   pc = saval[sa]/HZ
pc2 = int(pc * 100)/100
   sa = l "," int(.3*c)
   pc = saval[sa]/HZ
pc3 = int(pc * 100)/100
   sa = l "," int(.4*c)
   pc = saval[sa]/HZ
pc4 = int(pc * 100)/100
   sa = l "," int(.5*c)
   pc = saval[sa]/HZ
pc5 = int(pc * 100)/100
   sa = l "," int(.6*c)
   pc = saval[sa]/HZ
pc6 = int(pc * 100)/100
   sa = l "," int(.7*c)
   pc = saval[sa]/HZ
pc7 = int(pc * 100)/100
   sa = l "," int(.8*c)
   pc = saval[sa]/HZ
pc8 = int(pc * 100)/100
   sa = l "," int(.9*c)
   pc = saval[sa]/HZ
pc9 = int(pc * 100)/100
printf "%-44.44s %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g %5.5g\n",d,c,av,sd,mn,pc1,pc2,pc3,pc4,pc5,pc6,pc7,pc8,pc9,mx
}
}
}
}' start_time=$start_time end_time=$end_time runout$pid comout$pid |
tee timout$pid
#
# Report on the memory consumption and resources consumed by the benchmark
# run.
#
# NB. Image accounting must be active, and the usual name of the file varies
#     from UNIX implementation to implementation. Possibilities are:
#  - set up especially for this run (use accton; can use any name)
#  - /usr/adm/acct (BSD)
#  - /usr/adm/pacct (SYSV)
#
# Report computes the average sizes of various processes from the
# process image accounting records, and gets the system and user
# time. Memory units measure is a bit uncertain; usually K.
# This script assumes tick size is 1/100 of a second; best to go
# with the value of AHZ defined in /usr/include/sys/acct.h,
# /usr/include/sys/param.h or wherever (use grep to find it).
#
# If multiple users are being run to overcome number of active process
# limitations, insert between the hisps and awk steps a grep for the
# user id number, as located in /etc/passwd. eg. grep '\|252\|' if
# the script is being run by user 252. Otherwise, the report will sum
# over all user IDs.
#
hisps < /usr/adm/acct -s $start_time -e $end_time | awk -F\| 'BEGIN {
lab[1] = "ptydrive"
desc["ptydrive"] = "Forms Driver"
lab[2] = "cfxiap  "
desc["cfxiap  "] = "Cedardata Forms"
lab[3] = "orapop  "
desc["orapop  "] = "ORACLE Minder"
lab[4] = "oracle  "
desc["oracle  "] = "ORACLE Agent"
lab[5] = "act_r05   "
desc["act_r05 "] = "Hierarchical Cost Centre Report"
lab[6] = "act_r32 "
desc["act_r32 "] = "Detailed Item Listing"
lab[7] = "sqlplus "
desc["sqlplus "] = "SQL*PLUS"
lab[8] = "act_r90 "
desc["pstat   "] = "Trial Balance Report"
lab[9] = "vmstat  "
desc["vmstat  "] = "VM Monitoring"
lab[10] = "iostat  "
desc["iostat  "] = "I/O-CPU Monitoring"
lab[11] = "sqlload "
desc["sqlload "] = "SQL*Loader"
lab[12] = "ecp014  "
desc["ecp014  "] = "Interface File Pre-processing"
lab[13] = "act_r50 "
desc["act_r50 "] = "CLINK Posting"
lab[14] = "other"
desc["other"] = "Other Activity"
for (i = 1; i < 15; i++)
{
mem[lab[i]] = 0
cnt[lab[i]] = 0
usr[lab[i]] = 0
sys[lab[i]] = 0
}
}
{ if (($2 + $3) == 0)
     next
for (i = 1; i < 14; i++)
{
   if ($1 == lab[i])
   {
      if ($1 == "oracle  ")
          mem[lab[i]] += ($8 - 8068)
      else
          mem[lab[i]] += $8
      cnt[lab[i]]++
      usr[lab[i]] += $2
      sys[lab[i]]  += $3
      next
   }
}
# increment the other
      mem[lab[i]] += $8
      cnt[lab[i]]++
      usr[lab[i]] += $2
      sys[lab[i]]  += $3
}
END {
printf "%-20.20s %10.10s %10.10s %10.10s %10.10s %10.10s\n","Process","Count","Av.Mem/K","User CPU/s","Sys CPU/s","Total CPU/s" 
for (i = 1; i < 15; i++)
{
if (cnt[lab[i]] > 0)
{
d = desc[lab[i]]
c = cnt[lab[i]]
mm = mem[lab[i]]/cnt[lab[i]]
us = usr[lab[i]]
sy = sys[lab[i]]
tot = us + sy
printf "%-20.20s %10.10g %10.10g %10.10g %10.10g %10.10g\n",d,c,mm,us,sy,tot
}
}
printf "%-20.20s %10.10g %10.10g\n","ORACLE SGA",1,8068
}' | tee resout$pid
# The pstat output, looking for overflow during the run is in over$pid
cat << EOF
The run parameters for the run are in runout$pid
The vmstat output for the run is in vmout$pid
The iostat output for the run is in vmout$pid
The cpu/memory output for the run is in resout$pid
The response time output for the run is in timout$pid
The elapsed time output for the run is in elapsed$pid
EOF
exit
