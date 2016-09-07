/**********************
 * ttlex.c - lexical analysis of the ptydrive input stream.
 * there are two modes of operation:
 *   - see through
 *   - terminal-independent
 * The selection depends on the value of the variable pg.see_through 
 */

static char * sccs_id="@(#) $Name$ $Id$\n\
Copyright (c) E2 Systems Limited 1993";
#include <stdio.h>
#include <ctype.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <string.h>
#include <errno.h>
#include "circlib.h"
#include "matchlib.h"
#include "termdef.h"
#ifdef V32
#include <signal.h>
#include <stropts.h>
#include <poll.h>
void disc_catcher()
{
    return;
}
#ifdef FORK_CLOCK
void short_alarm(len)
int len;
{
struct pollfd fds[2];
int pid;
    pid=getpid();
    sigset(SIGUSR1,disc_catcher);
    if (fork() == 0)
    {
        poll(fds,0,len);
        kill(pid,SIGUSR1);
        exit(0);
    }
    else
        return;
}
#endif
void alarm_restore (al_sav, cur_alarm)
int al_sav;
void (*cur_alarm)();
{
 
     if (cur_alarm == SIG_DFL
      || cur_alarm == disc_catcher
      || cur_alarm == SIG_IGN)
         alarm(0);
     else
     {
         sigset(SIGALRM,cur_alarm);
         if (al_sav > 0)
             alarm(al_sav);
     }
     return;
}
#endif
static HASH_CON * key_hash;   /* hash table for key names */
static void append_macro();   /* routine to add to the has table */
/*******************************************************************
 * Support the injection of existing echo files
 * - This is distinct from the ability to switch between a script
 *   file and user input
 * - ttlex() loads up a circular buffer
 * - file and user input switching involves switching the buffer
 * - injection involves putting whatever into the current buffer
 * - EOF means switching the input file, BUT NOT THE BUFFER.
 *
 * A user interrupt will therefore terminate the injection, leaving the
 * injection source open. This will get tidied up later.
 *
 * It is possible to envisage specifying a generator program rather
 * than an echo file. This needs a dialogue box in order to specify
 * the extra input parameters to fdsetup.sh. Generator programs should
 * have some means of indicating what they are expecting; an input
 * form can then be dynamically constructed. This functionality should
 * therefore only be used in interactive mode?
 */
static FILE * stack_file[20]; /* Allow nesting 20 deep */
static FILE * stack_script[20]; /* Allow nesting 20 deep */
static int see_through[20];   /* Allow nesting 20 deep */
static int next_in;           /* Index to list */
void push_file(fname)
char * fname;
{
    FILE * fin;
    if (next_in < 20 && ((fin = fopen(fname,"r")) != (FILE *) NULL))
    {
        see_through[next_in] = pg.see_through; 
        stack_file[next_in] = pg.cur_in_file;
        stack_script[next_in] = pg.script_file;
        pg.script_file = fin;
        pg.cur_in_file = fin;
        pg.in_check = (1 << fileno(pg.cur_in_file));
        pg.see_through = 0;
        next_in++;
    }
#ifdef DEBUG
    fprintf(pg.log_output,"# Open echo file: %s no: %d errno: %d #\n",
                      fname, fileno(pg.cur_in_file), errno); 
    fflush(pg.log_output);
#endif
    return;
}
int pop_file()
{
#ifdef DEBUG
    fprintf(pg.log_output,"# pop_file() called; next_in : %d #\n", next_in);
    fflush(pg.log_output);
#endif
    alarm(0);                   /* No alarm clock */
    pg.sav_write_mask = pg.term_check;
    pg.write_mask = pg.sav_write_mask;
    if ( next_in <= 0 )
        return 0;                /* Exhausted */
    if (fileno(pg.cur_in_file))
        fclose(pg.cur_in_file);  /* Do not close stdin or the terminal */
    next_in--;
    pg.see_through = see_through[next_in]; 
    pg.cur_in_file = stack_file[next_in];
    pg.script_file = stack_script[next_in];
    pg.in_check = (1 << fileno(pg.cur_in_file));
    return 1;
}
/*******************************************************************
 *
 * The terminal-independent input stream has the following structure.
 *
stream :== keys
       | stream keys

keys :==
       | key
       | keysequence
       | commentsequence
       | whitespace

key :== [A-Za-z][A-Za-z]*
keysequence :== "[^"][^"]*"
             | '[^'][^']*'
             | \\[^\][^\]*\\
commentsequence :== #[^#][^#]*#
commandequence :== \\[^\][^\]*\\
whitespace :== [\t \n][ \t\n]*

 * Whitespace and commentsequence are ignored.
 * Command sequence controls timestamping and think times.
 * - S label:timeout:string:action: ...; set up
 *   to recognise a timestamp
 * - Tlabel; take a timestamp, and label it with the label
 * - Wwait time; wait this number of seconds
 * - Rtyping rate; keys per second
 * - Mname=value; macro definition
 *
 * keysequences are inserted into the pty buffer as individual
 * characters
 * key names are looked up, as terminfo short names for string variables.
 * If not found, they are treated as keysequences.
 * 
 * No sequence should be more than BUFLEN characters.
 */
int ttlex()
{
/**************************************************************************
 * Tokenisation of Terminal-independent input stream
 */
static signed char parm_buf[BUFLEN];
#ifdef TERMCAP
static signed char work_buf[BUFLEN];
#endif
                                    /* maximum size of single lexical element */
   register signed char * parm = parm_buf;
   register signed char c;
#ifdef DEBUG
   fprintf(pg.log_output,"# in fd=%d\n #",fileno(pg.cur_in_file));
   fflush(pg.log_output);
#endif
   for (;;)
   {   /* Outer loop for file switches */
again:
   if (pg.see_through)
   {
       short int do_esc;
       for ( do_esc = 0;;)
       {
           int ret;
           short int ch;
#ifdef V32
           void (*cur_alarm)();
           int al_sav;
#endif
#ifdef NOSTDIO
#ifdef V32
           cur_alarm = sigset(SIGALRM,disc_catcher);
           al_sav = alarm(1);
#endif
           ret = read(fileno(pg.cur_in_file),parm_buf,1);
           if (ret == 1)
           {
#ifdef V32
               alarm_restore(al_sav,cur_alarm);
#endif
               ch = (short int) *parm_buf;
           }
           else
           if (ret < 0 && errno == EAGAIN)
           {
#ifdef V32
               alarm_restore(al_sav,cur_alarm);
#endif
               return 1;
           }
           else
           if (ret < 0 && errno == EINTR)
           {
#ifdef V32
               alarm_restore(al_sav,cur_alarm);
#endif
               return 1;
           }
#ifdef DEBUG
           else
           if (ret < 0 )
           {
#ifdef V32
               alarm_restore(al_sav,cur_alarm);
#endif
               fprintf(pg.log_output,"# read error on %d errno %d #\n",
                        fileno(pg.cur_in_file), errno);
               fflush(pg.log_output);
               if (pop_file())
                   goto again;
               else
                   return 0;
           }
#endif
           else
           {
#ifdef V32
               alarm_restore(al_sav,cur_alarm);
#endif
               if (pop_file())
                   goto again;
               else
                   return 0;
           }
#else
           c = (signed char) getc(pg.cur_in_file);
           if (c == (signed char) EOF)
           {
               if (pop_file())
                   goto again;
               else
                   return 0;
           }
           ch = (short int) c;
#endif
           if (pg.esc_comm_flag == 1)
           {
               if (ch == pg.cur_esc_char)
               {
#ifdef PATH_AT
                   if (pg.dumpin_flag == 2)
                   {
                       do_pop_up();
                       return 1;
                   }
                   else
#endif
                   if (do_esc == 1)
                   {
                       char * nl = "\n";
                       do_esc = 0;
#ifdef NOSTDIO
                       (void) write(fileno(pg.log_output),nl,1);
#else
                       (void) putc((int) '\n',pg.log_output);
#endif
                   }
                   else do_esc = 1;
               }
               if ((do_esc == 1) || (ch == pg.cur_esc_char))
               {
#ifdef NOSTDIO
                   (void) write(fileno(pg.log_output),parm_buf,1);
#else
                   (void) putc(c, pg.log_output);
#endif
                   continue;
               }
           }
#ifdef PATH_AT
           else
           if ((ch == pg.cur_esc_char) && (pg.dumpin_flag == 2))
           {
               do_pop_up();
               return 1;
           }
#endif
           bufadd(PTYOUT, ch);
           bufadd(PTYCHAR,1);
           return 1;
       }
   }
   else
   for (c = 32; c != (signed char) EOF;)
   {
       if ((c=(signed char) getc(pg.cur_in_file)) == (signed char) EOF)
       {
           if (pop_file())
               goto again;
           else
               return 0;
       }
       if (c > 127)
       {
           fprintf(stderr,"c = %d EOF = %d\n",c,EOF);
           abort();
       }
   switch (c)
   {                               /* look at the character */
   case ' ':
   case '\n':
   case '\t':                      /* White Space */
              break;
   case '"':
   case '\'':                         /* string; does not support
                                       * embedded quotes, but concatenation
                                       * and alternative delimiters provide
                                       * work-rounds
                                       */
             parm = parm_buf;
             *parm = (signed char) getc(pg.cur_in_file);
             if (*parm ==  (signed char) EOF)
                 return 0; /* ignore empty non-terminated string*/
             if (*parm == c) break;      /* ignore empty string */
             {
                 do
                 { /* advance to end of string */
                     bufadd(PTYOUT, (short int) *parm);
                     bufadd(PTYCHAR,1);
                     *parm = (signed char) getc(pg.cur_in_file);
                 }
                 while ( *parm != (signed char) EOF && *parm != c);
                 return 1 ;
             }
   case '`':
                                      /* Single key string. The \ is
                                       * an escape character.
                                       */
             parm = parm_buf;
             *parm = (signed char) getc(pg.cur_in_file);
             if (*parm == EOF) return 0; /* ignore empty non-terminated string*/
             if (*parm == c) break;      /* ignore empty string */
             {
                 short int i;
                 i = 0;
                 for (i = 1; i < BUFLEN; i++)
                 { /* advance to end of string */
                     bufadd(PTYOUT, (short int) *parm);
                     *parm = (signed char) getc(pg.cur_in_file);
                     if (*parm == '\\')
                     {
                         *parm = (signed char) getc(pg.cur_in_file);
                     }
                     else
                     if (*parm == (signed char) EOF || *parm == c)
                         break;
                 }
                 bufadd(PTYCHAR,i);
#ifdef DEBUG
                 fprintf(stderr,
                         "PTYOUT->buf_cnt=%ld PTYCHAR->buf_cnt=%ld len=%ld\n",
                         PTYOUT->buf_cnt, PTYCHAR->buf_cnt, i);
                 fprintf(stderr,
                         "PTYOUT->base=%ld PTYOUT->top=%ld diff=%ld\n",
                         (long) PTYOUT->base, (long) PTYOUT->top,
                          PTYOUT->top -  PTYOUT->base);
                 fflush(stderr);
#endif
                 return 1 ;
             }
   case '\\':
                                      /* Command string */
             parm = parm_buf;
             *parm = (signed char) getc(pg.cur_in_file);
             if (*parm == (signed char) EOF)
                 return 0; /*ignore empty non-terminated command*/
             if (*parm == c) break;      /* ignore empty command */
             {                        
                 for ( parm++, *parm = (signed char) getc(pg.cur_in_file);
                        *parm != (signed char) EOF;
                            parm++,
                            *parm = (signed char) getc(pg.cur_in_file))
                                      /* advance to end of string,
                                       * treating '\\' as an escape character
                                       * storing it in parm_buf
                                       */
                      if (*parm == c)
                      {
                          *parm = (signed char) getc(pg.cur_in_file);
                          if (*parm != c)
                          {
                             (void) ungetc(*parm,pg.cur_in_file);
                                           /* pop back this character */
                             break;        /* got the lot */
                          }
                          /* otherwise, we have stripped an escape character */
                      }   
                 *parm = '\0';        /* terminate the string */
                 parm = parm_buf;
                 if (*parm == 'T')
                 {                 /* Take a timestamp */
                     short int x;
                     parm++;
                     parm = nextfield(parm,':');
                     x = (((short int) (*parm)) << 8) + ((short int) *(parm+1));
                     bufadd(PTYOUT,  x);
                     bufadd(PTYCHAR,0);
                 }
                 else if (*parm == 'S')
                 {                /* Declare a timestamp */
                     parm++;
                     stamp_declare(parm);
                     break;
                 }
                 else if (*parm == 'R')
                 {                /* User Typing Rate */
                     int speed;
                     parm++;
                     speed = atoi(parm);
                     if (speed < 1) speed = 1;
                     else if (speed > 14) speed = 14;
                     bufadd(PTYCHAR,(short int) (-speed));
                 }
                 else if (*parm == 'M')
                 {                /* Macro Definition */
                     char * nm;
                     char * sb;
                     parm++;
                     if ((nm = nextfield(parm, '='))
                      && (nm = strdup(nm))
                      && (sb = nextfield(NULL, '=' ))
                      && (sb = strdup(sb)))
                         append_macro(nm,sb);
                 }
                 else if (*parm == 'E')
                 {                /* Nested Echo File */
                     parm++;
                     push_file(parm);
                     continue;    /* Pick something from the new file */
                 }
                 else if  (*parm == 'W')
                 {                /* Sleep for some number of seconds */
                     int thinks;
                     parm++;
                     thinks = atoi(parm);
                     if (thinks < 0) thinks = 0;
                     else if (thinks > 32750) thinks = 32750;
                     bufadd(PTYCHAR,(short int) (- 15 - thinks));
                 }
                 else break;       /* unrecognised command */
                 return 1 ;
             }
   case '#':                          /* Comment; ignore */
             parm=parm_buf;
             *parm = (signed char) getc(pg.cur_in_file);
             if (*parm == (signed char) EOF)
                 return 0; 
             if (*parm == c) break;
             {
                 for ( *parm = (signed char) getc(pg.cur_in_file);
                        *parm != (signed char) EOF && *parm != c;
                            *parm = (signed char) getc(pg.cur_in_file));
                                      /* advance to end of comment */
                 break ;
             }
   default:                        /* Treat everything else as a key name */
             parm=parm_buf;
             *parm = c;
             if (!isalnum(c))
             {
                 short int y;
                 bufadd(PTYOUT, (short int) *parm);
                 bufadd(PTYCHAR,1);
                 break;
             }
             else
             for (parm++, *parm = (signed char) getc(pg.cur_in_file);
             isalnum (*parm) ;
                  *parm = isupper(*parm) ? tolower(*parm) : *parm,
                  parm++,
                  *parm = (signed char) getc(pg.cur_in_file));
         (void) ungetc(*parm,pg.cur_in_file);        /* pop back this character */
         if (parm_buf != parm)
         {
             short int j;
             HIPT * x;
             *parm = '\0';
             if ((x = lookup(key_hash,parm_buf)) != (HIPT *) NULL)
             {                   /* Name is known */
                 short int * ip;
/*
 * If the thing is found, output the string found, otherwise output the
 * name as a string
 */
                 for (j = 0,
                      ip = ((struct word_con *) (x->body))->words;
                            *ip != 0;
                                  j++,
                                  ip++)
                 {
                     bufadd(PTYOUT, *ip);
                 }
                 bufadd(PTYCHAR, j);
             }
             else                /* Name is not known */
             {
                 short int y;
                 parm=parm_buf;
                 while (*parm)
                 {
                     bufadd(PTYOUT, (short int) *parm);
                     bufadd(PTYCHAR,1);
                     parm++;
                 } 
             }
             return 1;
     }
    } /* switch is repeated on next character if have not returned */
    } /* switch is repeated on next character if have not returned */
    } /* In case we have a file switch */
}
/***********************************************************************
 * Manage the terminal string substitution capability
 */
void addmacro(slot, nm, sb)
struct event_con * slot;
char * nm;
char * sb;
{
    int i;
    short int * x;
    struct word_con * curr_word;
#ifdef DEBUG
    fprintf(pg.log_output,"# %s = %s #\n", nm, sb);
    fflush(pg.log_output);
#endif
    curr_word = (struct word_con *)
                        malloc(sizeof(struct word_con));
    curr_word->next_word = slot->first_word;
    slot->first_word = curr_word;
    insert(key_hash,nm,(char *) curr_word);
    i = strlen(sb) + 1;
    curr_word->comped = (short int *) NULL;
    curr_word->words = (short int *)
                           malloc ( i * sizeof(short int));
    x = curr_word->words;
    while (i--)
           *x++ = (short int) *sb++;
    i = strlen(nm) + 1;
    curr_word->action = (short int *)
                               malloc ( i * sizeof(short int));
    x = curr_word->action;
    while (i--)
        *x++ = (short int) *nm++;
    return;
}
/*
 * Add a macro definition to the terminal database
 */
static void append_macro(nm,sb)
char * nm;
char * sb;
{
    int x;
    HIPT * h;
    struct event_con * slot;
    x = ((int) 'Z') << 8;
    h = lookup(pg.poss_events,(char *) (x));
    if (h == (HIPT *) NULL)
    {
#ifdef DEBUG
    fprintf(pg.log_output,"# Failed to add macro: %s = %s #\n", nm, sb);
    fflush(pg.log_output);
#endif
        return;
    }
    slot =(struct event_con *) h->body;
    addmacro( slot, nm, sb);
    slot->first_word->tail = pg.term_buf.tail;
    slot->first_word->head = pg.term_buf.head;
    slot->first_word->state = slot->first_word->words;
    return;
}
/*
 * Routine that loads up the key definitions for the
 * current terminal. It:
 * - Initialises the terminal control structure
 * - Allocates an event control structure
 * - Defines the multi-character keys as the possible events
 * - Adds the key names to a hash table
 * - Returns a pointer to the allocated structure.
 */
struct event_con * termload()
{
    int x;
    HIPT * h;
    struct event_con * slot;
    struct key_name_desc * curr_key, * top_key;
    int i;
#ifndef PATH_AT
#ifdef TERMCAP
    (void) tgetent(termcapbuf,getenv("TERM"));  /* read in the termcap database
                                            * for this terminal
                                            */
#else
    (void) setupterm(getenv("TERM"),1,0);  /* read in the terminfo database
                                            * for this terminal
                                            */
#endif
#endif
    pg.term_buf = *(PTYOUT);
    key_hash = hash (sizeof(key_name_desc)/sizeof(struct key_name_desc),
                  string_hh,strcmp);
/*
 * Use a reserved event (Z) to hash the terminal definition
 */
    x = ((int) 'Z') << 8;
    h = lookup(pg.poss_events,(char *) (x));
    if (h != (HIPT *) NULL)
         /* clear up the existing terminal definition */
        event_con_destroy((struct event_con *) h->body);
    else
    {
        h = insert(pg.poss_events,(char *) (x),(char *) NULL);
        if (h == (HIPT *) NULL)
        {
            (void) fprintf(stderr,"Hash Insert Failure\n");
            return (struct event_con *) NULL;
        }
    }
    slot = (struct event_con *) malloc (sizeof (struct event_con));
    h->body = (char *) slot;
    strcpy((slot)->event_id,"Z");
    (slot)->time_int = 0.0;
    (slot)->min_delay = 0;
    (slot)->timeout.tv_sec = 0;
    (slot)->timeout.tv_usec = 0;
    (slot)->first_word = (struct word_con *) NULL;
    (slot)->word_found = (struct word_con *) NULL;
    (slot)->comment = (short int *) NULL;
    (slot)->match_type = NOMAGIC;
    for ( curr_key = key_name_desc,
         top_key =
             &key_name_desc[sizeof(key_name_desc)/sizeof(struct key_name_desc)];
             curr_key < top_key;
                   curr_key++)
    {
        int i;
        char * k_ptr;
        short int * x;
#ifdef TERMCAP
        signed char ** y;
        signed char * y1;
        y1 = work_buf;
        y = &y1;
#endif
        if ((
#ifdef TERMCAP
             k_ptr = (signed char *) tgetstr(curr_key->t_name,y)
#else
#ifdef TGNEED
#ifdef OSF
             k_ptr = (signed char *) tgetstr(curr_key->tc_name,"")
#else
             k_ptr = (signed char *) tgetstr(curr_key->tc_name,(char *) NULL)
#endif
#else
             k_ptr = (signed char *) tigetstr(curr_key->t_name)
#endif
#endif
             ) == (signed char *) NULL)
        {
            continue;        /* ignore if this key has no definition for this
                                 terminal */
        }
        addmacro(slot, curr_key->t_name,k_ptr);
    }
    match_set_up(&(pg.term_buf), slot);
    return slot;
}
