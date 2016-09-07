#!/bin/sh
# Add 300 users to the system
#
nawk 'END {for (i=1; i <= 300; i++)
    printf "user%d:x:4%d:26:Test Service User %d (CAPTIVE):/home/aa00e2/testhome/user%d:/bin/sh\n",i,i,i,i
}' </dev/null >>/etc/passwd
nawk 'END {for (i=1; i <= 300; i++)
printf "user%d:JPx.NNczxycR2:9191:0:30:1:30::\n",i
}' </dev/null >>/etc/shadow
i=1
while [ "$i" -lt 301 ]
do
mkdir /home/aa00e2/testhome/user$i
chown user$i  /home/aa00e2/testhome/user$i
chgrp ud /home/aa00e2/testhome/user$i
ln /home/aa00e2/testhome/common_profile /home/aa00e2/testhome/user$i
i=`expr $i + 1`
done 
exit
