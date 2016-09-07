#!/bin/sh
# concat.sh - concatenate a series of SQL files to give a PATH echo file
# that submits them to sqlplus a session at a time.
# Arguments:
# The files to concatenate
#
PATH_SOURCE=${PATH_SOURCE:-/oracle/e2/path}
PATH_AWK=${PATH_AWK:-awk}
usr_id=repairs/rep
export PATH_SOURCE PATH_AWK usr_id
{
cat << EOF
\\R10\\
\\W60\\
\\SD1:3600:Sync::Synchronisatition Point\\
EOF
for i in ` ls -tr $* `
do
$PATH_AWK 'BEGIN { flag = 0
print "`sqlplus '$usr_id' << EOF"
print "`"
}
/^REM/ {
    if (flag == 1)
    {
        print "/"
        print "`"
    }
    flag = 0 
    print "# " $0 " #"
    next
}
flag == 0 {
    print "`" $0
    flag = 1
    next
}
flag == 1 && $0 == "/" {
    print "/"
    print "`"
    print "`prompt Sync`" 
    print "\\TD1:\\" 
    print "`" 
    print "`" 
    flag = 0
    next
}
flag == 1 {
    print  $0
    next
}
END {
print "`exit"
print "EOF"
print "`"
}' $i
done 2>&1
} |
$PATH_AWK -f $PATH_SOURCE/fdconvert.awk - | cat $PATH_SOURCE/preamble1 -
