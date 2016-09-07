# awkprint formats final report
BEGIN {
print "===========================================================";
print "Report:=  Summary of regenerated script event processing" ;
print "===========================================================";
print "ID        Timeout       Look-for       Title" ;
print "===========================================================";
#
print " ";
sevent=0;
revent=0;
}
# Event declaration
/\\S/ {
      FS=":";
      print $1 "        " $3 "             " $4 "         " substr($NF,1,length($NF)-3); 
      sevent++;
      next
      }
# Process all other lines (relating to execution of event)
/Matched/ {
      FS="#";
      revent++;
      print "Response Time:"$2;
      }
END {
print "===========================================================";
print "Total events declared " sevent;
print "Total events executed " revent;
print "==========================================================="
}

