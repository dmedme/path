# fdintrun.sh
# **************************************************************************
# int_run
#
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# Function to actually run an Integration Group
# Parameters: 1 - Description
#
# Runs the current Integration Group, PATH_IG
#
. $PATH_SOURCE/fdintreview.sh
int_run () {
desc="$*"
cd $PATH_HOME/data/$PATH_IG
SHELL=/bin/sh make -f $PATH_HOME/igs/Mk.$PATH_IG
fdsysrep.sh "*"
intreview $desc 
return 0
}
