nawk 'BEGIN { print "\tSystem\t/u1\t/u2\t/u3"
 print "Time\tdisc3-0\tdisc3-1\tdisc3-2\tdisc3-3"
}
function out_line() {
    print t "\t" reads_writes["disc3-0"] "\t" reads_writes["disc3-1"] "\t" reads_writes["disc3-2"] "\t" reads_writes["disc3-3"]
    reads_writes["disc3-0"]  = 0
 reads_writes["disc3-1"]  = 0
reads_writes["disc3-2"]  = 0
 reads_writes["disc3-3"]  = 0
 t = ""
}
NF == 0 { if (t != "")
   out_line()
   next
}
$1 ~ ":" { t = $1
reads_writes[$2] = $5
    next
}
{
reads_writes[$1] = $4
} 
END {
    out_line()
}'
