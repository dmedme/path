/*
 *    ptytty.c - Common routines to do with handling pty/tty pairs
 *
 *    Copyright (C) E2 Systems 1993
 *
 *
 * Signal handling
 * ===============
 * SIGTERM - terminate request
 * SIGBUS  - should not happen (evidence of machine stress)
 * SIGALRM - used to control typing rate
 * SIGCHLD - watching for death of process
 * SIGINT  - Toggle between See Through and Terminal Independent input modes
 *
 */
static char * sccs_id="@(#) $Name$ $Id$\n\
Copyright (C) E2 Systems Limited 1993";
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef V4
#define NEED_STREAMS
#endif
#ifdef OSF
#define NEED_STREAMS
#endif
#ifdef V32
#include <time.h>
#define setsid() setpgrp()
#define getsid() getpgrp()
#else
#include <sys/time.h>
#endif
#ifdef SEQ
#include "/usr/.include/sys/ioctl.h"
#else
#ifdef PYR
#include "/usr/.ucbinclude/sys/ioctl.h"
#else
#include <sys/ioctl.h>
#endif
#endif
#ifdef SEQ
#include <fcntl.h>
#include <time.h>
#else
#ifdef ULTRIX
#include <fcntl.h>
#define JOB_STUFF
#else
#ifdef AIX
#include <fcntl.h>
#include <termios.h>
#define JOB_STUFF
#define MUST_SPAWN
#else
#ifdef ANDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif
#endif
#endif
#ifdef V32
#include <sys/termio.h>
#define termios termio
#define SIGCHLD SIGCLD
#define tcsetattr(x,y,z) ioctl((x),(y),(z))
#define TCSANOW TCSETA
#define tcgetattr(x,y) ioctl((x),TCGETA,(y))
#define JOB_STUFF
char * ptsname();
#else
#ifdef SCO
#include <termios.h>
#define JOB_STUFF
#define MUST_SPAWN
#endif
#ifdef LINUX
#include <termios.h>
#define JOB_STUFF
#define MUST_SPAWN
#endif
#ifdef ULTRIX
#include <sys/termios.h>
#endif
#include <sys/wait.h>
#endif
#include <signal.h>
#include <errno.h>
#ifdef NEED_STREAMS
#include <sys/stream.h>
#include <sys/stropts.h>
#ifdef DEBUG
#include <sys/param.h>
#ifndef V32
#include <sys/types.h>
#endif
#include <sys/conf.h>
#endif
#endif
#include "circlib.h"
#include "matchlib.h"
#include "ptytty.h"
#ifdef NEED_STREAMS
#ifdef DEBUG
static void strmdump ANSIARGS((int fd));
#endif
#endif
#ifdef HP7
#include <sys/termios.h>
#define JOB_STUFF
#endif
#ifdef SOLAR
#include <sys/ioccom.h>
#include <sys/ptyvar.h>
#endif
#ifdef DEBUG
static FILE * debug_log;
#endif
extern char * ttyname();
#ifdef MANUAL_RESTORE
static struct termios save_tm;
void restore_tty (fd)
int fd;
{
#ifdef JOB_STUFF
    if (tcsetattr(fd,TCSANOW,&save_tm) < 0)
        perror("Setting Terminal Attributes");
#endif
    return;
}
#endif
/***************************************************************************
 * Alarm clock routine
 */
void alarm_catch()
{
time_t t;

    if ((PTYCHAR->buf_cnt > 0) || pg.in_check)
        pg.write_mask = pg.sav_write_mask;        /* can send again */
#ifdef DEBUG_FULL
    t = time(0);
    fprintf(stderr, "# Alarm clock (flag=%d) gone off %s\n#",
               pg.alarm_flag, ctime(&t));
    pg.alarm_flag = 1;
#endif
    return;
}
/***************************************************************************
 * Dump the status of the circular buffers
 */
void statdump()
{
    (void) fputs("Control Buffer\n", stderr);
    bufndump(PTYCHAR);
    (void) fprintf(stderr, "PTYCHAR->buf_cnt : %d\n",PTYCHAR->buf_cnt);
    (void) fprintf(stderr, "PTYCHAR->head : %d\n",(int) PTYCHAR->head);
    (void) fprintf(stderr, "PTYCHAR->tail : %d\n",(int) PTYCHAR->tail);
    (void) fputs("Script Input Buffer\n", stderr);
    bufdump(PTYIN);
    (void) fprintf(stderr, "PTYIN->buf_cnt : %d\n",PTYIN->buf_cnt);
    (void) fprintf(stderr, "PTYIN->head : %d\n",(int) PTYIN->head);
    (void) fprintf(stderr, "PTYIN->tail : %d\n",(int) PTYIN->tail);
    (void) fputs("Program Output Buffer\n", stderr);
    bufdump(PTYOUT);
    (void) fprintf(stderr, "PTYOUT->buf_cnt : %d\n",PTYOUT->buf_cnt);
    (void) fprintf(stderr, "PTYOUT->head : %d\n",(int) PTYOUT->head);
    (void) fprintf(stderr, "PTYOUT->tail : %d\n",(int) PTYOUT->tail);
    return;
}
/*
 * Should never happen....
 */
void sigbus()
{
    (void) fprintf(stderr,"Bus Error Signal Received\n");
    statdump();
    return;
}
#ifdef PATH_AT
/*
 * Toggle the input processing mode
 */
void sigint()
{
    if (pg.int_flag)
        pg.int_flag = 0;
    else
        pg.int_flag = 1;
    return;
}
#endif
/***********************************************************************
 * This is the abnormal termination routine; dumps out status information
 * for debugging purposes
 */
void sigterm()
{
    (void) fprintf(stderr,"Abort command received\n");
    if (!pg.dumpout_flag)
        statdump();
    event_record("F", (struct event_con *) NULL);
    if (pg.child_pid)
    {
#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif
        (void) sigset(SIGCLD,SIG_DFL);
        (void) kill(pg.child_pid,SIGHUP);       /* HANGUP to the child */
    }
    if (isatty(0))
    {
#ifdef PATH_AT
        endwin();
#ifdef OSF
        unset_curses_modes();
#endif
#endif
#ifdef MANUAL_RESTORE
        restore_tty(0);
#endif
    }
    _exit(1);                                    /* scarper */
}
/***********************************************************************
 * Child process has died; shut down gracefully
 */
void sigchild(sig)
int sig;
{
int pid;
#ifdef SCO
int
#else
#ifdef AIX
int
#else
int
#endif
#endif
    stat_val;

    if (sig != SIGCHLD)
    {
        (void) fprintf(stderr,"Unexpected Child Status Change: %d\r\n",
                       sig);
        (void) fflush(stderr);

    }
    else
#ifndef V32
    if ((pid = waitpid(pg.child_pid,&stat_val,WNOHANG)) == pg.child_pid)
#else
    if ((pid = wait(&stat_val)) == pg.child_pid)
#endif
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Child Death signal received, status %x\n",
                       stat_val);
        if (WIFEXITED(stat_val))
            (void) fprintf(stderr,"Exited with status %d\n",
                       WEXITSTATUS(stat_val));
        else
        if (WIFSIGNALED(stat_val))
            (void) fprintf(stderr,"Signalled with %d\n",
                       WTERMSIG(stat_val));
         
        statdump();
#endif
        pg.sav_write_mask = 0;
        pg.write_mask = 0;
        pg.in_check = 0;
        PTYOUT->buf_cnt = 0;
        PTYCHAR->buf_cnt = 0;
        pg.child_pid = 0;
    }
    return;
}
/**********************************************************************
 * Get round the problem with setsid() not working on a process group
 * leader
 */
void sig_zap ()
{
    exit(0);
}
/*********************************************************
 * Get a tty/pty pair
 * -  There are a number of peculiarities about this code
 *    -  the pty skeleton needs to be adjusted to suit the 
 *       individual implementation
 *    -  pty's can only be driven by detached processes on pure BSD 4.2
 *    -  In general, only one process can read/write a pty at a time. So
 *       how is the fork() managed? 
 */
struct pty_tty_pair * getpty()
{
static struct pty_tty_pair x;
register short int i;
register short int j;
long int parm = -1;
#ifdef DEBUG
char * tname;

    debug_log = fopen("ptytty.log","w");
    setbuf(debug_log,NULL);
    if ((tname = ttyname(0)) != (char *) NULL)
    {
        fputs(tname, debug_log);
        fputc('\n', debug_log);
    }
#endif
#ifdef PTX
/*
 * DYNIX has a very similar interface to what we want!
 */
    char * ptyname;
    x.ptyfd = getpseudotty(&x.ttyname,&ptyname);
    x.ttyfd = open(x.ttyname,O_RDWR);
#else
#ifdef OSF_BROKEN
/*
 * OSF has a very similar interface to what we want!
 */
    if ( openpty(&x.ptyfd, &x.ttyfd, 0,0,0) != 0)
    {
        perror("pty open failed");
        return (struct pty_tty_pair *) NULL;
    }
    else
        x.ttyname = strdup(ttyname(x.ptyfd));
#else
#ifdef OSF
/*
 * Pseudo-terminals are now completely revamped. They are implemented as
 * streams. There is a single master, /dev/ptmx, which is a 'clone(7)' device,
 * and the usual clutch of slaves /dev/pts/*. The master is opened, and a
 * slave is allocated automatically (no more looping through the names).
 * The slave is locked on allocation.
 * grantpt() sets the ownership and mode appropriately.
 * unlockpt() unlocks it.
 * ptsname() gets its name.
 * It can now be opened.
 * Streams modules "ptem" and "ldterm" need to be pushed on to it.
 * It is now ready for use.
 */
    if ((x.ptyfd=open("/dev/ptmx_bsd",O_RDWR)) >=0)
    {
char * ptsname();

        grantpt(x.ptyfd);
        unlockpt(x.ptyfd);
        x.ttyname = ptsname(x.ptyfd);
        if ((x.ttyfd=open(x.ttyname,O_RDWR)) >=0)
        {
            if ( ioctl(x.ttyfd,I_PUSH,"ptem") < 0)
            {
                perror("Failed to push ptem");
            }
#ifdef WANT_TERM
            if ( ioctl(x.ttyfd,I_PUSH,"ldterm") < 0)
            {
                perror("Failed to push ldterm");
            }
#endif
#ifdef DEBUG
fprintf(stderr,"0 streams\n");
            strmdump(0);
fprintf(stderr,"PTY streams\n");
            strmdump(x.ptyfd);
fprintf(stderr,"TTY %s streams\n",x.ttyname);
            strmdump(x.ttyfd);
#endif
#ifdef WORKS
            if ( ioctl(x.ptyfd,TIOCREMOTE,(char *) &parm) < 0)
            {
                perror("Failed to set remote mode");
            }
#endif
        }
        else x.ptyfd = -1;
    }
#else
#ifdef V4
/*
 * Pseudo-terminals are now completely revamped. They are implemented as
 * streams. There is a single master, /dev/ptmx, which is a 'clone(7)' device,
 * and the usual clutch of slaves /dev/pts/*. The master is opened, and a
 * slave is allocated automatically (no more looping through the names).
 * The slave is locked on allocation.
 * grantpt() sets the ownership and mode appropriately.
 * unlockpt() unlocks it.
 * ptsname() gets its name.
 * It can now be opened.
 * Streams modules "ptem" and "ldterm" need to be pushed on to it.
 * It is now ready for use.
 */
    if ((x.ptyfd=open("/dev/ptmx",O_RDWR)) >=0)
    {
        grantpt(x.ptyfd);
        unlockpt(x.ptyfd);
        x.ttyname = ptsname(x.ptyfd);
        if ((x.ttyfd=open(x.ttyname,O_RDWR)) >=0)
        {
            if ( ioctl(x.ttyfd,I_PUSH,"ptem") < 0)
            {
                perror("Failed to push ptem");
            }
            if ( ioctl(x.ttyfd,I_PUSH,"ldterm") < 0)
            {
                perror("Failed to push ldterm");
            }
            if ( ioctl(x.ttyfd,I_PUSH,"ttcompat") < 0)
            {
                perror("Failed to push ttcompat");
            }
#ifdef DEBUG
fprintf(stderr,"0 streams\n");
            strmdump(0);
fprintf(stderr,"PTY streams\n");
            strmdump(x.ptyfd);
fprintf(stderr,"TTY %s streams\n",x.ttyname);
            strmdump(x.ttyfd);
#endif
        }
        else x.ptyfd = -1;
    }
#else
#ifdef AIX
/*
 * They are also revamped in AIX V.3. The philosophy is similar to
 * that of V.4, but a bit tidier from the user's viewpoint
 */
    if ((x.ptyfd=open("/dev/ptc",O_RDWR)) >= 0)
    {
        x.ttyname = strdup(ttyname(x.ptyfd));
        if ((x.ttyfd=open(x.ttyname,O_RDWR)) < 0)
            x.ptyfd = -1;
#ifdef DEBUG
        fputs("0 streams\n", debug_log);
        strmdump(0);
        fputs("PTY streams\n", debug_log);
        strmdump(x.ptyfd);
        fprintf(debug_log,"TTY %s streams\n",x.ttyname);
        strmdump(x.ttyfd);
#endif
    }
#else
#ifdef LINUX
#ifdef ANDROID
    if ((x.ptyfd=open("/dev/ptmx",O_RDWR)) >=0)
#else
    if ((x.ptyfd= posix_openpt(O_RDWR | O_NOCTTY)) >=0)
#endif
    {
char * ptsname();

        grantpt(x.ptyfd);
        unlockpt(x.ptyfd);
        x.ttyname = ptsname(x.ptyfd);
        x.ttyfd=open(x.ttyname, O_RDWR);
    }

#else
/*
 * Traditional BSD-style pty device allocation. HP has a smarter way of
 * allocating than this, but this is still supported.
 * The pty's are in groups; this supports up to 128.
 * Recent SCO variants are somewhat different; these need to be worked in
 */
#ifdef HP7
#define P_GRP_CNT 7
#define P_IND_CNT 16
#define P_GRP_OFF 13
#define P_IND_OFF 14 
#define P_T_OFF 10 
    static char ptybase[]="/dev/ptym/ptyp0";
#else
#ifdef SCO
#define P_GRP_CNT 1
#define P_GRP_OFF 8
#define P_IND_OFF 10 
#define P_IND_CNT 9
#define P_T_OFF 5
    static char ptybase[]="/dev/ptyp00";
#else
#define P_GRP_CNT 8
#define P_GRP_OFF 8
#define P_IND_OFF 9
#define P_IND_CNT 16
#define P_T_OFF 5
    static char ptybase[]="/dev/ptyp0";
#endif
#endif
    static char hexlis[]="0123456789abcdef";
    char * hexptr;
    for (x.ptyfd = -1,
         x.ttyfd = -1,
         i= P_GRP_CNT;
             (x.ptyfd < 0) && i;
                  i--,
                  ptybase[P_GRP_OFF]++)
    {
        for (j=P_IND_CNT,hexptr=hexlis;
                    j;
                       j--,hexptr++,ptybase[P_IND_OFF] = *(hexptr))
        {
            if ((x.ptyfd=open(ptybase,O_RDWR)) >=0)
            {
                ptybase[P_T_OFF]='t';
#ifdef HP7
                memcpy(&ptybase[P_T_OFF - 2],&ptybase[P_T_OFF - 1],
                                sizeof(ptybase) - P_T_OFF + 1);
#endif
                if ((x.ttyfd=open(ptybase,O_RDWR)) >=0)
                     break;
                else
                {
#ifdef HP7
                memcpy(&ptybase[P_T_OFF - 1],&ptybase[P_T_OFF - 2],
                                sizeof(ptybase) - P_T_OFF + 1);
                ptybase[P_T_OFF - 2] = 'm';
#endif
                    ptybase[P_T_OFF]='p';
                    (void) close(x.ptyfd);
                    x.ptyfd = -1;
                }
            }
        }
    }
    ptybase[P_GRP_OFF]--;
    x.ttyname = ptybase;
#endif
#endif
#endif
#endif
#endif
#endif
    if (x.ptyfd < 0)
        return (struct pty_tty_pair *) 0;
    else
    {
        if ( ioctl(x.ptyfd,(TIOCPKT),(char *) &parm) < 0)
        {
            perror("Failed to set packet mode");
            pg.packet_mode = 0;
        }
        else
            pg.packet_mode = 1;
        return &x;
    }
}
#ifdef DEBUG
void  debug_catcher(sig, code)
int sig;
int code;
{
#ifdef DEBUG
        (void) fprintf(debug_log,"Unexpected signal %d, code %d, errno = %d\n",
                       sig,code,errno);
#endif
    return;
}
#endif
/**********************************************************************
 * Routine to initiate the command to execute; give it the tty, and the 
 * command
 */
void comstart(pty_tty,command)
struct pty_tty_pair * pty_tty;
char * command;
{
char * com_args[20];                 /* nasty hard coded argument limit */
int  i;
int ttyfd = pty_tty->ttyfd;
int ptyfd = pty_tty->ptyfd;
int pgrp;
/*
 * Detach ourselves from the terminal. This won't work if we are the only
 * process in our group; in this case, we must fork, execute in the child,
 * and exit the parent.
 */
    if (!isatty(0)) 
    {
        int t;
        void (*old_int) ANSIARGS((int));
        int pid = getpid();
        old_int = (void *) sigset(SIGINT,sig_zap);
#ifdef JOB_STUFF
#ifdef MANUAL_RESTORE
        if (tcgetattr(0,&save_tm) < 0)
        {
            perror("Getting Terminal Attributes");
        }
#endif
#endif
#ifdef MUST_SPAWN
        for (t = 3; t > 0 && setsid() != pid; t--)
        {
            int child_pid;
            if ((child_pid = fork()) > 0)
            {              /* Parent */
                pause();   /* Wait to be killed off */
                exit(1);   /* The zap should get it */
            }
            else if (child_pid)
            {              /* fork() failed */
                perror("setsid() fork failed");
                exit(pid);
            }
            else
            {              /* Child */
                while (kill (pid,SIGINT) >=0)
                {
                    sleep(1);
                }
                pid = getpid();
            }
        }
        if (t <= 0)
        {
            perror("Failed to void Parent Control Terminal");
            fprintf(stderr,"pid %d pgrp %d sid %d\n",
                    pid, getpgrp(),
#ifdef V4
#ifdef V32
                    -1
#else
#ifdef PTX
                    -1
#else
                    getsid(0)
#endif
#endif
#else
                    -1
#endif
                    );
            exit(1);
        }
        (void) sigset(SIGINT,old_int);
#else
#ifdef SUN
        if (setsid() != pid)
        {
            perror("Failed to void Parent Control Terminal");
#ifdef PATH_AT
            if (isatty(0))
            {
                endwin();
#ifdef OSF
                unset_curses_modes();
#endif
            }
#endif
            exit(1);
        }
#else
#ifdef BSD
        t = open("/dev/tty", O_RDWR) ;
        if (t > 0)
        {
            ioctl(t, TIOCNOTTY, 0) ; /* void tty association */
            close(t) ;
        }        
#else
#ifdef AIX
        t = open("/dev/tty", O_RDWR) ;
        if (t > 0)
        {
            ioctl(t, TIOCNOTTY, 0) ; /* void tty association */
            close(t) ;
        }        
#endif
#endif
#endif
#endif
    }
    if ((pg.child_pid =fork()) == -1)
    {
        perror("Fork failed");
#ifdef PATH_AT
        if (isatty(0))
        {
            endwin();
#ifdef OSF
            unset_curses_modes();
#endif
#endif
        }
        exit(1);
    }
    else
    if (pg.child_pid == 0)
    {                    /* Child */
/*
 * CHILD - Initialise environment
 *   -  Make sure Hangup kills the program and its descendants off
 *   -  Break the connexion between this process and any control terminal
 *      (by making it the session leader)
 *   -  Make the tty half of the pty/tty pair the control terminal
 *   -  Make this the Process Group for this terminal
 */ 
#ifdef DEBUG
    int i;

        if (debug_log == (FILE *) NULL)
        {
            debug_log = fopen("ptytty.log","w");
            setbuf(debug_log,NULL);
        }
        for (i = 1; i < 32; i++)
             sigset(i, debug_catcher);
#else
        (void) sigset(SIGHUP,SIG_DFL);   /* make sure that the children die */
        (void) sigset(SIGINT,SIG_DFL);
        (void) sigset(SIGQUIT,SIG_DFL);
        (void) sigset(SIGTERM,SIG_DFL);
#endif
        (void) close(ptyfd);
        if ( dup2(ttyfd,0) == -1)
        { /* set stdin file descriptor to terminal */
#ifdef DEBUG
             (void) fprintf(debug_log,
                     "Failed to duplicate ttyfd on 0: errno = %d\n",errno);
             (void) fclose(debug_log);
#endif
            exit(1);
        }
        (void) dup2(ttyfd,1);
        (void) dup2(ttyfd,2);
        (void) close(ttyfd);
        pg.child_pid = getpid();
#ifdef PYR
/*
 * If setpgrp() worked properly, none of the following would be necessary
 */
        ttyfd = open("/dev/tty", O_RDWR) ;
        if (ttyfd < 0 ||
             ioctl(ttyfd, TIOCNOTTY, 0)  == -1)
        {                                  /* void tty association */
#ifdef DEBUG
   (void) fprintf(debug_log,"Failed to void control terminal for child: errno = %d\n",errno);
        (void) fclose(debug_log);
#endif
            exit(1);
        }        
        (void) close(ttyfd) ;
/*
 * Establish the new control terminal
 */
        pgrp = setpgrp();                 /* set up a new session */
#else
        pgrp = setsid();                 /* set up a new session */
#endif
        if (pgrp != pg.child_pid)
        {
#ifdef DEBUG
   (void) fprintf(debug_log,"Failed to set up process group for child: errno = %d\n",errno);
        (void) fclose(debug_log);
#endif
            exit(1);
        }
        if ((ttyfd = open(pty_tty->ttyname, O_RDWR)) > -1)
        { /* set control terminal */
#ifdef JOB_STUFF
            struct termios t;
#endif
#ifndef V32
#ifdef SCO
            if (tcsetpgrp(ttyfd, pgrp) == -1)
#else
#ifdef HP7
            if (tcsetpgrp(ttyfd, pgrp) == -1)
#else
            if (ioctl(ttyfd, TIOCSPGRP, &pgrp) == -1)
#endif
#endif
            { /* set process group for terminal */
#ifdef DEBUG
                int sav_err = errno;
                (void) fprintf(debug_log,
"Failed to set up process group %d, fd %d, ttyname()=%s, terminal %s: errno = %d\n",
                   pgrp,ttyfd,ttyname(ttyfd),
                   pty_tty->ttyname,sav_err);
#ifndef OSF
                (void) fclose(debug_log);
#endif
#endif
#ifndef OSF
                exit(1);
#endif
            }
#ifdef DEBUG
            else
            {
                (void) fprintf(debug_log,
 "Set up process group %d, fd %d, ttyname()=%s, terminal %s: group = %d sid= %d\n",
                   pgrp,ttyfd,ttyname(ttyfd),
                   pty_tty->ttyname,tcgetpgrp(ttyfd), getsid(getpid()));
            }
#endif
#endif /* V32 */
#ifdef JOB_STUFF
            if (tcgetattr(ttyfd,&t) < 0)
            {
#ifdef DEBUG
                fprintf(debug_log,"Error %d Getting Terminal Attributes\n",
                        errno);
                fflush(debug_log);
#endif
            }
#ifdef MOD
            t.c_cflag |= CS8 ; /* 8 bit characters with no parity */
            t.c_cflag &= ~PARENB;
            t.c_cc[VINTR] = (cc_t) 3;
            t.c_cc[VQUIT] = (cc_t) 28;
            t.c_cc[VERASE] = (cc_t) 8;
            t.c_cc[VKILL] = (cc_t) 21;
            t.c_cc[VREPRINT] = (cc_t) 18;
#endif
            t.c_iflag = ICRNL | ISTRIP | IXOFF | IGNPAR;
            t.c_oflag = OPOST | ONLCR;
            t.c_lflag = ICANON | ISIG | ECHO |ECHOE | ECHOK | ECHONL;
            if (tcsetattr(ttyfd,TCSANOW,&t) < 0)
            {
#ifdef DEBUG
                fprintf(debug_log,"Error %d Setting Terminal Attributes\n",
                        errno);
                fflush(debug_log);
#endif
            }
            (void) close(ttyfd) ;
#endif
        }        
        else
        {
#ifdef DEBUG
            (void) fprintf(debug_log,
                           "Failed to set up control terminal %s: errno = %d\n",
                           pty_tty->ttyname,errno);
            (void) fclose(debug_log);
#endif
            exit(1);
        }
        if (command != (char *) NULL)
        {
/*
 * Process the arguments in the string that has been read
 */
            if ((com_args[0]=strtok(command," \t\n"))
                             != (char *) NULL)
/*
 * Generate an argument vector
 */
                for (i=1;
                         (com_args[i]=strtok(NULL," \t\n"))
                                      != (char *) NULL;
                            i++);
             else command = (char *) NULL;
        }
        if (command == (char *) NULL)
        {
               com_args[0] = getenv("SHELL");
               if (com_args[0] == (char *) NULL)
                   com_args[0] = "sh";
               com_args[1] = (char *) 0;
        }
#ifdef DEBUG
        (void) fprintf(debug_log,"Command argv[0] = %s\n",com_args[0]);
        (void) fclose(debug_log);
#endif
        if (execvp(com_args[0],com_args))
            exit(1);        /* Should not return */
    }
    else /* Parent */
    {
        (void) close(ttyfd);
        (void) sigset(SIGCHLD,sigchild);
    }
    return;
}
/****************************************************************************
 * Function to handle the pty input data. Called from ptydrive.c and
 * from pathdlib.c after screen refresh
 */
int ptyread(ptyfd)
int ptyfd;
{
char buf[BUFLEN];
int read_count;
short int j;
#ifdef DEBUG
    fprintf(stderr,"ptyread() called\n");
#endif
    if (pg.curr_event == (struct event_con *) NULL)
    {                        /* not interested */
flow_control:
        if ((read_count = read(ptyfd,buf,sizeof(buf)))<=0)
        {        /* Child has terminated */
#ifdef DEBUG
            perror("pty read returned nothing");
            (void) fprintf(stderr,"Error number %d\n",errno);
            (void) fprintf(stderr,"Characters read %d\n",read_count);
            statdump();  /* terminate with a status dump */
#endif
            return 0;
        }
        if ( pg.packet_mode )
        {
/*
 * If first byte isn't zero, we could have an OR of any of the following.
 * 
 * TIOCPKT_FLUSHREAD   whenever the read queue for the terminal is flushed.
 *
 * TIOCPKT_FLUSHWRITE  whenever the write queue for the terminal is flushed.
 *
 * TIOCPKT_STOP        whenever output to the terminal is stopped a la '^S'.
 *
 * TIOCPKT_START       whenever output to the terminal is restarted.
 *
 * TIOCPKT_DOSTOP      whenever t_stopc is '^S' and t_startc is '^Q'.
 *
 * TIOCPKT_NOSTOP      whenever the start and stop characters are not '^S/^Q'.
 *
 * Whilst in Packet Mode, the presence of control status information to be read
 * from the master side may be detected by a select(2) for exceptional
 * conditions.
 *
 * TIOCPKT_IOCTL       When this bit is set, the slave has changed the
 *                     termios(4) structure (TTY state), and the remainder of
 *                     the data read from the master side of the pty is a
 *                     copy of the new termios(4) structure.
 */
            if (buf[0] == 0x20)
                goto flow_control; /* Don't care; discard */
            if (pg.dumpin_flag == 2)
                logadd(IN_DIRECTION,&buf[1],read_count - 1);
        }
        else
        if (pg.dumpin_flag == 2)
             logadd(IN_DIRECTION,buf,read_count);
        if (pg.dumpout_flag)
        {
            if (pg.packet_mode)
            {
                if (read_count > 1)
                {
#ifdef NOSTDIO
                    (void) write(fileno(stderr),&buf[1],read_count - 1);
#else
                    (void) fwrite(&buf[1],sizeof( char ),read_count - 1,stderr);
#endif
                }
            }
            else
            {
#ifdef NOSTDIO
                (void) write(fileno(stderr),buf,read_count);
#else
                (void) fwrite(buf,sizeof( char ),read_count,stderr);
#endif
            }
        }
    }
    else if (pg.curr_event->first_word == (struct word_con *) NULL)
    {                                  /* nothing to look for */
    int think_left;

        event_record(pg.curr_event->event_id, pg.curr_event);
/*
 * Sleep the rest of the think time
 */
        think_left = (int) (pg.curr_event->min_delay
                             - pg.curr_event->time_int/100.0);
        pg.sav_write_mask = pg.term_check;     /* wake up once more */
        if (think_left > 0)
        {                                /* sleep time in progress */
            (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
            pg.alarm_flag = 0;
#endif
            alarm(think_left);           /* wait some more */
            pg.write_mask = 0;
        }
        else pg.write_mask = pg.term_check;
        pg.curr_event = (struct event_con *) NULL;
    }
    else
    {
    struct word_con * curr_word;
    struct word_con * disc;

        if ((j = bufread(ptyfd,PTYIN,pg.dumpout_flag)) <= 0)
        {
#ifdef DEBUG
            perror("End of File Looking for a match");
            (void) fprintf(stderr,"Error number %d\n",errno);
            statdump();  /* terminate with a status dump */
#endif
            event_record("Z",pg.curr_event);
            return 0;             /* Slave process has exited */
        }
        else
        if (((pg.force_flag & FORCE_DUE) &&
              match_force(PTYIN,pg.curr_event,&curr_word,&disc))
         ||(lanalyse(PTYIN,pg.curr_event,&curr_word,&disc)))
        {                      /* Matched */
        short int * x;
        struct circ_buf cb1,cb2;
        int think_left;
        short int to_do;

            pg.curr_event->word_found = curr_word; 
#ifdef DEBUG
            statdump();
#endif
            event_record(pg.curr_event->event_id, pg.curr_event);
            pg.force_flag = 0;
            if (*(curr_word->action) != 0)
            {
#ifdef SBC
            if (!strcmp(pg.curr_event->event_id,"N1"))
                do_sbspec();
            else
            {
#endif

/*
 * Now have to tidy up afterwards
 * - inject the action string into circular buffer
 * - sleep the rest of the think time
 */
#ifdef ONE_STRING
            for (x = curr_word->action, to_do = 0; *x != 0; to_do++, x++)
                bufuntake(PTYOUT);
            bufuntake(PTYCHAR);
            cb1 = *(PTYCHAR);
            cb2 = *(PTYOUT);
            cb1.head=cb1.tail;
            cb2.head=cb2.tail;
            for (x = curr_word->action; *x != 0; x++)
            {
                bufadd(&cb2,*x);
            }
            bufadd(&cb1,to_do);
#else
            for (x = curr_word->action; *x != 0; x++)
            {
                bufuntake(PTYCHAR);
                bufuntake(PTYOUT);
            }
            cb1 = *(PTYCHAR);
            cb2 = *(PTYOUT);
            cb1.head=cb1.tail;
            cb2.head=cb2.tail;
            for (x = curr_word->action; *x != 0; x++)
            {
                bufadd(&cb1,1);
                bufadd(&cb2,*x);
            }
#endif
#ifdef SBC
            }
#endif
            }
            think_left = (int) (pg.curr_event->min_delay -
                                pg.curr_event->time_int/100.0);
            pg.sav_write_mask = pg.term_check;  /* wake up once more */
            if (think_left > 0)
            {                             /* sleep time in progress */
                (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
                pg.alarm_flag = 0;
#endif
                alarm(think_left);        /* wait some more */
                pg.write_mask = 0;
            }
            else pg.write_mask = pg.term_check;
            pg.curr_event = (struct event_con *) NULL;
        }
    }
    return 1;
}
#ifdef NEED_STREAMS
#ifdef DEBUG
/*************************************************
 * Dump off the streams modules loaded. Only used to
 * find out what is there, during set up
 */
static void strmdump(fd)
int fd;
{
struct str_mlist str_names[20];
struct str_list s_feed;
int i;

    memset((char *) &str_names[0],0,sizeof(str_names));
    s_feed.sl_nmods = sizeof(str_names)/sizeof(struct str_mlist);
    s_feed.sl_modlist = str_names;
    for ( i=ioctl(fd,I_LIST,&s_feed),
          ((i>-1)?(i=s_feed.sl_nmods):(-1)), i--; i > -1; i--)
    {
         fputs(str_names[i].l_name, debug_log);
         fputc('\n', debug_log);
    }
    return;
}
#endif
#endif
