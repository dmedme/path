/**********************************************************************
 * matchlib.c - incrementally search for patterns.
 */
static char * sccs_id="@(#) $Name$ $Id$\n\
Copyright (C) E2 Systems Limited 1993";
#include <sys/types.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <signal.h>
#include <stdio.h>
#include "circlib.h"
#include "matchlib.h"
/*
 * constants for re's
 */
#define CCHR 2
#define CDOT 4
#define CCL 6
#define NCCL 8
#define CEOF 11

#define CSTAR 01

static int match_exec();
static int cclass();
/*********************************************************************
 * lanalyse() - routine to identify events
 *
 * Keeps track of how far through each word it has got.
 *
 * returns  0; no match
 * returns  1; match made, in which case match points to matched stuff.
 * In both cases, disc points to discarded stuff.
 *
 * Copyright (c) 1993 E2 Systems Limited - All rights reserved
 */
int lanalyse(circular,curr_event,match,disc)
struct circ_buf * circular;
struct event_con * curr_event;
struct word_con ** match;
struct word_con ** disc;
{
char  c;
register short int * x;
register struct circ_buf * circ;
double time_stamp;
register struct word_con * curr_word;
static struct word_con discard;        /* To track what we discard */
short int j;
 
    circ = circular;
    discard.tail = circ->tail;
    discard.head = circ->tail;
#ifdef DEBUG
    write(2, "lanalyse() called\n", 18);
#endif

    for (curr_word = curr_event->first_word;
                  curr_word != (struct word_con *) NULL;
                  curr_word = curr_word->next_word)
    for (x = curr_word->head;
             (curr_word->tail != circ->head) ||(*(curr_word->state) == 0)
                 ;)
    {
#ifdef DEBUG
        if (x > circ->top || x < circ->base || circ->buf_cnt < 0 ||
            circ->buf_cnt > BUFLEN)
            abort();
#endif
        if (*(curr_word->state) == 0)
        {
            curr_word->head = x;
            *match = curr_word;
/*
 * Look to see if the start of the discard apparently lies within the match
 */
            if (!((curr_word->head >= curr_word->tail &&
                discard.tail >= curr_word->tail &&
                discard.tail <= curr_word->head)
             || (curr_word->head < curr_word->tail &&
                ((discard.tail >= circ->base &&
                discard.tail <= curr_word->head) ||
                (discard.tail < circ->top &&
                discard.tail >= curr_word->tail)))))
                discard.head = curr_word->tail;
#ifdef DEBUG
            fprintf(stderr,"discard.tail %ld discard.head %ld circ->tail %ld\n\
curr_word->head %ld curr_word->tail %ld\n",
                        (long) discard.tail,
                        (long) discard.head,
                        (long) circ->tail,
                        (long) curr_word->head,
                        (long) curr_word->tail);
            fflush(stderr);
#endif
            *disc = &discard;
            return 1;
        }
        else
/*
 * Non-regular expression search
 */
        if ((curr_event->match_type == NOMAGIC
             || curr_word->comped == (short int *) NULL)
         &&( *(curr_word->state) == (short int) '.'
             || *curr_word->state == *x))
        {
#ifdef DEBUG
            fprintf(stderr,
                       "NOMAGIC (x - circ->tail)=%ld offset=%ld *x=%ld\n",
                        (long) (x -  circ->tail),
                        (long) (curr_word->state - curr_word->words),
                        (long) *x);
            fflush(stderr);
#endif
            x++;
            if (x == circ->top)
                x=circ->base;
            curr_word->state++;
            curr_word->head = x;
            if (*(curr_word->state) == 0)
                continue;
            else if (x == circ->head)
                break;
        }
        else
/*
 * Regular expression search
 */
        if ((curr_event->match_type == MAGIC
             && curr_word->comped != (short int *) NULL)
         && match_exec(circ,curr_word))
        {
#ifdef DEBUG
            fprintf(stderr,"MAGIC (x - circ->tail)=%ld offset=%ld *x=%ld\n",
                        (long) (x - circ->tail),
                        (long) (curr_word->state - curr_word->comped),
                        (long) *x);
            fflush(stderr);
#endif
            if (*(curr_word->state) != 0)
                 break;   /* This is the case where we have run out of buffer
                           * before matching the whole thing
                           */
        }
        else
        {
            curr_word->tail++;
            if (curr_word->tail == circ->top)
                curr_word->tail = circ->base;
            if (curr_event->match_type == MAGIC
              && curr_word->comped != (short int *) NULL)
                curr_word->state = curr_word->comped;
            else
                curr_word->state = curr_word->words;
            x = curr_word->tail;
            curr_word->head = x;
        }
    }
/*
 * Otherwise, need to advance the tail until it bumps into a back marker
 */
    for(;;)
    {
    short int j;

        for (curr_word = curr_event->first_word;
                  curr_word != (struct word_con *) NULL;
                  curr_word = curr_word->next_word)
        {
           if (circ->tail == curr_word->tail)
           {
#ifdef DEBUG
            fprintf(stderr,"discard.tail %ld circ->tail %ld curr_word %ld\n",
                        (long) discard.tail,
                        (long) circ->tail,
                        (long) curr_word);
            fflush(stderr);
#endif

               discard.head = circ->tail;
               *match = (struct word_con *) NULL;
               *disc = &discard;
               return 0;
           }
        }
        if (circ->buf_cnt > 0)
        {
            buftake(circ,&j);
#ifdef DEBUG
            fprintf(stderr,"discard.tail %ld circ->tail %ld j %d\n",
                        (long) discard.tail,
                        (long) circ->tail,
                        j);
            fflush(stderr);
#endif
            discard.head = circ->tail;
        }
        else
        {
            (void) fputs(
               "Logic Error - buffer empty but haven't caught up match tail\n",
                         stderr);
            statdump();
            discard.head = circ->tail;
            *match = (struct word_con *) NULL;
            *disc = &discard;
            return 0;
        }
    }
}
/*
 * Flag the fact that the user has hit SIGQUIT
 */
static void request_force()
{
    if (pg.force_flag)
        pg.force_flag |= FORCE_DUE;
    else
        pg.force_flag |= FORCE_REQUEST;
    return;
}
/*
 * Set up for forced (auto) matching.
 */
void ini_force(timeout)
int timeout;
{
char buf[60];
    if (timeout < 1)
        timeout = 10;
    sprintf(buf,"A0:%d:..*..*..*..*..*..*..*..*::Skeleton Event",timeout);
    pg.auto_event = stamp_declare(buf);   /* Do not use it yet */
    pg.next_auto++;
    sigset(SIGQUIT,request_force);
    return;
}
/*
 * Force a match. Called when the match buffer wraps, or after the
 * expiry of a timeout.
 */
int match_force(circular,curr_event,match,disc)
struct circ_buf * circular;
struct event_con * curr_event;
struct word_con ** match;
struct word_con ** disc;
{
char  c;
register short int * x;
register struct circ_buf * circ;
register struct word_con * curr_word;
static struct word_con discard;        /* To track what we discard */
short int j;
 
    circ = circular;
    discard.tail = circ->tail;
    discard.head = circ->tail;
#ifdef DEBUG
    fprintf(stderr,"match_force() called\n");
#endif

    if ((curr_event == (struct event_con *) NULL) ||
         ((curr_word = curr_event->first_word) == (struct word_con *) NULL))
        return 0;
    *match = curr_word;
/*
 * Look to see if the start of the discard apparently lies within the match
 */
    if (!((curr_word->head >= curr_word->tail &&
                discard.tail >= curr_word->tail &&
                discard.tail <= curr_word->head)
    || (curr_word->head < curr_word->tail &&
                ((discard.tail >= circ->base &&
                discard.tail <= curr_word->head) ||
                (discard.tail < circ->top &&
                discard.tail >= curr_word->tail)))))
        discard.head = curr_word->tail;
#ifdef DEBUG
    fprintf(stderr,"discard.tail %ld discard.head %ld circ->tail %ld\n\
curr_word->head %ld curr_word->tail %ld\n",
                        (long) discard.tail,
                        (long) discard.head,
                        (long) circ->tail,
                        (long) curr_word->head,
                        (long) curr_word->tail);
    fflush(stderr);
#endif
    *disc = &discard;
    return 1;
}
/*
 * Function to log the key-match discard in terminal-independent format
 * in the case that we are dealing with a force match.
 * Note that no consideration has been given to the possible need to
 * stuff regular expression meta-characters. This is currently left as an
 * exercise for the users, since these definitions are going to be in
 * the wrong place anyway!
 */
void match_out (curr_word)
struct word_con * curr_word;
{
short int lchar;
struct circ_buf disc_buf;
    disc_buf = *(PTYIN);
    disc_buf.head = curr_word->head; 
    disc_buf.tail = curr_word->tail; 
    disc_buf.buf_cnt = (disc_buf.head >= disc_buf.tail) ?
                       (disc_buf.head - disc_buf.tail) :
                       (BUFLEN  + disc_buf.head - disc_buf.tail) ;
    while (disc_buf.buf_cnt > 0)
    {
        (void) buftake(&disc_buf,&lchar);
        if (lchar == 17 || lchar == 19)
            continue;   /* Ignore ^Q and ^S */
        fputc( lchar, pg.fo);
        if (pg.dumpin_flag == 2)
        {
            if ((char) lchar == '#')
                fputc( lchar, pg.log_output);
            fputc( lchar, pg.log_output);
        }
    }
    return;
}
/*
 * Routine to ready an event and a buffer for searching
 */
void match_set_up (buf, e)
struct circ_buf * buf;
struct event_con * e;
{
register struct word_con * curr_word;
    e->word_found = (struct word_con *) NULL;
#ifdef DEBUG
   fprintf(stderr,"match_set_up(%s)\n", e->event_id);
#endif

    for (curr_word = e->first_word;
             curr_word != (struct word_con *) NULL;
                 curr_word = curr_word->next_word)
    {
        curr_word->head = buf->head;
        curr_word->tail = buf->tail;
        if (e->match_type == MAGIC && curr_word->comped != (short int *) NULL)
            curr_word->state = curr_word->comped;
        else
            curr_word->state = curr_word->words;
    }
    return;
}
/************************************************************************
 * Compile the regular expression argument into a dfa
 */
short int * match_comp(sp,reg_len)
register short int *sp;
int *reg_len;    
{
static short int expbuf[512];   /* Should be enough even for long strings */
register int    c;
register short int    *ep = expbuf;
short int * endptr = expbuf + sizeof(expbuf);
int    cclcnt;
short int    *lastep = 0;

    if (sp == 0 || *sp == '\0')
    {
        return(0);
    }
    for (;;)
    {
        if (ep >= endptr)
            return 0;
        if ((c = *sp++) == '\0')
        {
            *ep++ = CEOF;
            *ep++ = 0;
            *reg_len = (ep - expbuf);
            return expbuf;
        }
        if (c != '*')
            lastep = ep;
        switch (c)
        {
        case '.':
            *ep++ = CDOT;
            continue;

        case '*':
            if (lastep == 0)
                goto defchar;
            *lastep |= CSTAR;
            continue;

        case '[':
            *ep++ = CCL;
            *ep++ = 0;
            cclcnt = 1;
            if ((c = *sp++) == '^') {
                c = *sp++;
                ep[-2] = NCCL;
            }
            do
            {
                if (c == '\0')
                    return 0;
                if (c == '-' && ep [-1] != 0)
                {
                    if ((c = *sp++) == ']')
                    {
                        *ep++ = '-';
                        cclcnt++;
                        break;
                    }
                    while (ep[-1] < c)
                    {
                        *ep = ep[-1] + 1;
                        ep++;
                        cclcnt++;
                        if (ep >= endptr)
                            return 0;
                    }
                }
                *ep++ = c;
                cclcnt++;
                if (ep >= endptr)
                    return 0;
            } while ((c = *sp++) != ']');
            lastep[1] = cclcnt;
            continue;

        case '\\':
            *ep++ = CCHR;
            *ep++ = c;
            continue;

        defchar:
        default:
            *ep++ = CCHR;
            *ep++ = c;
        }
    }
}
/************************************************************************** 
 *
 * Incrementally see if there is a match with any of the alternatives.
 *
 * Arguments:
 * - The circular buffer being searched
 * - The current state of this match (as defined by the word_con structure)
 */
static int match_exec(buf,word_con)
struct circ_buf * buf;
struct word_con * word_con;
{
register struct word_con *curr_word = word_con;
register short int *lp = curr_word->head;
register short int * curlp;
int    rv;

#ifdef DEBUG
    fprintf(stderr,"match_exec(input=%d, state=%d)\n", *lp,
                     (curr_word->state - curr_word->comped));
#endif

    for (;;)
    {
#ifdef DEBUG_FULL
        fprintf(stderr,"curr_word->state = %d\n", *(curr_word->state));
        bufdump(buf);
#endif
        switch (*curr_word->state++)
        {

        case CCHR:
            if (*curr_word->state++ == *lp++)
                break;
            return(0);

        case CDOT:
            if (*lp++)
                break;
            return(0);

        case CEOF:
            return(1);

        case CCL:
            if (cclass(curr_word->state, *lp++, 1))
            {
                curr_word->state += *curr_word->state;
                break;
            }
            return(0);

        case NCCL:
            if (cclass(curr_word->state, *lp++, 0))
            {
                curr_word->state += *curr_word->state;
                break;
            }
            return(0);


        case CDOT|CSTAR:
            curlp = lp;
            while (*lp++)
            {
                if (lp == buf->top)
                    lp=buf->base;
                curr_word->head = lp;
                if ( lp== buf->head)
                    return 1;
            }
            goto star;

        case CCHR|CSTAR:
            curlp = lp;
            while (*lp++ == *curr_word->state)
            {
                if (lp == buf->top)
                    lp=buf->base;
                curr_word->head = lp;
                if ( lp== buf->head)
                    return 1;
            }
            curr_word->state++;
            goto star;

        case CCL|CSTAR:
        case NCCL|CSTAR:
            curlp = lp;
            while (cclass(curr_word->state,
                      *lp++, curr_word->state[-1] == (CCL|CSTAR)))
            {
                if (lp == buf->top)
                    lp=buf->base;
                curr_word->head = lp;
                if ( lp== buf->head)
                    return 1;
            }
            curr_word->state += *curr_word->state;
            goto star;

        star:
            do
            {
                if (lp == buf->base)
                    lp=buf->top;
                lp--;
                curr_word->head = lp;
                if (rv = match_exec(buf, curr_word))
                    return(rv);
            }
            while (lp > curlp);
            return(0);

        default:
            return(-1);
        }
        if (lp == buf->top)
            lp=buf->base;
        curr_word->head = lp;
        if ( lp == buf->head && *curr_word->state != CEOF)
            return 1;
    }
}
/*
 * Routine to check that character c is or is not a member of the 
 * character set s
 */
static int cclass(set, c, af)
register short int    *set, c;
int    af;
{
    register int    n;

    if (c == 0)
        return(0);
    n = *set++;
    while (--n)
        if (*set++ == c)
            return(af);
    return(! af);
}
