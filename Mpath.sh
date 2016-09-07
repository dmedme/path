:
# Mpath.sh - compile the PATH executables
# @(#) $Name$ $Id$
# Copyright (c) E2 Systems Limited 1993
#
# One parameter: the Operating System
if [ "$#" -lt 1 ]
then
echo Provide the Operating System
exit
fi
OS=$1
# PATH
# ====
#
# AIX Version 3.2
case "$OS" in
AIX)
CFLAGS="-gx -DAIX -DNOSTDIO -DTIMEOUT_HANDLER -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT"
CLIBS="../e2common/comlib.a -lcur -lm -lc -lbsd"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
# On release 4 of AIX, must use -lcurses rather than -lcur.
AIX4)
CFLAGS="-gx -DAIX -DAIX4 -DNOSTDIO -DTIMEOUT_HANDLER -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT"
CLIBS="../e2common/comlib.a -lcurses -lm -lc -lbsd"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
LINUX)
CFLAGS="-g2 -DPOSIX -I. -I../e2common -DAT -DTIMEOUT_HANDLER -DLINUX -DPATH_AT -DUNIX -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
CATFLAGS="$CFLAGS"
CLIBS="../e2common/comlib.a -lpthread -lncurses -lm -lc"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
ANDROID)
NDK=/opt/NVPACK/android-ndk-r7
export NDK
SYSROOT=$NDK/platforms/android-8/arch-arm
export SYSROOT
CC="$NDK/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gcc --sysroot=$SYSROOT"
export CC
AND_CFLAGS='-march=armv7-a -mfloat-abi=softfp'
export AND_CFLAGS
AND_LDFLAGS='-Wl,--fix-cortex-a8'
export AND_LDFLAGS
CFLAGS="$AND_CFLAGS -g2 -DPOSIX -I. -I../e2common -DAT -DTIMEOUT_HANDLER -DLINUX -DPATH_AT -DUNIX -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DANDROID -I$NDK/../ncurses-5.7/include"
CATFLAGS="$CFLAGS"
CLIBS="$AND_LDFLAGS ../e2common/comlib.a -L$NDK/../ncurses-5.7/lib -lncurses -lc -lncurses -lm -lc"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
OSF)
CFLAGS="-g -DAIX -DOSF -DMOD -DNOSTDIO -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT"
CLIBS="../e2common/comlib.a -lcurses -ltermlib -lm -lc -lbsd"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# SUNOS 4 or higher. SUNOS 3 is more like Ultrix, and the code
# probably doesn't work any more..
#
SUNOS)
CC=/usr/xpg2bin/cc
CFLAGS="-g -DNOSTDIO -DSUN -DBSD -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -I. -I../e2common"
CLIBS="../e2common/comlib.a /usr/5lib/libcurses.a /usr/5lib/libtermlib.a -lm"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB=ranlib
HLIBS="$CLIBS /usr/lib/libc.a"
AR="ar rv"
;;
#
# Pyramid OSx 5.1
#
PYR)
CFLAGS="-gx -DPYR -DNOSTDIO -I. -I../e2common -I/usr/.attinclude -I/usr/.ucbinclude"
LIBS="pathatlib.a /usr/.attlib/libcurses.a /.attlib/libc.a /.ucblib/libc.a"
CC="att cc"
RANLIB=ranlib
HLIBS=$CLIBS
AR="ar rv"
;;
#
# Ultrix
#
ULTRIX)
LIBS="pathatlib.a -ltermcap -lm"
CFLAGS="-DULTRIX -DBSD -DNOSTDIO -g -I. -I../e2common"
LIBS="pathatlib.a -ltermcap -lm"
CATFLAGS="$CFLAGS -DPATH_AT"
CLIBS="../e2common/comlib.a -lcursesX -ltermlib -lm"
ATLIBS="pathatlib.a $CLIBS"
SELIBS="pathselib.a $CLIBS"
RANLIB="ranlib"
HLIBS="$CLIBS -lc"
AR="ar rv"
;;
#
# Sequent DYNIX
# - Compile in the ATT Universe, but must pick up the BSD select()
#
SEQ)
CC="cc"
CFLAGS="-O -DBSD -DNOSTDIO -DSEQ -DTERMCAP -I. -I../e2common -I/usr/.include"
LIBS="pathatlib.a /.lib/libc.a -lcurses -lm"
RANLIB="ranlib"
HLIBS="$CLIBS"
PATHLIBS="pathatlib.a $CLIBS"
AR="ar rv"
;;
# 
# SCO  (System V.3)
#
# The Microsoft C compiler does not generate correct code when
# unions or pointers to unions are passed to functions. We are
# therefore using the AT&T compiler, which generates larger programs,
# but which gets the symbol information correct for sdb, and which
# generates correct coff.
SCO)
#
CC="rcc"
CFLAGS="-g -DSCO -DNOSTDIO -DM_UNIX -DATT -I. -I../e2common"
SEFLAGS="$CFLAGS"
CATFLAGS="$CFLAGS -DPATH_AT"
CLIBS="../e2common/comlib.a -lsocket -lcurses -ltermlib -lm"
PATHLIBS="pathatlib.a $CLIBS"
RANLIB="ar ts"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# System V4
#
V4)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc /usr/ucblib/libucb.a"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DV4 -DSCO -DNOSTDIO -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
HP8)
#
# HP-UX 9 for 800
#
#CLIBS="../e2common/comlib.a -lHcurses -ltermlib -lm -lc -lV3 -lBSD"
CLIBS="../e2common/comlib.a -lcurses -ltermlib -lm -lc -lV3 -lBSD"
CFLAGS="-DPOSIX -DHP7 -DHP10 -DNOSTDIO -g -I. -I../e2common"
SEFLAGS="$CFLAGS"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
RANLIB="ar ts"
CC=cc
#CC=c89
AR="ar rv"
;;
#
#
# ICL System V4
#
ICL)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc /usr/ucblib/libucb.a"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DICL -DV4 -DSCO -DNOSTDIO -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
SOLAR)
CC="gcc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc"
SEFLAGS="$CFLAGS"
CFLAGS="-DSOLAR -m64 -g -DDO_95 -DMOD -DV4 -DSCO -DNOSTDIO -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# DG-UX System V4
#
DGUX)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lnsl -lcurses -lm -lc"
SEFLAGS="$CFLAGS"
CFLAGS="-O2 -DV4 -DSCO -DNOSTDIO -DDGUX -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
#
# Pyramid OSXDC
#
OSXDC)
CC="/usr/ccs/bin/cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc /usr/ucblib/libucb.a"
SEFLAGS="$CFLAGS"
CFLAGS="-O -DICL -DV4 -DSCO -DNOSTDIO -DOSXDC -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
NILE)
CC="/usr/ccs/bin/cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc /usr/ucblib/libucb.a"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DICL -DV4 -DSCO -DNOSTDIO -DOSXDC -I. -I../e2common -DNILE -WM,-mips2"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# DYNIX PTX
#
PTX)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc -lseq"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DV4 -DSCO -DNOSTDIO -DPTX -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# ATT system V.3.2
#
V32)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lpt -lnsl_s mod/libcurses.a mod/libtermlib.a -lm -lc -lx"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DV4 -DV32 -DSCO -DNOSTDIO -I/usr/include -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;
#
# Olivetti V4
#
MOD)
CC="cc"
RANLIB="ar ts"
CLIBS="../e2common/comlib.a -lsocket -lnsl -lcurses -ltermlib -lm -lc /usr/ucblib/libucb.a"
SEFLAGS="$CFLAGS"
CFLAGS="-g -DV4 -DMOD -DSCO -DNOSTDIO -I. -I../e2common"
CATFLAGS="$CFLAGS -DPATH_AT -DATT"
PATHLIBS="pathatlib.a $CLIBS"
HLIBS="$CLIBS"
AR="ar rv"
;;

esac
export CC RANLIB CLIBS SEFLAGS CFLAGS CATFLAGS CFLAGS2 CATFLAGS2 PATHLIBS HLIBS AR
#PATH_SOURCE=${PATH_SOURCE:-`pwd`}
#cd $PATH_SOURCE
shift
make -e -f M.path $*
