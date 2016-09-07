BEGIN { of=0
}
NF == 7 && $NF == "`" && $1 ~ "/$" {
if (of != 0)
    close(outf)
   outf=substr($1,1,(length($1) - 1))
   of = 1
 print outf
next
}
of == 1 {
   print $0 >outf
} 


