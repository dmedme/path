/****************************************
 * Circular buffer manipulation functions
 *  - Simple add and remove
 *  - read into/write from the circular buffer
 */
static char * sccs_id= "@(#) $Name$ $Id$\n\
Copyright (c) E2 Systems Limited, 1993";
#include <stdio.h>
#include <errno.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "circlib.h"
#include "matchlib.h"
static char hold_area[BUFLEN];
#ifdef PATH_AT
static char log_area[BUFLEN];            /* Building up log strings */
static char * log_ptr=log_area;          /* What is currently there */
/* 
 * Function to insert in the circular buffer logging area. 
 */ 
static void log_flush()
{
    if (log_ptr > log_area + 3
     || log_area[0] != '\'' || log_area[1] != '\'')
    {
#ifdef NOSTDIO
        (void) write(fileno(pg.log_output), log_area,(log_ptr - log_area));
#else
        (void) fwrite(log_area, (log_ptr - log_area), 1, pg.log_output);
#endif
    }
    log_ptr = log_area;
    return;
}
void log_inject(str) 
char * str; 
{ 
    if (log_ptr == log_area + 1)
        log_ptr = log_area;
    if (log_ptr != log_area)
    {
        *log_ptr++ = '\'';
        *log_ptr++ = '\n';
    }
    (void) sprintf(log_ptr,"#\n%s\n#\n",str); 
    log_ptr += strlen(log_ptr); 
    log_flush();
    return; 
} 
/*
 * Function to log the key-match discard in terminal-independent format
 */
void log_discard (buf,head,tail)
struct circ_buf *buf;
short int *head;
short int*tail;
{
short int lchar;
struct circ_buf disc_buf;
    disc_buf = *buf;
    disc_buf.head = head; 
    disc_buf.tail = tail; 
    disc_buf.buf_cnt = (disc_buf.head >= disc_buf.tail) ?
                       (disc_buf.head - disc_buf.tail) :
                       (BUFLEN  + disc_buf.head - disc_buf.tail) ;
    if (log_ptr == log_area)
        *log_ptr++ = '\'';
    while (disc_buf.buf_cnt > 0)
    {
        (void) buftake(&disc_buf,&lchar);
        if (lchar == 17 || lchar == 19)
            continue;   /* Ignore ^Q and ^S */
        if (log_ptr >= (log_area + 78) ||
            lchar < (short int) ' ')
        {
            *log_ptr++ = '\'';
            *log_ptr++ = '\n';
            log_flush();
            *log_ptr++ = '\'';
            *log_ptr++ = (char) lchar;
            if (lchar < (short int) ' ')
            {
                *log_ptr++ = '\'';
                *log_ptr++ = '\n';
                log_flush();
                if (disc_buf.buf_cnt > 0)
                    *log_ptr++ = '\'';
            }
        }
        else
            *log_ptr++ = (char) lchar;
    }
    return;
}
/*
 * Add an element to the buffer
 */
int bufadd(buf,x)
struct circ_buf *buf;
int x;
{
register struct circ_buf * rbuf = buf;
register short int * new_head = rbuf->head;
    new_head++;
    if (new_head == rbuf->top)
        new_head = rbuf->base;
    if (new_head == rbuf->tail)
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Wrap-around on the buffer on bufadd()\n");
        bufdump(buf);
#endif
        return 0;   /* would wrap round; cannot process */
    }
    else
    {
        *(rbuf->head) = (short int) x;
        rbuf->head = new_head;
        return ++(rbuf->buf_cnt);
    }
}
/*
 * Increment the value on the most recent addition to the buffer.
 */
int bufinc(buf,x)
struct circ_buf *buf;
int x;
{
register struct circ_buf * rbuf = buf;
register short int * new_head = rbuf->head;
    if (new_head == rbuf->tail)
    {
#ifdef DEBUG
        (void) fprintf(stderr,
               "bufinc(): Nothing in the buffer to increment!\n");
        bufdump(buf);
#endif
        return 0;   /* Nothing to increment! */
    }
    new_head--;
    if (new_head < rbuf->base)
        new_head = rbuf->top - 1;
    *new_head += x; 
    return 1;
}
/*
 * Remove an element from the buffer
 */
int buftake(buf,x)
struct circ_buf * buf;
short int * x;
{
    register struct circ_buf * rbuf = buf;
    if (rbuf->buf_cnt <= 0)
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Taking from an empty buffer!");
        bufdump(buf);
#endif
        return -1;
    }
    else
        *x = *(rbuf->tail++);
        if (rbuf->tail==rbuf->top)
            rbuf->tail = rbuf->base;
    return --(rbuf->buf_cnt);
}
/*
 * Remove an element from the buffer from the head end.
 */
int bufpop(buf,x)
struct circ_buf * buf;
short int * x;
{
    register struct circ_buf * rbuf = buf;
    if (rbuf->buf_cnt <= 0)
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Popping from an empty buffer!");
        bufdump(buf);
#endif
        return -1;
    }
    else
    {
        register short int * new_head = rbuf->head;
        new_head--;
        if (new_head < rbuf->base)
            new_head = rbuf->top - 1;
        *x = *new_head;
        rbuf->head = new_head;
    }
    return --(rbuf->buf_cnt);
}
/*
 * Restore an erroneously removed buffer element
 */
int bufuntake(buf)
struct circ_buf * buf;
{
    register struct circ_buf * rbuf = buf;
    if (rbuf->buf_cnt == (BUFLEN - 1))
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Wrap-around on the buffer on bufuntake()\n");
        bufdump(buf);
#endif
        return 0;
    }
    else if (rbuf->tail == rbuf->base)
        rbuf->tail = rbuf->top - 1;
    else rbuf->tail--;
    return ++(rbuf->buf_cnt);
}
#endif
/*
 * Read from a file descriptor into the circular buffer
 */
int bufread(fd,buf,debug_flag)
int fd;
struct circ_buf * buf;
int debug_flag;
{
register struct circ_buf * rbuf = buf;
register int read_count;
register char * x = hold_area;
int n;
time_t t;

    read_count = BUFLEN - 1 - rbuf->buf_cnt;
    if (read_count <= 0)
    {
#ifdef DEBUG
        (void) fprintf(stderr,"Wrap-around on the buffer on read: %d\n",
                       read_count);
        bufdump(buf);
#endif
        return read_count;
    }
/*
 * Packet mode does flow control. I cannot believe this has never caused a
 * problem before!
 */
flow_control:
    read_count = read(fd,x, read_count);
#ifdef LINUX
    if (read_count <= 0 && errno == EINTR)
    {
#ifdef DEBUG_FULL
        if ( pg.alarm_flag == 0)
        {
            fputs("# Bogus interrupt!?\n#", stderr);
#endif
            read_count = BUFLEN - 1 - rbuf->buf_cnt;
            goto flow_control;
#ifdef DEBUG_FULL
        }
        else
        {
            t = time(0);
            fprintf(stderr, "# bufread() timed out %s\n#", ctime(&t));
        }
#endif
    }
#endif
    if (read_count >= 0 && pg.packet_mode)
    {
#ifdef DEBUG
        if (*x != 0)
        {
            fprintf(stderr, "# packet_mode byte: 0x%02x length: %d\n#",
                    *x, read_count);
        }
#endif
        if (*x != 0x0 || read_count == 1)
        {
            read_count = BUFLEN - 1 - rbuf->buf_cnt;
            goto flow_control;
        }
        x++;
        read_count--;
    }
    n = read_count;
#ifdef DEBUG
#ifdef NOSTDIO
    write(2,"\nbufread()\n",11);
    if (read_count > 0)
        write(2,x, read_count);
#else
    t = time(0);
    fprintf(stderr, "# %s: bufread()=%d:%.*s\n#",
        ctime(&t), read_count, read_count, x);
#endif
#endif
    if (debug_flag)
#ifdef NOSTDIO
        (void) write(fileno(stderr), x, (read_count < 0)?0: read_count);
#else
        (void) fwrite(x,sizeof( char ), (read_count < 0)?0:read_count,stderr);
#endif
    if (pg.dumpin_flag == 2)
        logadd(IN_DIRECTION,x,n);
    while (read_count-- > 0)
    {
        short int y;
        y = (short int) (*x++);
        (void) bufadd(rbuf,y);
    }
    return n;
}
#ifdef TIMEOUT_HANDLER
#include <signal.h>
static int clear_fd;
static int sav_sleep;
static char junk_buf[BUFLEN];
static void unblock()
{
int read_count;

    sav_sleep--;
    if ((read_count = read(clear_fd,junk_buf,sizeof(junk_buf)))<=0)
    {        /* Child has terminated */
#ifdef DEBUG
            perror("unblock() read returned nothing");
            (void) fprintf(stderr,"Error number %d\n",errno);
            (void) fprintf(stderr,"Characters read %d\n",read_count);
            statdump();  /* terminate with a status dump */
#endif
            return;
    }
#ifdef DEBUG
    (void) write(2,"\nunblock() read\n",16);
    write(2, junk_buf, read_count);
#endif
    if (pg.dumpout_flag)
    {
#ifdef NOSTDIO
        (void) write(fileno(stderr),junk_buf,read_count);
#else
        (void) fwrite(junk_buf,sizeof( char ),read_count,stderr);
#endif
    }
    return;
}
#endif
/*
 * Write from a circular buffer onto the file descriptor
 * If debug flag, also put to pg.log_output
 */
int bufwrite(fd,buf,n,debug_flag)
int fd;
struct circ_buf * buf;
int n;
int debug_flag;
{
long excymask;
long testmask;
char ioc;
register struct circ_buf * rbuf = buf;
static int rec_flag;
struct circ_buf dbuf;
register int write_count = n;
int nwrite_count;
register char * x = hold_area; /* should get the same buffer
                                  each time; saves zeroising it
                                  setting up the stack frame */
short int y;
#ifdef TIMEOUT_HANDLER
   void (*prev_alarm)();
   clear_fd = fd;
   prev_alarm = sigset(SIGALRM,unblock);
   sav_sleep = alarm(1);
   testmask = 1 << fd;
#endif
#ifdef PATH_AT
    if (debug_flag == 2)
    {
        register struct word_con * curr_word;
        pg.term_buf.head = rbuf->tail + n;
        if (pg.term_buf.head >= pg.term_buf.base + BUFLEN)
            pg.term_buf.head = pg.term_buf.head - BUFLEN;
        for (curr_word = pg.term_event->first_word;
               curr_word != (struct word_con *) NULL &&
                curr_word->state == curr_word->words;
                   curr_word = curr_word->next_word);
        if (curr_word == (struct word_con *) NULL)
        {
#ifdef DEBUG
            char buf[2048];
            sprintf(buf,"pg.term_buf.head %ld pg.term_buf.tail %ld\n",
                        (long) pg.term_buf.head,
                        (long) pg.term_buf.tail);
            write(fileno(pg.log_output),buf,strlen(buf));
#endif
            pg.term_buf.tail = rbuf->tail;
            for (curr_word = pg.term_event->first_word;
                   curr_word != (struct word_con *) NULL;
                       curr_word = curr_word->next_word)
            {
                 curr_word->tail = pg.term_buf.tail;
                 curr_word->head = pg.term_buf.tail;
            }
        }
        pg.term_buf.buf_cnt = (pg.term_buf.head >= pg.term_buf.tail) ?
                               (pg.term_buf.head - pg.term_buf.tail) :
                               (BUFLEN  + pg.term_buf.head - pg.term_buf.tail) ;
    }
#endif
    if (write_count > rbuf->buf_cnt)
    {
        write_count = rbuf->buf_cnt;
        n = write_count;
    }
#ifdef DEBUG
    if (n == 216 && rec_flag == 0)
    {
        write(2,"Before\n",7);
        rec_flag = 1;
        statdump();
        rec_flag = 0;
        fflush(stderr);
    }
#endif
    while ( write_count-- > 0)
    {
        (void) buftake(rbuf,&y);
        *x++ = (char) y;
    } 
#ifdef DEBUG
    if (n == 216 && rec_flag == 0)
    {
        write(2,"After\n",7);
        rec_flag = 1;
        statdump();
        rec_flag = 0;
        fflush(stderr);
    }
#endif
    write_count = n;
    nwrite_count = 0;
    do
    {
    static struct timeval do_poll;
        write_count -= nwrite_count;
#ifdef TIMEOUT_HANDLER
        if (write_count > 32)
            nwrite_count = 32;
        else
            nwrite_count = write_count;
        while ( (nwrite_count = write ( fd, x - write_count, nwrite_count))
                  <0 && errno == EINTR)
        {
            (void) sigset(SIGALRM,unblock);
#ifdef DEBUG
            (void) write(2,"\nBlocked!!!\n",12);
#endif
            alarm(1);
        }
        alarm(0);
        excymask = testmask;
        if (select(20,0,0,&excymask,&do_poll) > 0)
        {
            if (read(fd, &ioc, 1) > 0 && ioc == 0x20)
                read(fd, &ioc, 1);
        }
#else
        nwrite_count = write ( fd, x - write_count, write_count);
#endif
#ifdef DEBUG
        (void) fprintf(stderr,"\nbufwrite(%ld)=%ld\n",time(0),nwrite_count);
        fflush(stderr);
        (void) write(2,x - write_count, nwrite_count);
#endif
    }
    while (nwrite_count > 0 && nwrite_count != write_count); 
    x -= n;
#ifdef PATH_AT
    if (debug_flag == 1)
    {
#ifdef NOSTDIO
        (void) write(fileno(pg.log_output), x, (int) n);
#else
        (void) fwrite(x, sizeof(char), (int) n, pg.log_output);
#endif
    }
    else if (debug_flag == 2)
    {
        short int i;
        logadd(OUT_DIRECTION,x,n);
        do
        {
            struct word_con * curr_word;
            struct word_con * disc;
            i = lanalyse(&pg.term_buf,pg.term_event,&curr_word,&disc);
            if (disc->head != disc->tail)
            {
#ifdef DEBUG
                char buf[2048];
            sprintf(buf,"disc->head %ld disc->tail %ld i %ld curr_word %ld\n",
                            (long) disc->head,
                            (long) disc->tail,
                            (long) i,
                            (long) curr_word);
                write(fileno(pg.log_output),buf,strlen(buf));
#endif
                log_discard (&pg.term_buf,disc->head,disc->tail);
            }
            if (i)
            {                      /* Matched */
                short int * y;
                if (log_ptr != log_area)
                {
                    *log_ptr++ = '\'';
                    *log_ptr++ = '\n';
                    log_flush();
                }
                pg.term_event->word_found = curr_word; 
                for (y = curr_word->action,
                     x = hold_area;
                         *y != 0;
                              *x++ = (char) *y++);
                *x++ = '\n';
#ifdef NOSTDIO
                (void) write(fileno(pg.log_output), hold_area, x - hold_area);
#else
                (void) fwrite(hold_area, sizeof(char), x - hold_area,
                              pg.log_output);
#endif
                pg.term_buf.tail = curr_word->head;
                pg.term_buf.buf_cnt = (pg.term_buf.head >= pg.term_buf.tail) ?
                                   (pg.term_buf.head - pg.term_buf.tail) :
                               (BUFLEN  + pg.term_buf.head - pg.term_buf.tail) ;
                match_set_up(&pg.term_buf,pg.term_event);
            }
        } while (i);
    }
#endif
#ifdef TIMEOUT_HANDLER
    (void)  sigset(SIGALRM,prev_alarm);
    if (sav_sleep > 0)
    {
        alarm(sav_sleep);
    }
#endif
    return n;
}
#ifdef PATH_AT
/*************************************************************************
 * Functions to support the maintenance of the I/O list.
 *
 * Data structures etc.
 * 4 circular buffers.
 * - characters received
 * - characters sent
 */
static struct circ_buf * buf_char[2];
/*
 * - counts of characters in each group
 */
static struct circ_buf * buf_cnt[2];
static int io_toggle = -1;   /* Set to an impossible value to bootstrap it */
/*
 * Initialisation function
 * -  Allocate the circular buffers
 */
void pair_track_ini()
{
    int i;
    for (i = 0; i < 2; i++)
    {
        buf_char[i] = cre_circ_buf();
        buf_cnt[i] = cre_circ_buf();
    }
    return;
}
/*
 * - add function
 *   - Parameters:
 *     - in or out
 *     - pointer
 *     - length
 *   - remove triggered by buffers getting full
 *   - only check the _char array; must have >= number in cnt arrayt
 *   - needs to remove from both in step
 *   - if the same as last;
 *     - extend current
 *   - otherwise
 *     - new one
 */
void logadd(io_ind,ptr,len)
int io_ind;
char * ptr;
int len;
{

    short int i;
/*
 * If there is no more room, discard the earliest
 */
    if (len + buf_char[io_ind]->buf_cnt >
            (buf_char[io_ind]->top - buf_char[io_ind]->base))
    {
        short int j;
        (void) buftake(buf_cnt[io_ind], &i);
        if (i < 0 || i > BUFLEN)
        {
            fprintf(stderr,"Logic Error: receive count out of range\n");
            fflush(stderr);
            i = len;           /* There must be room for this */
            io_toggle = -1;    /* Make sure that the logging is added as
                                * a new one rather than being incremented
                                */
        }
        while (i--)
            (void) buftake(buf_char[io_ind], &j);
        if (buf_char[io_ind]->buf_cnt <= 0)
            io_toggle = -1;    /* Make sure that the logging is added as
                                * a new one rather than being incremented
                                */
    }
/*
 * Now add the new item
 */
    for (i = len; i--; )
       bufadd(buf_char[io_ind], (short int) *ptr++);
/*
 * If not the same as before, create a new entry in the count buffer
 */
    if (io_ind != io_toggle)
    {
        (void) bufadd(buf_cnt[io_ind],(short int) len);
        io_toggle = io_ind;
    }
/*
 * Otherwise, increment the counter
 */
    else
        (void) bufinc(buf_cnt[io_ind], len);
    return;
}
/*
 * Dump out the contents of a circular buffer to stderr
 */
void bufdump(buf)
struct circ_buf * buf;
{
    struct circ_buf nbuf;
    nbuf = *buf;
    (void) fflush(stderr);
    (void) bufwrite(2,&nbuf,nbuf.buf_cnt,0);
    return;
}
/*
 * dump out the numbers in a circular buffer to stderr
 */
void bufndump(buf)
struct circ_buf * buf;
{
struct circ_buf nbuf;
short int y;
    nbuf = *buf;
    while ( nbuf.buf_cnt > 0)
    {
        (void) buftake(&nbuf,&y);
        (void) fprintf(stderr,"<%d>",(int) y);
    }
    (void) fprintf(stderr,"\n");
    return;
}
/******************************************************************
 * Set up to display the last few characters.
 *   - model on the do_os() function
 *   - go through buffers alternately in reverse order
 *   - add to the scrolling type block, preceded by send or receive 
 * - add this to the menus in pathdlib.c
 */
#include "natmenu.h"
void pair_review(screen,plate)
struct screen * screen;
struct template * plate;
{
    char line_buf[80];
    struct template * new;
    struct circ_buf tmp_buf_char[2];
    struct circ_buf tmp_buf_cnt[2];
    int dir;
    short int cnt;
    if (io_toggle == -1)
    {
        mess_write(screen,plate,"No history to display");
        return;
    }
/*
 * Use def_form() to get a new form
 *
 * Use add_member() to add to the scrolling region,
 */
    new =  def_form(&screen->shape,plate,SEL_YES,COMM_YES,MENU,
             "INSPECT: Reverse order message pairs",
             "Use ARROW keys to inspect, ^P to print, ^Z to exit",
"                                                                              \
 ","",(char *)NULL, (void (*)()) NULL);
    if (new == (struct template *) NULL)
    {
        mess_write(screen,plate,"Cannot create new form");
        return;
    }
/*
 * Loop through the strings embedded in the history in reverse order
 */
    for (dir = 0; dir < 2; dir++)
    {
        tmp_buf_char[dir] = *(buf_char[dir]);
        tmp_buf_cnt[dir] = *(buf_cnt[dir]);
    }
    for (dir = io_toggle;bufpop(&(tmp_buf_cnt[dir]),&cnt) > -1;)
    {
        int i;
        short int c;
        char * x, *x1, *x2, *buf;
/*
 * Allocate a buffer to get the stuff in reverse order, and with control
 * characters expanded.
 */
        if ((buf = (char *) malloc(2*cnt+1)) == (char *) NULL)
        {
            mess_write(screen,plate,"Out of core: Cannot create new form");
            return;
        }
#ifdef DEBUG
        fprintf(pg.log_output,"# Dir: %d Count: %d As Read:\n",dir,cnt);
#endif
        x1 = buf + 2*cnt; 
        *x1-- = '\0';
        i = cnt; 
/*
 * Pull out the characters, and place them in the buffer in the correct
 * order
 */
        while (i-- && (bufpop(&(tmp_buf_char[dir]),&c) > -1))
        {
            
            if (c < 32)
            {
                c += 64;
                *x1-- = (char) c;
                *x1-- = '^';
            }
            else
                *x1-- = (char) c;
#ifdef DEBUG
            putc( c,pg.log_output);
#endif
        }
#ifdef DEBUG
        putc('#',pg.log_output);
        putc('\n',pg.log_output);
#endif
        cnt = (buf + 2 * cnt) - x1 -1;
        if (dir == IN_DIRECTION)
        {
            line_buf[0] = 'A';
            line_buf[1] = '=';
        }
        else
        {
            line_buf[0] = 'U';
            line_buf[1] = '=';
        }
#ifdef DEBUG
        fprintf(pg.log_output,"# re-ordered: %d: %d: %s #\n",dir,cnt,x1+1);
#endif
        for ( x1 ++,   /* Now points at first character to display */
               x = x1 + cnt,
              x2 = &line_buf[2];
                  x1 < x;
                       x1++)
        {
            *x2++ = *x1;
            if (x2 >= &line_buf[screen->shape.men_cols])
            {
                *x2 = '\0';
                add_padded_member(&(new->men_cntrl),line_buf,
                                       screen->shape.men_cols);
                add_member(&(new->field_cntrl),"EXIT:");
                add_member(&(new->sel_cntrl)," ");
#ifdef DEBUG
                fprintf(pg.log_output,"# line_buf: %d: %s #\n",dir,line_buf);
#endif
                line_buf[0] = ' ';
                line_buf[1] = ' ';
                x2 = &line_buf[2];
            }
        }
        if (x2 != &line_buf[2])
        {
            *x2 = '\0';
            add_padded_member(&(new->men_cntrl),line_buf,
                                       screen->shape.men_cols);
            add_member(&(new->field_cntrl),"EXIT:");
            add_member(&(new->sel_cntrl)," ");
#ifdef DEBUG
            fprintf(pg.log_output,"# line_buf: %d: %s #\n",dir,line_buf);
#endif
        }
        if (dir == IN_DIRECTION)
            dir = OUT_DIRECTION;
        else
            dir = IN_DIRECTION;
        (void) free(buf);
    }
    (void) do_form(screen,new);
    destroy_form(new);
    dis_form(screen,plate);
    return;
}
#endif
