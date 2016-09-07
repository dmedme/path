/*
 * sbspec.c - Handle the generated number from the udt GUI process
 */
static char * sccs_id="Copyright (C) E2 Systems Limited 1997\n\
@(#) $Name$ $Id$";
#include <sys/types.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "circlib.h"
#include "matchlib.h"
static int look_pair[100] = 
{
6,
-1,
-1,
9,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
18,
-1,
21,
18,
-1,
21,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
33,
-1,
-1,
-1,
36,
36,
-1,
36,
-1,
-1,
-1,
42,
-1,
42,
-1,
-1,
-1,
-1,
51,
-1,
-1,
-1,
-1,
-1,
54,
58,
-1,
-1,
60,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
81,
-1,
-1,
-1,
-1,
-1,
-1,
91,
-1,
-1,
90,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1,
-1
};
/*
 * Find the challenge pair
 */
static int find_pair(n)
int n;
{
int x;
    if (n > 99 || n < 0)
        return 0;
    x = look_pair[n];  
    if (x == -1)
        x = n + 3;
    return x; 
}
/*
 * Logic:
 * -  Search through the incoming buffer for the number
 * -  Convert it to an integer
 * -  Look up its mate
 * -  Convert it to a string
 * -  Make space for the action string
 * -  Make space for the number and the end of the stuff
 */
void do_sbspec()
{
char ret_val[5];
char *y;
short int i;
short int * x;
struct circ_buf cb1,cb2;
short int to_do;
    for (;;)
    {
        buftake(PTYIN,&i);
        if (i == (short int) ';')
            break;
    }
    for (y = &ret_val[0], to_do = 2; to_do; to_do--, y++)
    {
        buftake(PTYIN,&i);
        *y = (char) i; 
    }
    sprintf(ret_val,"%02d\r\n", find_pair(atoi(ret_val)));
/*
 * Allow for the event to be rescheduled
 */
    bufuntake(PTYOUT);
    bufuntake(PTYCHAR);
/*
 * Allow for the action string
 */
    for (x = pg.curr_event->word_found->action, to_do = 0;
            *x != 0;
               to_do++, x++)
         bufuntake(PTYOUT);
    bufuntake(PTYCHAR);
/*
 * Allow for the extra 4 bytes added to the action string as the response
 * to the challenge
 */
    for (i = 0; i < 4; i++)
         bufuntake(PTYOUT);
    to_do += 4;
/*
 * Now set up to add the action string back
 */
    cb1 = *(PTYCHAR);
    cb2 = *(PTYOUT);
    cb1.head=cb1.tail;
    cb2.head=cb2.tail;
/*
 * Cause the script to loop by injecting this event back in
 */
    i = (((short int) (*(pg.curr_event->event_id))) << 8)
                   + ((short int) (*(pg.curr_event->event_id+1)));
    bufadd(&cb2, i);
    bufadd(&cb1,0);
    for (x = pg.curr_event->word_found->action; *x != 0; x++)
        bufadd(&cb2,*x);
    for (y = &ret_val[0]; *y != '\0'; y++)
        bufadd(&cb2,(int) (*y));
    bufadd(&cb1,to_do);
    return;
}
