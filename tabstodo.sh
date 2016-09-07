:
# tabstodo.sh - Identify the application tables referenced by the Bull
# repairs system.
#
if [ $# -lt 1 ]
then
   echo Provide a list of SQL files produced by trcproc from Repairs System
   echo Traces
   exit 1
fi
files=$*
nawk '/Accessed Table.*"/ { print $NF }'  $files | sed 's/"//g' | sort | uniq
