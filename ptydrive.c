/*
 *    ptydrive.c - Program to drive and collect timings through a pty device.
 *
 *    Copyright (C) E2 Systems 1993
 *
 * This program is the heart of the Mark 2 E2 Systems Test Harness. It
 * combines the separate functionality of the original lanalyse and eanalyse
 * programs, with the difference that:
 *   -  it drives a terminal device (pty/tty) rather than named pipes
 *   -  it multi-threads the feeding and the take-off processing
 *   -  the input definition is terminal independent rather than ORACLE
 *      forms specific
 *   -  the pattern definition is in terms of dynamic strings with limited
 *      wild-card capabilities rather than fixed strings.
 *   -  The program takes its instructions from the input stream with respect
 *      to:
 *      - Typing rates (the R command)
 *      - Sleeps (the W command)
 *      - Strings to look for (the S command)
 *      - Timestamps (the T command)
 *
 *    The program:
 *         -  gets a  pty/tty pair
 *         -  fork()s and exec()s to get the command running
 *
 *      It then loops between the stdin, the pty and standard output.
 *
 *      Timestamps are written when the appropriate events are spotted.
 *
 * Arguments
 * =========
 *   - arg 1 = name of file to output timestamps to
 *   - arg 2 = pid of fdriver
 *   - arg 3 = pid of bundle
 *   - arg 4 = i number within 'rope'
 *
 * Flag Options
 * ============
 * -d           - log output from the driven terminal to stderr
 * -f           - Enable forced event handling.
 * -x command   - execute this command rather than spawn a shell
 * -i event     - search for this event on startup
 * -p           - log driven terminal input to stdout
 * -t           - log driven terminal input to stdout in terminal-independent
 *                format
 * -s           - see through mode; no terminal-independent processing
 * -e           - Escape comment mode; treat input char as comment entry
 * -r           - redraw string (PATH/AT only)
 *
 * Signal handling
 * ===============
 * SIGTERM - terminate request
 * SIGBUS  - should not happen (evidence of machine stress)
 * SIGALRM - used to control typing rate
 * SIGCHLD - watching for death of process
 * SIGINT  - Toggle between See Through and Terminal Independent input modes
 * SIGQUIT - Force an event after the next keystroke
 *
 */
static char * sccs_id="@(#) $Name$ $Id$\n\
Copyright (C) E2 Systems Limited 1993";
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#ifdef SEQ
#include <fcntl.h>
#include <time.h>
#else
#ifdef ULTRIX
#include <fcntl.h>
#else
#ifdef AIX
#include <fcntl.h>
#else
#ifdef ANDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif
#endif
#endif
#ifdef SCO
#include <sys/termio.h>
#endif

#ifdef TERMCAP
char termcapbuf[1024];              /* because termcap rather than
                                       terminfo */
#endif
#include <signal.h>
#include <errno.h>
#include "circlib.h"
#include "matchlib.h"
#include "ptytty.h"

static void event_begin();
#ifdef PATH_AT
static int adjust_mask();
static int user_intervene();
#endif
#undef select
/*******************
 * Global data
 */
struct ptydrive_glob pg;
/*
 * Apparent Loop Detection
 */
#define MAX_LOOPS 50
#ifdef SIGUSR2
void catch_usr2()
{
    if (pg.dumpout_flag)
        pg.dumpout_flag = 0;
    else
        pg.dumpout_flag = 1;
    return;
}
#endif

/***********************************************************************
 * Main program starts here
 * VVVVVVVVVVVVVVVVVVVVVVVV
 */
int main (argc,argv)
    int argc;
    char* argv[];
{
char * ttn;         /* Terminal to use for User Dialogue */
struct pty_tty_pair * gotpty;
short int bufleft;
char * read_ptr;
char * start_event;
struct timeval long_stop;
char * force_time;
int loop_detect;
#ifdef SCO
long readymask, writeymask, excymask;
#else
#ifdef LINUX
long readymask, writeymask, excymask;
#else
int readymask, writeymask, excymask;
#endif
#endif
char * term;
int testmask, ptyfd, ttyfd, read_count;
int ch;
char c;
char * com_ex;
FILE * tinfile;
FILE * toutfile;
static char * rdr_str = REDRAW_STRING;
short int i,j;
/*
 * Allocate the circular buffers
 */
    for (i = 0; i < BUF_CNT; i++)
        pg.circ_buf[i] = cre_circ_buf();
/*
 * Set up the hash table for events
 */
    pg.poss_events = hash(MAX_EVENT,long_hh,icomp);

    pg.cps = PATH_CPS;             /* Default typing rate is 10 cps */
    
/****************************************************
 *    Initialise
 */
    setbuf(stderr,NULL);
    pg.curr_event = (struct event_con *) NULL;
    pg.abort_event = (struct event_con *) NULL;
    com_ex = (char *) 0;
    pg.break_flag=0;
    pg.dumpin_flag=0;
    pg.dumpout_flag=0;
    pg.see_through=0;
    pg.esc_comm_flag=0;
#ifdef PATH_AT
    pg.next_auto=0;
    pg.force_flag= 0;
    force_time = (char *) NULL;
    pg.auto_event = (struct event_con *) NULL;
#endif
    if ((gotpty = getpty()) == (struct pty_tty_pair *) 0)
    {
        (void) fprintf(stderr,"Cannot get a system test tty\n");
        exit(1);
    }
    if (((ch =  dup(0)) < 0) 
      || ((pg.script_file = fdopen(ch,"r")) == (FILE *) NULL))
    {
        perror("Attempt to switch input descriptor failed");
        sigterm();
    }
/*
 * The following code is for the benefit of AIX, which doesn't provide
 * newterm().
 */
    if (((ch =  dup(1)) < 0) 
      || ((pg.log_output = fdopen(ch,"w")) == (FILE *) NULL))
    {
        perror("Attempt to switch output descriptor failed");
        sigterm();
    }
    start_event = (char *) NULL;
    ttn = (char *) NULL;
    pg.frag_size = 4096;
    while ( ( ch = getopt ( argc, argv, "f:x:hdpse:tr:i:bu:g:" ) ) != EOF )
    {
        switch ( ch )
        {
                case 'd' :
                    pg.dumpout_flag=1;
                    break;
 
                case 'g' :
                    pg.frag_size = atoi(optarg);
                    if (pg.frag_size < 1 || pg.frag_size > BUFLEN)
                        pg.frag_size = 4096;
                    break;
 
                case 'e' :
                    pg.esc_comm_flag = 1;
#ifdef PATH_AT
                    pg.cur_esc_char = (short int) *optarg;
#else
                    pg.cur_esc_char = (short int) '#';
#endif
                    break;
 
                case 'p' :
                    pg.dumpin_flag=1;
                    break;
 
                case 't' :
                    pg.dumpin_flag = 2;
                    break;

#ifdef PATH_AT 
                case 'f' :
                    force_time = optarg;
                    break;
 
                case 'r' :
                    rdr_str = optarg;
                    break;

                case 'b' :
                    pg.break_flag = 1;
                    break;
#endif 
                case 's' :
                    pg.see_through = 1;
                    break;
 
                case 'i' :
                    start_event = optarg;
                    break;
 
                case 'u' :
                    ttn = optarg;
                    break;

                case 'x' :
                    com_ex = optarg;
                    break;
 
                case 'h' :
                    (void) printf("ptydrive: pseudo-keyboard driver\n\
  You can specify:\n\
  -x to initiate a particular command\n\
  -i to declare an initial event command\n\
  -d to catch the command output on stderr\n\
  -f timeout to enable forced events\n\
  -e escape character to put comments on stdout, and not pass them through\n\
  -p to catch the command input on stdout\n\
  -t to catch the command input on stdout in terminal-independent format\n\
  -s to start in See through rather than terminal independent mode\n\
  -r to specify the screen refresh string\n\
  -u to use a specific terminal device for the user dialogue\n\
  -b to specify break mode, when it will pause after each function key\n\
 and -h to get this message.\n\
\n");
                    exit(0);
                default:
                case '?' : /* Default - invalid opt.*/
                       (void) fprintf(stderr,"Invalid argument; try -h\n");
                       exit(1);
                    break;
        }
    }
    term = getenv("TERM");
#ifdef TERMCAP
    (void) tgetent(termcapbuf,term);       /* read in the termcap database
                                            * for this terminal
                                            */
#else
#ifdef PATH_AT
    if (pg.dumpin_flag == 2)
    {
        if (ttn == (char *) NULL)
            ttn = "/dev/tty";
/*
 * The following is because various curses implementations variously insist
 * on file descriptor 0 for input and descriptor 1 for output. Note that
 * the ICL DRS-6000 doesn't like the /dev/tty file descriptors later on.
 */
        toutfile = fopen(ttn,"w");
        tinfile = fopen(ttn,"r");
        dup2(fileno(tinfile),0);
        (void) fclose(tinfile);
        dup2(fileno(toutfile),1);
        (void) fclose(toutfile);
        tinfile = fdopen(0,"r");
        toutfile = fdopen(1,"w");
        pair_track_ini();        /* Initialise buffers for message pair
                                     history */
        dia_init(rdr_str, gotpty, term, tinfile, toutfile);
    }
    else
#endif
        (void) setupterm(term,1,0);  /* read in the terminfo database
                                            * for this terminal
                                            */
#endif
    pg.term_event = termload();             /* read in the terminal database */
/*
 * Get the script from a different file descriptor, so can use
 * stdin for curses access if PATH_AT
 */
    if (!pg.see_through)
    {
        pg.cur_in_file = pg.script_file;
#ifdef PATH_AT
        pg.cur_dia = fix_man;
        pg.int_flag = 0;
#endif
    }    
    else
    {
        pg.cur_in_file = stdin;
#ifdef PATH_AT
        pg.int_flag = 0;
        pg.cur_dia = dia_man;
#endif
    }
    comstart(gotpty,com_ex);
    pg.seqX = 0;                              /* timestamp sequencer       */
    if (argc < (optind + 4))
    {
        fprintf(stderr,"Insufficient Arguments Supplied\n");
#ifdef PATH_AT
        if (isatty(0) && pg.dumpin_flag == 2)
        {
            endwin();
#ifdef OSF
            unset_curses_modes();
#endif
        }
#endif
        exit(1);
    } 
    pg.logfile=argv[optind];                  /*                           */
    pg.fdriver_seq=argv[optind+1];            /* Details needed by event   */
    pg.bundle_seq=argv[optind+2];             /* recording                 */
    pg.rope_seq=argv[optind+3];               /*                           */

    event_record("S", (struct event_con *) NULL); /* announce the start */
#ifdef DEBUG
void debug_catcher();
    for (i = 1; i < 32; i++)
        sigset(i, debug_catcher);
#endif
    (void) sigset(SIGBUS,sigbus);
                            /* Initialise the bus error signal catcher */
#ifdef SIGUSR2
    (void) sigset(SIGUSR2,catch_usr2);
                            /* Initialise debug toggle */
#endif
#ifdef PATH_AT
    (void) sigset(SIGINT,sigint);
                            /* Initialise the interrupt signal catcher */
#else
    (void) sigset(SIGINT,SIG_IGN);
#endif
    (void) sigset(SIGTERM,sigterm);
                            /* Initialise the termination signal catcher */
#ifndef V32
     (void) sigset(SIGTTOU,SIG_IGN);
                             /* Ignore silly stops */
     (void) sigset(SIGTTIN,SIG_IGN);
                             /* Ignore silly stops */
     (void) sigset(SIGTSTP,SIG_IGN);
                             /* Ignore silly stops */
#endif
     (void) sigset(SIGHUP,sigterm);
                             /* Treat hangups as instructions to go away */
     (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
     pg.alarm_flag = 0;
#endif

/*******************************************************************
 * Variables used to control main loop processing
 */ 
    pg.match_out = match_out;
    pg.match_comp = match_comp;
    pg.in_check = (1 << fileno(pg.cur_in_file)); /* select bit for script */
    ptyfd = gotpty->ptyfd;
    pg.term_check =  (1<<ptyfd);                 /* select bit for pty */
    pg.sav_write_mask = pg.term_check;  /* mask to restore after alarm expiry */
    pg.write_mask = pg.term_check;      /* mask for writes (ie. the pty) */
    testmask =  pg.term_check | pg.in_check; /* mask for reads (stdin+pty) */
    pg.think_time = PATH_THINK;           /* default think time */
    long_stop.tv_sec = PATH_TIMEOUT;      /* give up after 20 minutes */
    long_stop.tv_usec = 0;   /* give up after 20 minutes */
#ifdef PATH_AT
    if ( force_time != (char *) NULL)
        ini_force(atoi(force_time));
#endif
/*******************************************************************
 * If there is a startup event, process it
 */
    if (start_event != (char *) NULL)
    {
        if ((pg.curr_event = stamp_declare(start_event))
             != (struct event_con *) NULL)
        {
            if (pg.dumpin_flag == 2)
            {
                fprintf(pg.log_output,"\\T%s:\\\n",pg.curr_event->event_id);
                fflush(pg.log_output);
            }
            event_begin();
        }
    }
/*******************************************************************
 *  Main Loop;
 *  - wait
 *  - feed information from stdin to the pty
 *  - at the output.
 *
 *  Terminate when signal arrives (termination request SIGTERM or child death)
 *
 *  - This is the heart of the program, and the interesting bit.
 *  - It works as follows
 *
 *  PROPER DESIGN FOR THE PSEUDO-KEYBOARD MANAGER
 *  =============================================
 *  - There are two possible ways of working:
 *    - Cyclically, where an input event must be followed by an output
 *      event, and the next input is not scheduled until the output has
 *      been seen; the usual way
 *    - With type-ahead, where input events can be fed in at the operator-
 *      defined rate; needed if there are buffers that delay matters, and
 *      which need to be filled before anything gets through.
 *    - The program handles both these circumstances; the
 *      difference needs to be seen in the way that the response is defined:
 *      - In the former case, it is from the input event to the output event
 *      - In the latter, it is the time between successive output events.
 *  - The following co-routines need to be scheduled.
 *
 *  - KEYSTROKE INPUT PRE-PROCESSING
 *    ==============================
 *    -  Stdin receives keystrokes defined in a terminal-independent,
 *       terminfo fashion
 *    -  The read on stdin should break it up into keystroke tokens
 *    -  The tokens should be compared with the list of input tokens to
 *       be time-stamped.
 *    -  The appropriate terminfo routines should be called to look up any
 *       keyboard equivalents.
 *    -  The characters are then transferred to an input/output circular buffer.
 *    -  In parallel, entries are made in a circular buffer indicating:
 *       -  the number of characters per keystroke
 *       -  the time that must have elapsed since the last output
 *       -  whether or not the output event to the pty need be time-stamped. 
 *
 *  - INPUT/OUTPUT PROCESSING CO-ORDINATION
 *    =====================================
 *    -  This code must:
 *       - Be non-blocking
 *       - Schedule the other co-processing
 *    -  The following need to be tracked:
 *       - The time intervals that must elapse before another keystroke can be
 *         sent
 *       - Whether the pty is ready to be written to
 *       - Whether the pty is ready to be read from
 *       - Whether some number of the interesting patterns in the output (eg.
 *         'Transaction Completed') are not at all, partially, or fully matched.
 *
 *  - WRITING TO THE PSEUDO-KEYBOARD
 *    ==============================
 *    - When the time interval has expired, and the pseudo-keyboard is ready
 *      to accept program output (as indicated by a select()), all 
 *      keystrokes' worth of characters whose expiry time has arrived can be
 *      sent:
 *      - write the characters
 *      - down-date the number still to be sent for this keystroke
 *      - look out for wrap-round on the circular buffer; only do up to
 *        the boundary, if the head is less than the tail
 *      - Note that:
 *        - The write() may not send all the characters
 *        - The readiness for write may change after the characters
 *          have been sent.
 *
 *  - READING FROM THE PSEUDO-KEYBOARD
 *    ==============================
 *    - This is the highest priority task
 *    - Characters need to be fetched into a circular buffer. Note that this is
 *      a different buffer to those associated with stdin processing
 *    - The regular expression processing needs to be triggered.
 *    - If the pseudo-keyboard has characters ready for reading (as indicated
 *      by a select())
 *      - read the characters
 *      - look out for wrap-round on the circular buffer; only do up to
 *        the boundary, if the head is less than the tail
 *      - otherwise keep reading until it is not ready
 *      - Loop through the regular expressions that need to be processed
 *      - Adjust the circular tail to match the back marker for any active
 *        matches.
 *
 *  - SLAVE PROCESS OUTPUT POST-PROCESSING
 *    ====================================
 *    -  The match code had to be hacked:
 *       - Look out for wrap-round in a circular buffer; only do up to an edge
 *         if the head is less than the tail
 *       - Must allow match processing to be restarted at a point at which an
 *         extra number of characters has been read.
 *
 *   - WHAT IS ACTUALLY HERE
 *     =====================
 *     - High and Low water marks are implemented on the buffers in order to
 *       avoid buffer over-run.
 *
 */
    for(i=0, loop_detect = MAX_LOOPS;
             loop_detect >= 0 &&
             pg.child_pid != 0
#ifdef AGGRESSIVE_EXIT
          &&
             (pg.in_check !=0 || (pg.curr_event != (struct event_con *) NULL))
              || (PTYCHAR->buf_cnt > 0)
#endif
                          ;
                 loop_detect--)
    {
        int retcode;
        struct timeval * timeout;
#ifdef PATH_AT
/*
 * User has attempted to force matters.
 */
        if (pg.force_flag)
        {
            if (pg.next_auto == 0) /* Force not enabled */
            {
                pg.int_flag = 1;   /* Treat like an interrupt */
                pg.force_flag = 0;
            }
            else
            {
/*
 * If there is a current open event, force close it as if it had succeeded.
 */
                if (pg.curr_event != (struct event_con *) NULL)
                {
                    if (pg.force_flag & FORCE_DUE)
                    {
                        struct word_con * curr_word;
                        struct word_con * disc;
                        if ( match_force(PTYIN,pg.curr_event,&curr_word,&disc))
                        {                      /* Matched */
                            pg.curr_event->word_found = curr_word; 
                            event_record(pg.curr_event->event_id,
                                          pg.curr_event);
                            pg.sav_write_mask = pg.term_check;
                                           /* wake up once more */
                            pg.write_mask = pg.term_check;
                            pg.curr_event = (struct event_con *) NULL;
                        }
                        pg.force_flag = 0;
                    }
                }
                else
                {
                    short int x;
                    static struct timeval tpoll;   /* Guaranteed 0 */
/*
 * Turn our auto event into its next incarnation, and get it launched.
 */
                    (void) sprintf(pg.auto_event->event_id,
                                    "%X",0xa0 + pg.next_auto++);
                    pg.curr_event = pg.auto_event;
                    if ((pg.dumpin_flag == 2))
                    {
                        log_inject("Event"); /* Flush out any accumulated
                                                 characters */
                        fprintf(pg.log_output,
                                  "\\T%s:\\\n",pg.curr_event->event_id);
                        fflush(pg.log_output);
                    }
/*
 * We are probably in see-through mode.
 * Pull in all the components of the next keystroke
 */
                    do
                    {
                        (void) ttlex();
                        readymask = 1 << fileno(pg.cur_in_file);
                    }
                    while ((select(20,&readymask,0,0,&tpoll)) > 0);
                    while (PTYCHAR->buf_cnt > 0)
                    {
                        (void) buftake(PTYCHAR,&x);
                        (void)bufwrite(ptyfd,PTYOUT,(int)x,pg.dumpin_flag);
                                                   /* Get the next character */
                                                   /* Send it */
                    }
                    event_begin();
                    loop_detect = MAX_LOOPS;
                    pg.force_flag = FORCE_REQUEST;   /*
                                                       * This is what it is,
                                                       * unless the user has
                                                       * messed about with the
                                                       * QUIT key
                                                       */
                }
            }
        }
/*
 * User interrupted something boring
 */
        if (pg.int_flag)
        {
            testmask =user_intervene(testmask);
                                 /* Pass control to user; might not return */
            loop_detect = MAX_LOOPS;
        }
#endif
        readymask = testmask;
        if (PTYCHAR->buf_cnt > 0)
            writeymask = pg.write_mask;
        else
            writeymask = 0;
#ifdef DEBUG
        (void) fprintf(pg.log_output,
     "# \nfileno(pg.cur_in_file)= %d pg.term_check=%d pg.in_check=%d\n\
testmask=%d writeymask=%d\n #",
              fileno(pg.cur_in_file), pg.term_check, pg.in_check, testmask,
               writeymask);
        (void) fflush(pg.log_output);
#endif
        timeout = (pg.curr_event == (struct event_con *) NULL)? &long_stop 
                                  : &(pg.curr_event->timeout);
#ifdef DEBUG
        if ((testmask & (1 << fileno(pg.cur_in_file))) && (!pg.in_check))
             continue;   /* Child has died */
#endif
        excymask = readymask;
        long_stop.tv_sec = PATH_TIMEOUT; 
#ifdef AGGRESSIVE_EXIT
        if ((retcode=select(20,&readymask,&writeymask, &excymask,timeout)) <= 0)
#else
        if ((retcode=select(20,&readymask,&writeymask, &excymask,NULL)) <= 0)
#endif
        {
            if (retcode == 0)
            {                           /* timed out */
                if (pg.curr_event == (struct event_con *) NULL)
                    continue;    /* Long Stop Keyboard Timeout */
#ifndef PATH_AT
                event_record("Z", pg.curr_event);
                sigterm();                /* shut down */
#else
/*
 * If we are in force match (auto) mode, record the so-far as a match
 */
                if (pg.curr_event == pg.auto_event)
                {
                    pg.force_flag |= FORCE_DUE;
                }
                if ( pg.force_flag )
                    continue;
                testmask =user_intervene(testmask);
                                           /* Pass control to user. This
                                              might not return */
                loop_detect = MAX_LOOPS;
                continue;
#endif
            }
            else  if (errno != EINTR)
            {
                perror("Unexpected Select Error");
                (void) fprintf(pg.log_output,"# Error number %d\n",errno);
                (void) fprintf(pg.log_output,"Before Read mask %d\n",testmask);
                (void) fprintf(pg.log_output,"Before Write mask %d#\n",pg.write_mask);
                sigterm();  /* terminate with a status dump */
            }
            else
                continue;
        }
#ifdef DEBUG
        (void) fprintf(pg.log_output,
             "# pg.term_check=%d pg.in_check=%d readymask=%d writeymask=%d #\n",
              pg.term_check, pg.in_check, readymask, writeymask);
        (void) fflush(pg.log_output);
#endif
        if ((readymask & pg.term_check) || (excymask & pg.term_check))
                                      /* Something on the system test */
        {
            if (!ptyread(ptyfd))
                break;               /* End of File on pty */
            else
                loop_detect = MAX_LOOPS;
        }
        if ((readymask & pg.in_check))
        {                            /* Something on stdin */
            if (!ttlex())            /* EOF */
            {
#ifdef PATH_AT
                testmask = adjust_mask(testmask);
#endif
                testmask &=(~pg.in_check);
                pg.in_check = 0;
                continue;
            }
            else
            {
                loop_detect = MAX_LOOPS;
#ifdef PATH_AT
                testmask = adjust_mask(testmask);
#endif
            }
            if (PTYOUT->buf_cnt > PTYOUT->buf_high)
                testmask &= (~pg.in_check);
        }
        else
        if ((excymask & pg.in_check))
        {
            testmask &= ~pg.in_check;
            pg.in_check = 0;
        }

        else if (!(testmask & pg.in_check)
               && (PTYOUT->buf_cnt < PTYOUT->buf_low))
            testmask |= pg.in_check;
        if (writeymask && (PTYCHAR->buf_cnt > 0))
        {
            short int x;
            loop_detect = MAX_LOOPS;
            buftake(PTYCHAR,&x);      /* Get the next command */
            if (x > 0)                /* characters to send */
            {
                int f;
                if (*(PTYOUT->tail) < 32)
                    f = 1;           /* This is a function key */
                else
                    f= 0;
                if (!pg.child_pid)
                    continue;      /* In case it has died in the meantime */
#ifdef CHOP_UP
                if ((int) x > 10)
                {
                    (void) bufuntake(PTYCHAR);
                    *(PTYCHAR->tail) -= 10;
                    (void) bufwrite(ptyfd,PTYOUT,10,pg.dumpin_flag);
                }
                else
#endif
                {
                    (void) bufwrite(ptyfd,PTYOUT,(int) x,pg.dumpin_flag);
                                                                /* send it */
                    if ((pg.dumpin_flag == 2)
                      && (pg.see_through == 0)
                      && pg.break_flag
                      && f)
                    {                     /* In break mode */ 
                        i = 0;
                        (void) read(0,&f,1); /* Await a prod from the user */
                    }
                    else
                    {
                        i++;
                        if ( pg.see_through == 0 && i >= pg.cps) 
                        {
                            pg.write_mask = 0;  /* switch off write until
                                                    the alarm goes off */
                            (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
                            pg.alarm_flag = 0;
#endif
                            alarm(1);
                            i = 0;
                        }      /* Typing rate sleep */
                    }      /* Typing rate sleep */
                }
            }
            else if (x == 0)         /* Take a Timestamp */
            {
            HIPT * h;
            short int event_id;
            short int j,k,l;

                l=0;
                buftake(PTYOUT,&event_id);  /* Identifying character */
                if ((h = lookup(pg.poss_events,(long) event_id)) ==
                       (HIPT *) NULL)
                {
                    (void) fprintf(stderr,
                                    "Error: undefined event %x (%*.*s)\n",
                                    event_id,
                                    sizeof(event_id),sizeof(event_id),
                                    (char *) &event_id);
                    continue;       /* Crash out here */
                }
/*
 * If we are talking to a real terminal, if there is no next command waiting
 * we will have to get another command when the user types.  So we do not
 * disable the input stream. pg.see_through is usually only set in this case.
 *
 * If we are reading from a file, the command will already be there if the
 * ttlex() call fails. So in that case, we disable the input stream regardless
 */
                if (!ttlex())     /* Make sure we have another command */
                {
                    testmask &=(~pg.in_check);
                    pg.in_check = 0;
                }
                pg.curr_event = (struct event_con *) (h->body);
#ifdef PATH_AT
                if ((pg.curr_event  != (struct event_con *) NULL)
                 && (pg.dumpin_flag == 2))
                {
                    log_inject("Event"); /* Flush out any accumulated
                                             characters */
                    fprintf(pg.log_output,
                              "\\T%s:\\\n",pg.curr_event->event_id);
                    fflush(pg.log_output);
                }
#endif
                (void) buftake(PTYCHAR,&x);      /* Get the next command */
                (void) bufwrite(ptyfd,PTYOUT,(int) x,pg.dumpin_flag);
                                                                /* send it */
                if (pg.curr_event  != (struct event_con *) NULL)
                    event_begin();
            }
            else if (x > -15)        /* New Typing Rate */
            {
                pg.cps = -x;
                if (pg.dumpin_flag == 2)
                {
                    fprintf(pg.log_output,"\\R%d\\\n",pg.cps);
                    fflush(pg.log_output);
                }
            }
            else /* x < -14, time to allow for thinking */
            {
                pg.think_time = -x -15;   /* Also, pause for a time */
                pg.write_mask = 0;         /* switch off write until the alarm
                                                goes off */
                (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
                pg.alarm_flag = 0;
#endif
                alarm(pg.think_time);
                i = 0;
                if (pg.dumpin_flag == 2)
                {
                    fprintf(pg.log_output,"\\W%d\\\n",pg.think_time);
                    fflush(pg.log_output);
                }
            }
        }
    }                                    /* End of Main Loop */
#ifdef PATH_AT
    if (pg.dumpin_flag == 2)
        log_inject(" Finished ");
#endif
    if (loop_detect <= 0)
    {
        (void) fprintf(stderr,
               "Possible problem; Apparent Loop Detected; Terminating\n");
    }
    (void) fflush(stderr);
    (void) fflush(stdout);
    if (pg.child_pid)
    {                                    /* catch-up */
#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif
        (void) sigset(SIGCLD,SIG_DFL);
        (void) sigset(SIGALRM,alarm_catch);
#ifdef DEBUG_FULL
        pg.alarm_flag = 0;
#endif
        alarm(1);
        (void) ptyread(ptyfd);
        (void) kill(pg.child_pid,SIGHUP);    /* HANGUP to the child if
                                              * still there */
    }
    if (isatty(0) && pg.dumpin_flag == 2)
    {
#ifdef PATH_AT
        endwin();
#ifdef OSF
        unset_curses_modes();
#endif
#endif
    }
#ifdef MANUAL_RESTORE
    if (isatty(0))
        restore_tty(0);                      /* Make sure terminal settings
                                                are O.K.
                                              */
#endif
    event_record("F", (struct event_con *) NULL);
    _exit(0);                                 /* scarper */
}    /* End of Main program */
/*************************************************************************
 * Adjust the control parameters for event searching
 */
static void event_begin()
{
    if (pg.curr_event->first_word != (struct word_con *) NULL)
    {          /* start looking for this string */
        pg.curr_event->time_int = timestamp();
                               /* record the time */
        pg.sav_write_mask=0;   /* Switch off write until seen;
                                * No typeahead
                                */
        pg.write_mask=0;
        pg.curr_event->word_found = (struct word_con *) NULL;
        pg.curr_event->min_delay = pg.think_time;
        PTYIN->buf_cnt=0;   /* Initialise the scan buffer */
        PTYIN->head=PTYIN->base;  
        PTYIN->tail=PTYIN->base;
        match_set_up(PTYIN,pg.curr_event);
    }
    else
    {
        event_record(pg.curr_event->event_id,
                    (struct event_con *) NULL);
        pg.curr_event = (struct event_con *) NULL;
    }
}
#ifdef PATH_AT
/*
 * Adjust the mask if the in_file has switched
 */
static int adjust_mask(old_mask)
int old_mask;
{
register int s = (1 << fileno(pg.script_file));
register int i = (1 << fileno(stdin));
register int o = old_mask;
register int c = pg.in_check;
/*
 * Make sure the old mask is valid
 */
    if (o != (o & (s|i|c|pg.term_check)))
        o =  c | (o & (s|i|pg.term_check));
    if (i != s && i == c && pg.curr_event == (struct event_con *) NULL)
    {
        pg.write_mask = pg.term_check;
        pg.sav_write_mask = pg.term_check;
    }
    if (c == 0)
        o &= ~( i | s);
    else
    if ((c & i) && (o & s))
        o &= ~ s;
    else
    if ((c & s) && (o & i))
        o &= ~ i;
    if (!(c & o))
        o |= c;
    return o;
}
/*
 * Tidy up after a user intervention (interrupt, quit) or timeout
 *
 * If we have the system test variant, and this is a failure
 * pop up the dialogue
 * 
 * The objectives are:
 * -  Hand over to the human user
 * -  Allow him or her to intervene
 * -  Let the script continue
 *    So, need to switch to:
 *    -   Screen input, saving the current position in the output
 *    -   See through mode
 *    -   From the screen
 * -  Then switch back to:
 *    -   File input, from where we left off.
 * 
 * This is done in the optional file pathdlib.c
 * Adding extra dialogues, and allowing a user to pop up them up
 * under other circumstance, would be easy.
 */ 
static int user_intervene(testmask)
int testmask;
{
    alarm(0);                         /* Cancel any alarm call */
    pg.int_flag = 0;                  /* Cancel the interrupt flag */
    pg.force_flag = 0;                /* Cancel the force flag */
    if (pg.curr_event != (struct event_con *) NULL)
        event_record("Z", pg.curr_event);
                                      /* Record the event as failed */
    pg.abort_event = pg.curr_event;
    pg.curr_event = (struct event_con *) NULL;
    if (pg.dumpin_flag == 2)
    {
        (*pg.cur_dia)();
        do_redraw();
        testmask = adjust_mask(testmask);
    }
    else
        sigterm();
    return testmask;
}
#endif
