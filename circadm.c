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
/*
 * Function to create a circular buffer
 */
struct circ_buf * cre_circ_buf()
{
    short int * x = (short int *) malloc( sizeof( short int) * BUFLEN + 4);
    struct circ_buf * buf = (struct circ_buf *) malloc(sizeof(struct circ_buf));
    if (x == (short int *) NULL || buf == (struct circ_buf *) NULL)
        return (struct circ_buf *) NULL;
    else
    {
        buf->buf_cnt = 0;
        buf->buf_low = BUF_LOW;
        buf->buf_high = BUF_HIGH;
        buf->head = x;
        buf->tail = x;
        buf->base = x;
        buf->top = x + BUFLEN;
        return buf;
    }
}
/*
 * Function to destroy a circular buffer
 */
void des_circ_buf(buf)
struct circ_buf * buf;
{
   (void) free( buf->base);
   (void) free( buf);
   return;
}
