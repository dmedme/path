/*
 *    ptytty.h - Header file for pty/tty common stuff.
 * @(#) $Name$ $Id$
 *
 *    Copyright (C) E2 Systems Limited 1993
 *
 */
#ifndef PTYTTY_H
#define PTYTTY_H
extern char * ttyname();

struct pty_tty_pair {
   int ptyfd;
   int ttyfd;
   char * ttyname;
};
struct pty_tty_pair * getpty();
void comstart();                      /* start up a command on the tty */
void alarm_catch();
/***********************************************************************
 * Getopt support
 */
extern int optind;           /* Current Argument counter.      */
extern char *optarg;         /* Current Argument pointer.      */
extern int opterr;           /* getopt() err print flag.       */
extern struct event_con * curr_event; /* the event we are looking for; used as
                                         a flag to see if scanning or not */
extern short int dumpout_flag;

void statdump();
/*
 * Should never happen....
 */
void sigbus();
/*
 * Toggle the input processing mode
 */
void sigint();
/***********************************************************************
 * This is the abnormal termination routine; dumps out status information
 * for debugging purposes
 */
void sigterm();
/***********************************************************************
 * Child process has died; shut down gracefully
 */
void sigchild();
/*********************************************************
 * Get a tty/pty pair
 * -  There are a number of peculiarities about this code
 *    -  the pty skeleton needs to be adjusted to suit the 
 *       individual implementation
 *    -  pty's can only be driven by detached processes on pure BSD 4.2
 *    -  In general, only one process can read/write a pty at a time. So
 *       how is the fork() managed? 
 */
struct pty_tty_pair * getpty();
/**********************************************************************
 * routine to initiate the command to execute; give it the tty, and the 
 * command
 */
void comstart();
int ptyread();
#ifndef PATH_AT
#define MANUAL_RESTORE
#endif
#ifdef MANUAL_RESTORE
void restore_tty();
#endif
#endif
