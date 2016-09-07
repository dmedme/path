#!/bin/sh
# siftdata.sh - Identify the input elements in the data sets where the
# scripts aborted (bad) or succeeded (good)
#
# The script can be used either to weed out bad data from input files,
# or to weed out used data from input files where the values cannot be re-used
# between runs
#
# The datafiles are built up in the current directory. If they already
# exist, more data will be appended to them.
#
if [ $# -lt 1 ]
then
    echo Provide a runout file ID
    exit
fi
if [ $# -lt 2 ]
then
look_for=bad
else
look_for=$2
fi
pid=$1
#
# Scan the runout and comout file, repeat the generation valfetch, and
# either find the failed element (look_for = bad) or the good elements
# (look_for = good) 
#
nawk 'function script_data() {
df["fSHSE_EA.awk"] = "postable.db"
df["fSHSE_EPO.awk"] = "inventpo.db"
df["fSHSE_EPO.awk"] = "acc.db"
df["fSHSE_ESV.awk"] = "directorate.db"
df["fShs_budenq.awk"] = "budenq.db"
df["fShs_budin.awk"] = "budin.db"
df["fShs_expenq.awk"] = "expenq.db"
df["fShs_expin.awk"] = "expin.db"
df["fShs_grn.awk"] = "grn.db"
df["fShs_invmatch.awk"] = "invmatch.db"
df["fShs_invregpo.awk"] = "inventpo.db"
df["fShs_jenq.awk"] = "jouenq.db"
df["fShs_jent.awk"] = "jouent.db"
df["fShs_miles.awk"] = "nose84.db"
df["fShs_poentns.awk"] = "poentns.db"
df["fShs_post.awk"] = "post.db"
df["fShs_stockreq.awk"] = "requisit.db"
mult["fSHSE_EA.awk"] = 10
mult["fSHSE_EPO.awk"]  = 1
mult["fSHSE_ESV.awk"]  = 1
mult["fShs_budenq.awk"]  = 1
mult["fShs_budin.awk"]  = 1
mult["fShs_expenq.awk"]  = 1
mult["fShs_expin.awk"]  = 1
mult["fShs_grn.awk"]  = 1
mult["fShs_invmatch.awk"]  = 1
mult["fShs_invregpo.awk"]  = 1
mult["fShs_jenq.awk"]  = 1
mult["fShs_jent.awk"]  = 1
mult["fShs_miles.awk"]  = 1
mult["fShs_poentns.awk"]  = 1
mult["fShs_post.awk"]  = 1
mult["fShs_stockenq.awk"]  = 1
mult["fShs_stockreq.awk"]  = 1
suc_ev["fSHSE_EA.awk"] ="N9"
suc_ev["fSHSE_EPO.awk"]  ="DJ"
suc_ev["fSHSE_ESV.awk"]  ="A5"
suc_ev["fShs_budenq.awk"]  ="A5"
suc_ev["fShs_budin.awk"]  ="A7"
suc_ev["fShs_expenq.awk"]  ="A5"
suc_ev["fShs_expin.awk"]  ="B3"
suc_ev["fShs_grn.awk"]  ="A7"
suc_ev["fShs_invmatch.awk"]  ="AA"
suc_ev["fShs_invregpo.awk"]  ="AA"
suc_ev["fShs_jenq.awk"]  ="B0"
suc_ev["fShs_jent.awk"]  ="A8"
suc_ev["fShs_miles.awk"]  ="B3"
suc_ev["fShs_poentns.awk"]  ="A6"
suc_ev["fShs_post.awk"]  ="A6"
suc_ev["fShs_stockenq.awk"]  ="B3"
suc_ev["fShs_stockreq.awk"]  ="C2"
return
}
BEGIN {run_parm = 1
tr_mx=1
event_cnt = 0
cat_cnt = 0
script_data()
}
run_parm == 1 && NR > 3 {
if ($5 == "end_time")
{
    run_parm = 0
    FS=":"
}
else
{
    run_cnt[tr_mx] = 0
    tot_elapsed[tr_mx] = 0
    tr[tr_mx] = $2
    tr_mx++
}
next
}
#
# Start event ...
#
$6 == "S" {
out_flag = 0
this_bun = $2
this_g = $3
this_scr = tr[this_bun]
this_df = df[this_scr]
this_mult = mult[this_scr]
this_suc_ev = suc_ev[this_scr]
cnt = 0
next
}
#
# Finish event ...
#
$6 == "F" { 
if (out_flag == 1 && lookfor == "bad")
{
#print "valfetch.sh $PATH_HOME/rules/" this_df " " this_bun " " this_g " " this_mult*(cnt + 1) " > junk.tmp" 
#print "tail -" this_mult " junk.tmp >> " this_df 
system("valfetch.sh $PATH_HOME/rules/" this_df " " this_bun " " this_g " " this_mult*(cnt + 1) " > junk.tmp" )
system("tail -" this_mult " junk.tmp >> " this_df )
}
else
if (lookfor == "good")
#print "valfetch.sh $PATH_HOME/rules/" this_df " " this_bun " " this_g " " this_mult*cnt " >> " this_df 
system("valfetch.sh $PATH_HOME/rules/" this_df " " this_bun " " this_g " " this_mult*cnt " >> " this_df )
next
}
$6 == "Z" {
out_flag = 1
next
}
$6 == this_suc_ev {
    cnt++
}' lookfor=$look_for runout$pid comout$pid
