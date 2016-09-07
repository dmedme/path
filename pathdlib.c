/*
 * pathdlib.c - Dialogue Management for PATH/AT
 *
 * This code is designed to be called by ptydrive to manage a 'pop-up'
 * dialogue with a user who is running a variant of ptydrive in script
 * generation mode.
 *
 */ 
static char * sccs_id = "@(#) $Name$ $Id$\nCopyright (c) E2 Systems 1993";
#include <limits.h>
#include <ctype.h>
#include <sys/types.h>
#ifdef V32
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include "ansi.h"
#include "circlib.h"
#include "matchlib.h"
#include "ptytty.h"
#include "natmenu.h"
#ifdef OSF
#undef FIELD_ATTR
#define FIELD_ATTR 0
#endif
#ifdef SCO
#define signed
#undef timeout
#endif
#ifdef SUN
#undef timeout
#endif
bool scroll_done;
char null_select[133];
struct template * menu_tree[FORMS_MAX];
/*
 * Module global data
 */
static struct pd_glob {
struct screen top_screen;
struct screen bot_screen;
char redraw[20];
char cnorm[20];
struct pty_tty_pair *pty_tty;
struct screen * cur_screen;
struct template * cur_plate;
char home[20];
FILE * out_file;
} pd_glob;
/*
 * Screen data processing routines
 */
static void toggle();
void pair_review();
static void start_loop();
static void end_loop();
static void comment();
static void new_rule();
static void new_event();
static void do_echo();
static void exist_rule();
static void new_esc();
static void new_rdw();
static void start_force();
static void comment();
static void setup_event();
static void setup_rdr();
static void setup_esc();
static void intncpy();
/*
 * Skeleton screen definitions
 */
static struct form_skel {
    enum sel_allow sel_allow;
    enum comm_allow comm_allow;
    enum comm_type comm_type;
    char * parent;
    char * head;
    char * sel_lines[17];
    char * prompt;
    void (*trigger_fun)();
} form_skel[] = {
{
    SEL_YES, COMM_NO, MENU, (char *) NULL,
    "MANAGEMENT: Script Management              ",
    {
        "EVENT:   Define an Event               ",
        "REVIEW:  Review the Message Pairs      ",
        "COMMENT: Enter Remarks                 ",
        "SCRIPT:  Manipulate Generated Script   ",
        "CONFIG:  Change Configuration Options  ",
        "TOGGLE:  Switch Pop-up                 ",
        "EXIT:    Close Pop-up                  "
     },
     "Choose Option                          ",
     toggle
},
{
    SEL_YES, COMM_NO, MENU, "MANAGEMENT:",
    "SCRIPT:  Manipulate Generated Script    ",
    {
        "ECHO:    Execute an Echo File          ",
        "OPEN:    Start of Loop                 ",
        "CLOSE:   End of Loop                   ",
        "RULE:    Field Value Supply            ",
        "EXIT:    Return to previous menu       "
     },
     "Choose Option and Press RETURN         ",
     do_echo
},
{
    SEL_YES, COMM_NO, MENU, "MANAGEMENT:",
    "CONFIG: Change Configuration Parameters",
    {
        "NEWESC:  Change Escape                 ",
        "CHGRDW:  Change Redraw String          ",
        "FORCE:   Enable Forced Events          ",
        "EXIT:    Return to previous menu       "
     },
     "Choose Option and Press RETURN         ",
     toggle
},
{
    SEL_YES, COMM_NO, MENU, "MANAGEMENT:",
    "REVIEW:  Review the Message Pairs      ",
    {
        "INSPECT: Reverse ordered message pairs ",
        "EVENT:   Define an Event               ",
        "COMMENT: Enter Remarks                 ",
        "EXIT:    Return to previous menu       "
     },
     "Choose Option and Press RETURN         ",
     pair_review
},
{
    SEL_NO, COMM_NO, SYSTEM, "MANAGEMENT:",
    "OPEN: Start of Loop                    ",
    {
       "How many ?                             "
    },
    "Enter how many times you want to loop  ",
    start_loop
},
{
    SEL_NO, COMM_NO, SYSTEM, "MANAGEMENT:",
    "CLOSE: Finish Loops                    ",
    {
       "How many ?                             "
    },
    "Enter the number of loop levels to close",
    end_loop
},
{
    SEL_YES, COMM_NO, MENU, "MANAGEMENT:",
    "RULE: Field Value Supply               ",
    {
        "DEFINE:  Name New Rule              ",
        "PICK:    Pick Existing Rule         ",
        "EXIT:    Return to Previous Menu    "
    },
    "Choose Option and Press RETURN         ",
    exist_rule
},
{
   SEL_NO, COMM_NO, SYSTEM, "RULE:",
        "DEFINE: Name New Rule                 ",
    {
       "Rule Name:                             ",
       "Table Col:                             "
    },
    "Enter name and source database column  ",
     new_rule
},
{
    SEL_NO, COMM_NO, SYSTEM, "MANAGEMENT:",
    "EVENT: Define an Event                 ",
    {
         "ID      :                              ",
         "Title   :                              ",
         "Timeout :                              ",
         "Look for:                              ",
         "Response:                              ",
         "Look for:                              ",
         "Response:                              ",
    },
    "Enter 2 char. ID, time out and matches ",
    new_event
},
{
    SEL_NO, COMM_YES, SYSTEM, "MANAGEMENT:",
    "COMMENT: Enter Remarks                 ",
    {
         "Text:                                  "
    },
    "Enter comments and Press RETURN        ",
    comment
},
{
    SEL_NO, COMM_NO, SYSTEM, "CONFIG:",
    "NEWESC: Change the Escape character    ",
    {
         "Character:                             "
    },
    "Enter new escape character             ",
    new_esc
},
{
    SEL_NO, COMM_NO, SYSTEM, "CONFIG:",
    "CHGRDW: Change the Redraw String       ",
    {
         "String:                             "
    },
    "Enter new redraw string                ",
    new_rdw
},
{
    SEL_NO, COMM_NO, SYSTEM, "CONFIG:",
    "FORCE:  Enable Forced Events           ",
    {
       "How long ?                             "
    },
    "Enter how long to wait for a response  ",
    start_force
},
{
    SEL_YES, COMM_NO, MENU, (char *) NULL,
    "EXCEPTION: Test Recovery               ",
    {
        "ABORT:   Abandon Run                   ",
        "FIXUP:   Manual Fixup                  ",
        "RESYNCH: Resume Script                 ",
        "MANAGEMENT: Script Management          ",
        "COMMENT: Enter Remarks                 ",
        "DISPLAY: Inspect Failed Event          ",
        "TOGGLE:  Switch Pop-up                 "
     },
     "Choose Option                          ",
},
{
    SEL_NO, COMM_NO, SYSTEM, "EXCEPTION:",
    "DISPLAY: Inspect Failed Event         ",
    {
         "ID      :                              ",
         "Title   :                              ",
         "Timeout :                              ",
         "Look for:                              ",
         "Response:                              ",
         "Look for:                              ",
         "Response:                              ",
    },
    "Press Return to Exit"
}
};
 
/****************************************************************************
 * Populate the form cache from the prompt strings
 */
static void form_init()
{
struct screen * screen;
struct template work_plate; /* home for various working variables and defaults
*/
char * comm;          /* command to execute */
char * prompt;        /* header string if wanted */
char line_buf[4096];
static char spare_buf[133];
struct template * new;
struct template ** par;
int i;
int ret;
register struct form_skel * fsp;
/*
 * Loop through the templates in the skeleton array, and initialise forms
 * for them.
 */
    natmenu = 0;   /* Declare that we are embedded */
    memset (spare_buf,' ',sizeof(spare_buf) - 1);
    for (i = 0,
         fsp = form_skel;
             i < sizeof(form_skel)/sizeof(struct form_skel);
                  i++,
                  fsp++)
    {
    int j;
        if (fsp->parent == (char *) NULL
            || ((par = find_form(fsp->parent)) == (struct template **) NULL))
        {
            new = (struct template *) NULL;
            par = &new;
        }
        if ((new =  def_form(&pd_glob.top_screen.shape, *par,
                                fsp->sel_allow,
                                fsp->comm_allow,
                                fsp->comm_type,
                                fsp->head,fsp->prompt,spare_buf,spare_buf,
                                (char *) NULL, fsp->trigger_fun))
                           == (struct template *) NULL)
        {
#ifdef DEBUG
     printf("Cannot create form!?\n");
#endif
            return;
        }
        for (j = 0; j < sizeof(fsp->sel_lines)/
                       sizeof(fsp->sel_lines[0]) &&
                    fsp->sel_lines[j] != (char *) NULL; j++)
        {
             char * x;
             add_member(&(new->sel_cntrl)," ");
             add_member(&(new->men_cntrl),fsp->sel_lines[j]);
             if (fsp->comm_type == MENU &&
                 (x = strchr(fsp->sel_lines[j],':')) != (char *) NULL)
             {
                  strncpy(line_buf,fsp->sel_lines[j],
                         (x - fsp->sel_lines[j]+1)); 
                  *(line_buf +(x - fsp->sel_lines[j]+1)) = '\0';
                  add_member(&(new->field_cntrl),line_buf);
             }
             else 
                  add_member(&(new->field_cntrl),spare_buf);
        }
    }
    help_init(&pd_glob.top_screen.shape);
#ifdef DEBUG_FULL
    dump_tree();
#endif
    return;
}
/***********************************************************************
 * chg_redraw() - Function to change the redraw string
 */
void chg_redraw(rdr_str)
char * rdr_str;
{
char *x;
#ifdef TGNEED
#ifdef OSF
    x = (signed char *) tgetstr(rdr_str,"");
#else
    x = (signed char *) tgetstr(rdr_str,(char *) NULL);
#endif
#else
    x = (signed char *) tigetstr(rdr_str);
#endif
    if (x != (char *) NULL && (int) x != -1)
        strcpy(pd_glob.redraw,x);
    else
        strcpy(pd_glob.redraw,rdr_str);
    return;
}
/************************************************************************
 * Set up the terminal modes
 */
void good_term_modes()
{
    keypad(pd_glob.top_screen.full_scr,TRUE);
    crmode();
    nonl();
    noecho();
#ifdef DEBUG
    fprintf(stderr,"Done good_term_modes()\n");
    fflush(stderr);
#endif
    return;
}
/************************************************************************
 * dia_init() - Function to initialise the screen structures for the
 *              PATH/AT dialogues. Because it uses the curses library,
 *              needs to have the terminal on file number 0.
 *              Parameters are:
 *              -   The string to use for redrawing the underlying
 *                  screen
 *              -   The pty_tty pair.
 *
 * Processing.
 * -  Initialise the curses library
 * -  Initialise the menus and forms.
 * -  Set up two sets of windows; one at the top right corner, the other at
 *    the bottom right
 * -  Initialise the global state:
 *    -  Redraw screen
 *    -  Which to display (start with bottom right) 
    curses_screen = new_term(term,out_file,in_file);
 *  not sure where the "WINDOW" is?
 */
void dia_init (rdr_str,pty_tty,term,in_file,out_file)
char * rdr_str;
struct pty_tty_pair * pty_tty;
char * term;
FILE * in_file;
FILE * out_file;
{
char * x;
struct form_out_line shape_low, shape_high;
int pad_mode;
int e2lines;
int e2cols;
struct template ** wform;
    if ((x = getenv("LINES")) == (char *) NULL)
        e2lines = DEF_LINES/2;
    else
        e2lines = atoi(x)/2;
    if ((x = getenv("COLUMNS")) == (char *) NULL)
        e2cols = DEF_COLUMNS/2;
    else
        e2cols = atoi(x)/2;
    setup_shape(&shape_high,e2lines,e2cols,e2cols,0);
    setup_shape(&shape_low,e2lines,e2cols,e2cols,e2lines);
#ifdef OSF
    set_curses_modes();
#endif
#ifdef AIX
/*
 * AIX only works with stdin and stdout?
 */
    pd_glob.top_screen.full_scr = initscr();
#else
#ifdef HP8
/*
 * AIX only works with stdin and stdout?
 */
    pd_glob.top_screen.full_scr = initscr();
#else
    (void) newterm(term,out_file,in_file);
#endif
#endif
    pd_glob.top_screen = setup_wins(&shape_high);
    pd_glob.top_screen.full_scr = stdscr;
    pd_glob.bot_screen = setup_wins(&shape_low);
    pd_glob.bot_screen.full_scr = stdscr;
    pd_glob.cur_screen = &pd_glob.bot_screen;
    pd_glob.pty_tty = pty_tty;
    good_term_modes();
    wclrtobot(stdscr);
    wrefresh(stdscr);
    chg_redraw(rdr_str);
    form_init();
    if ((wform = find_form("CHGRDW:"))!= (struct template **) NULL)
        setup_rdr(*wform);
    if ((wform = find_form("NEWESC:"))!= (struct template **) NULL)
        setup_esc(*wform);
#ifdef TGNEED
#ifdef OSF
     x = (signed char *) tgetstr("ho","");
#else
     x = (signed char *) tgetstr("ho",(char *) NULL);
#endif
#else
     x = (signed char *) tigetstr("home");
#endif
     if (x != (char *) NULL && (int) x != -1)
         strcpy(pd_glob.home,x);
     else
         pd_glob.home[0] =  '\0';
#ifdef TGNEED
#ifdef OSF
     x = (signed char *) tgetstr("ve","");
#else
     x = (signed char *) tgetstr("ve",(char *) NULL);
#endif
#else
     x = (signed char *) tigetstr("cnorm");
#endif
     if (x != (char *) NULL && (int) x != -1)
         strcpy(pd_glob.cnorm,x);
     else
         pd_glob.cnorm[0] =  '\0';
     pd_glob.out_file = out_file;
#ifdef DEBUG
    fprintf(stderr,"Done dia_init()\n");
    fflush(stderr);
#endif
     return;
}
/**********************************************************************
 * do_redraw() - Function to redraw the underlying screen
 */
void do_redraw()
{
struct timeval t;
int r_mask;
#ifdef ULTRIX
    t.tv_sec = 1;
    t.tv_usec = 0;
#else
    t.tv_sec = 0;
    t.tv_usec = 250000;
#endif
    (void) write(pd_glob.pty_tty->ptyfd,pd_glob.redraw,strlen(pd_glob.redraw));
    blank_shape(&pd_glob.cur_screen->shape);
    mvwin(stdscr,0,0);
    wclrtobot(stdscr);
    wrefresh(stdscr);
    for (r_mask = 1 << (pd_glob.pty_tty->ptyfd);
            select(20,&r_mask, 0, 0, &t) > 0
            && ptyread(pd_glob.pty_tty->ptyfd);
                r_mask = 1 <<  (pd_glob.pty_tty->ptyfd));
#ifdef DEBUG
    fprintf(stderr,"Done do_redraw()\n");
    fflush(stderr);
#endif
    return;
}
/***********************************************************************
 * do_pop_up() - Handle an interrupt, clearing up afterwards
 * - Call the appropriate function.
 * - Redraw the underlying screen
 * - Clear the event
 * - Update global state.  
 */
void do_pop_up()
{
    (*pg.cur_dia)();
    if (pg.curr_event != (struct event_con *) NULL)
    {
        event_record("Z", pg.curr_event);
        pg.abort_event = pg.curr_event;
        pg.curr_event = (struct event_con *) NULL;
    }
    do_redraw();
    pg.write_mask = pg.term_check;
    pg.sav_write_mask = pg.term_check;
    return;
}
/***********************************************************************
 * toggle() - swap between the top window and the bottom window
 *
 * - Redraw the underlying screen
 * - Display the other windows
 * - Update global state.  
 */
static void toggle()
{
    if (pd_glob.cur_screen == &pd_glob.bot_screen)
        pd_glob.cur_screen = &pd_glob.top_screen;
    else
        pd_glob.cur_screen = &pd_glob.bot_screen;
    do_redraw();
    (*pg.cur_dia)();
    return;
}
/**********************************************************************
 * dia_man() - Function to manage the dialogue
 * - Restore the saved x,y
 * - Redraw the windows
 * - Run the dialogue
 * - Write out anything required to the trace channel, between comments
 * - Save the x,y?
 * - Send the redisplay string to the application
 * - Exit
 */
void dia_man()
{
static struct template ** root_form;
char * ret;
    if (root_form == (struct template **) NULL)
    {
        if ((ret = getenv("PATH_DIALOGUE")) == (char *) NULL ||
                (root_form = find_form(ret)) == (struct template **) NULL) 
            root_form = find_form("MANAGEMENT:"); 
    }
    if (root_form != (struct template **) NULL)
    {
        (void) fwrite(pd_glob.home,sizeof(char),strlen(pd_glob.home),
                      pd_glob.out_file);
        (void) fwrite(pd_glob.cnorm,sizeof(char),strlen(pd_glob.cnorm),
                      pd_glob.out_file);
        (void) fflush(pd_glob.out_file);
#ifdef DEBUG
    fprintf(stderr,"Doing dia_man()\n");
    fflush(stderr);
#endif
        ret = do_form(pd_glob.cur_screen, *root_form);
    }
    return;
}
/**********************************************************************
 * fix_man() - Function to restore control after a test has
 * stuffed up.
 */
void fix_man()
{
static int sav_in_check;     /* for the case where we have EOF on the script
                                file */
static struct template ** fix_form, **dis_form;
    if (fix_form == (struct template **) NULL)
        fix_form = find_form("EXCEPTION:"); 
    if (dis_form == (struct template **) NULL)
        dis_form = find_form("DISPLAY:"); 
    if (fix_form != (struct template **) NULL)
    {
        char * ret;
        (void) fwrite(pd_glob.cnorm,sizeof(char),strlen(pd_glob.cnorm),
                      pd_glob.out_file);
        (void) fwrite(pd_glob.home,sizeof(char),strlen(pd_glob.home),
                      pd_glob.out_file);
        (void) fflush(pd_glob.out_file);
        if (pg.abort_event != (struct event_con *) NULL &&
                dis_form != (struct template **) NULL)
            setup_event(*dis_form);
        for (;;)
        {
            ret = do_form(pd_glob.cur_screen,*fix_form);
            if (!strcmp(ret,"ABORT:"))
            {
                endwin();
#ifdef OSF
                unset_curses_modes();
#endif
                sigterm(); /* Terminate */
            }
            else
            if (!strcmp(ret,"FIXUP:"))
            {
                struct circ_buf *tmp;
                if (pg.see_through)
                {
                    mess_write(pd_glob.cur_screen,*fix_form,
                      "Already fixing up");
                    continue;
                }
                pg.see_through = 1;
                pg.sav_write_mask = pg.term_check;
                pg.write_mask = pg.sav_write_mask;
                pg.cur_in_file = stdin;
                sav_in_check = pg.in_check;
                pg.in_check = (1 << fileno(pg.cur_in_file));
                tmp = pg.circ_buf[3];
                pg.circ_buf[3] = pg.circ_buf[0];
                pg.circ_buf[0] = tmp;
                tmp = pg.circ_buf[4];
                pg.circ_buf[4] = pg.circ_buf[1];
                pg.circ_buf[1] = tmp;
                PTYOUT->buf_cnt = 0;
                PTYOUT->tail = PTYOUT->base;
                PTYOUT->head = PTYOUT->base;
                PTYCHAR->buf_cnt = 0;
                PTYCHAR->tail = PTYCHAR->base;
                PTYCHAR->head = PTYCHAR->base;
                log_discard(&pg.term_buf,pg.term_buf.head,pg.term_buf.tail);
                pg.term_buf = *(PTYOUT);
                match_set_up(PTYOUT,pg.term_event);
                break;
            }
            else
            if (!strcmp(ret,"RESYNCH:"))
            {
                struct circ_buf *tmp;
                if (!pg.see_through)
                {
                    mess_write(pd_glob.cur_screen,*fix_form,
                      "Carrying on regardless!?");
                    continue;
                }
                pg.see_through = 0;
                pg.cur_in_file = pg.script_file;
                pg.in_check = sav_in_check;
                pg.sav_write_mask = pg.term_check;
                pg.write_mask = pg.sav_write_mask;
                tmp = pg.circ_buf[0];
                pg.circ_buf[0] = pg.circ_buf[3];
                pg.circ_buf[3] = tmp;
                tmp = pg.circ_buf[1];
                pg.circ_buf[1] = pg.circ_buf[4];
                pg.circ_buf[4] = tmp;
                log_discard(&pg.term_buf,pg.term_buf.head,pg.term_buf.tail);
                pg.term_buf = *(PTYOUT);
                match_set_up(PTYOUT,pg.term_event);
                break;
            }
            else
            if (!strcmp(ret,"TOGGLE:"))
            {
                toggle();
                return;   /* toggle() will redraw */
            }
        }
    }
    else abort();    /* Should not happen */
    return;
}
char * pick_one_file(screen,plate,pattern,prompt)
struct screen * screen;
struct template * plate;
char * pattern;       /* pattern to search for */
char * prompt;        /* header string if wanted */
{
char line_buf[80];
static char comm_buf[80]; /* Used for returning selected file */
    int i, ret;
    FILE * fp;
/*
 * Loop through the output from the ls command until
 * no more, stuffing them in a control structure
 */
    sprintf(comm_buf,"ls %s",pattern);
    if ((fp = npopen(comm_buf, "r")) == (FILE *) NULL)
    {
        (void) strcpy(line_buf,"Couldn't execute ls command");
        comm_buf[0] = '\0';
    }
    else
    {
    struct template * new;
/*
 * Use def_form() to get a new form
 *
 * Use add_member() to add line_buf and next_buf to the scrolling region,
 * and loop
 * through picking up the others
 */
         comm_buf[0] = '\0';
         new =  def_form(&screen->shape,plate,SEL_YES,COMM_NO,SYSTEM,
         prompt,"Use ARROW keys to inspect, ^P to print, ^Z to exit",
"                                                                              \
 ","",(char *)NULL, (void (*)()) NULL);
        if (new == (struct template *) NULL)
        {
            mess_write(screen,plate,"Cannot create new form");
            return;
        }
        while (fgets(line_buf,sizeof(line_buf),fp) != (char *) NULL)
        {
            i = strlen(line_buf);
            if (*(line_buf+i-1) == '\n')
                *(line_buf+i-1) = '\0';
            add_padded_member(&(new->men_cntrl),line_buf,
                                screen->shape.men_cols);
            add_member(&(new->field_cntrl),"EXIT:");
            add_member(&(new->sel_cntrl)," ");
        }
        ret = npclose(fp);
        (void) do_form(screen,new);
        for (i = 0; i < new->men_cntrl.member_cnt; i++)
        {
            char *x;
            if (((x = find_member( &(new->sel_cntrl), i))
                      != (char *) NULL)
                && *x == '*'
                && ((x = find_member( &(new->men_cntrl), i))
                      != (char *) NULL))
            {
                strcpy(comm_buf,x);
                break;
            }
        }
        destroy_form(new);
        dis_form(screen,plate);
    }
    if (comm_buf[0] != '\0')
    {
        (void) sprintf(line_buf,"Picked file %s",comm_buf);
        mess_write(screen,plate,line_buf);
        return comm_buf;
    }
    else
    {
        (void) sprintf(line_buf,"No file picked");
        mess_write(screen,plate,line_buf);
        return (char *) NULL;
    }
}
/*********************************************************************
 * PATH/AT Screen Application Routines
 * VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 */

static void start_loop(screen,plate)
struct screen * screen;
struct template * plate;
{
    char * x;
    int i;
    char buf[133];
    if ((x = find_member( &(plate->field_cntrl),0)) != (char *) NULL)
    {
        if (!strncmp(x,"ntrans",6))
        {
            log_inject("{ntrans");
            return;
        }
        else
            i = atoi(x);
        if (i != 0)
        {
            sprintf(buf,"{%d",i);
            log_inject(buf);
        }
    }
    return;
}
static void end_loop(screen,plate)
struct screen * screen;
struct template * plate;
{
    char * x;
    int i;
    char buf[133];
    if ((x = find_member( &(plate->field_cntrl),0)) != (char *) NULL)
    {
        i = atoi(x);
        if (i != 0)
        {
            sprintf(buf,"}%d",i);
            log_inject(buf);
        }
    }
    return;
}
static void exist_rule(screen,plate)
struct screen * screen;
struct template * plate;
{
    char buf[133];
    char * x;
    if ((x = getenv("PATH_RULE_BASE")) == (char *) NULL)
    {
        mess_write(screen,plate,"No PATH_RULE_BASE in the environment");
        return;
    }
    sprintf(buf,"%s/*.db",x);
    if ((x = pick_one_file(screen,plate,buf,"EXIST: Select from the list"))
                 != (char *) NULL)
    {
        sprintf(buf,"ER\n%s",x);
        log_inject(buf);
    }
    return;
}
static void do_echo(screen,plate)
struct screen * screen;
struct template * plate;
{
    char buf[133];
    char * x;
    if ((x = getenv("PATH_HOME")) == (char *) NULL)
    {
        mess_write(screen,plate,"No PATH_HOME in the environment");
        return;
    }
    sprintf(buf,"%s/echo/E*.ech",x);
    if ((x = pick_one_file(screen,plate,buf,"EXECUTE: Select from the list"))
           != (char *) NULL)
    {
        TRAIL_SPACE_STRIP(x);
        push_file(x);
    }
    return;
}
static void new_rule(screen,plate)
struct screen * screen;
struct template * plate;
{
    char buf[BUFSIZ];
    int i;
    register char *x = buf;
    register char *x1;
    char * desc;
    *x++ = 'N';
    *x++ = 'R';
    *x++ = '\n';
    if ((x1 = find_member( &(plate->field_cntrl),0)) == (char *) NULL)
    {
        mess_write(screen,plate,"No rule ID defined");
        return;
    }
    TRAIL_SPACE_STRIP(x1);
    if (*x1 == '\0')
    {
        mess_write(screen,plate,"No rule ID defined");
        return;
    }
    sprintf(x,"%s\n",x1);
    x += strlen(x);
    if ((x1 = find_member( &(plate->field_cntrl),1)) != (char *) NULL)
    {
        TRAIL_SPACE_STRIP(x1);
        if (*x1 != '\0')
        {
            sprintf(x,"%s\n",x1);
            x += strlen(x);
        }
    }
    log_inject(buf);
    return;
}
/****************************************************************************
 * Routine to stuff things for the event definition
 */
static char * evstuff(in)
char * in;
{
    static char buf[133];
    register char *x1=in, *x2=buf;
    while (*x1 != '\0')
    {
        if (*x1 == ':')
        {
            *x2++ = '\\';
            *x2++ = '\\';
        }
        else if (*x1 == '\\')
        {
            *x2++ = '\\';
            *x2++ = '\\';
            *x2++ = '\\';
        }
        *x2++ = *x1++;
    }
    *x2 = '\0';
    return buf;
}
/******************************************************************
 * convert an array of short ints to a string
 */
static void intncpy(str,int_arr,n)
char * str;
short int * int_arr;
int n;
{
    short int * x;
    for (x = int_arr, n--;
            n && *x != 0;
                 n--,
                 x++)
        *str++ = (char) *x;
    *str = '\0';
    return;
}
/****************************************************************************
 * Ready the aborted event for display
 */
static void setup_event(plate)
struct template * plate;
{
char buf[30];
struct word_con * curr_word;
int i;
    chg_member(&(plate->field_cntrl),0, pg.abort_event->event_id);
    if (pg.abort_event->comment != (short int *) NULL)
    {
        intncpy(buf,pg.abort_event->comment,sizeof(buf));
        chg_member(&(plate->field_cntrl),1,buf) ;
    }
    else
        chg_member(&(plate->field_cntrl),1, "");
    (void) sprintf(buf,"%d",pg.abort_event->timeout.tv_sec);
    chg_member(&(plate->field_cntrl),2, buf);;
    for (curr_word = pg.abort_event->first_word, i=3;
                  i < 7 && curr_word != (struct word_con *) NULL;
                  curr_word = curr_word->next_word)
    {
        intncpy(buf,curr_word->words,sizeof(buf));
        chg_member(&(plate->field_cntrl),i++, buf);;
        intncpy(buf,curr_word->action,sizeof(buf));
        chg_member(&(plate->field_cntrl),i++, buf);;
    }
    return;
}
/****************************************************************************
 * Ready the redraw string for edit/display.
 */
static void setup_rdr(plate)
struct template * plate;
{
struct word_con * curr_word;
int i;
    chg_member(&(plate->field_cntrl),0, pd_glob.redraw);
    return;
}
static void setup_esc(plate)
struct template * plate;
{
    char buf[2];
    buf[0] = (char) pg.cur_esc_char;
    buf[1] = '\0';
    chg_member(&(plate->field_cntrl),0, buf);
    return;
}
/****************************************************************************
 * Define a new event
 */
static void new_event(screen,plate)
struct screen * screen;
struct template * plate;
{
    char buf[BUFSIZ];
    int i;
    register char *x = buf;
    register char *x1;
    char * desc;
    *x++ = '\\';
    *x++ = 'S';
    if ((x1 = find_member( &(plate->field_cntrl),0)) == (char *) NULL)
    {
        mess_write(screen,plate,"No event ID defined");
        return;
    }
    TRAIL_SPACE_STRIP(x1);
    for (i = 0; *x1 != '\0'; *x1++)
    {
        if ( islower(*x1))
             *x1 = (char ) toupper(*x1);
        if ( isdigit(*x1) || isupper(*x1))
        {
            *x++ = *x1; 
            i++;
        }
    }
    if (!i)
    {
        mess_write(screen,plate,"No event ID defined");
        return;
    }
    *x++ = ':';
    if ((desc = find_member( &(plate->field_cntrl),1)) == (char *) NULL)
    {
        mess_write(screen,plate,"No event description defined");
        return;
    }
    TRAIL_SPACE_STRIP(desc);
    if ((x1 = find_member( &(plate->field_cntrl),2)) == (char *) NULL ||
        (i = atoi(x1)) == 0)
    {
        mess_write(screen,plate,"Illegal timeout");
        return;
    }
    sprintf(x,"%d:",i);
    x += strlen(x);
    for (i = 3;
         (x1 = find_member( &(plate->field_cntrl),i)) != (char *) NULL;
              i++)
     {
        TRAIL_SPACE_STRIP(x1);
        x1 = evstuff(x1);
        if (*x1 == '\0')
            break;
        sprintf(x,"%s:",x1);
        x += strlen(x);
        i++;
        if ((x1 = find_member( &(plate->field_cntrl),i)) != (char *) NULL)
        {
            TRAIL_SPACE_STRIP(x1);
            x1 = evstuff(x1);
            sprintf(x,"%s:",x1);
            x += strlen(x);
        }
        else
            *x++ = ':';
    }
    desc = evstuff(desc);
    sprintf(x,"%s\\\n\\T",desc);
    x += strlen(x);
    x1 = buf+2;
    while ( *x1 != ':' )
        *x++ = *x1++;
    *x++= ':';
    *x++ = '\\';
    *x = '\0';
    log_inject(buf);
    return;
}
/****************************************************************************
 * Enter a comment
 */
static void comment(screen,plate)
struct screen * screen;
struct template * plate;
{
    char buf[BUFSIZ];
    int i;
    register char *x = buf;
    register char *x1;
    char * desc;
    sprintf(x,"REMARKS\n=======\n");
    x += strlen(x);
    if ((x1 = find_member( &(plate->field_cntrl),0)) == (char *) NULL)
    {
        mess_write(screen,plate,"No comment entered");
        return;
    }
    TRAIL_SPACE_STRIP(x1);
    if (*x1 == '\0')
    {
        mess_write(screen,plate,"No comment entered");
        return;
    }
    sprintf(x,"%s\n",x1);
    x += strlen(x);
    for (i = 1;
         (x1 = find_member( &(plate->field_cntrl),i)) != (char *) NULL
         && ((x + strlen(x1)) < (buf + sizeof(buf)));
              i++)
     {
        TRAIL_SPACE_STRIP(x1);
        if (*x1 == '\0')
            continue;
        sprintf(x,"%s\n",x1);
        x += strlen(x);
    }
    log_inject(buf);
    return;
}
/****************************************************************************
 * Change the character that pops up the menu
 */
static void new_esc(screen,plate)
struct screen * screen;
struct template * plate;
{
    int i;
    register char *x1;
    char * desc;
    if ((x1 = find_member( &(plate->field_cntrl),0)) == (char *) NULL)
    {
        mess_write(screen,plate,"No character entered");
        return;
    }
    TRAIL_SPACE_STRIP(x1);
    if (*x1 == '\0')
    {
        mess_write(screen,plate,"No character entered");
        return;
    }
    pg.cur_esc_char = (short int) *x1;
    return;
}
/***************************************************************************
 * Accept a new redraw string
 */
static void new_rdw(screen,plate)
struct screen * screen;
struct template * plate;
{
    int i;
    register char *x1;
    char * desc;
    if ((x1 = find_member( &(plate->field_cntrl),0)) == (char *) NULL)
    {
        mess_write(screen,plate,"No string entered");
        return;
    }
    TRAIL_SPACE_STRIP(x1);
    chg_redraw(x1);
    return;
}
/**********************************************************************
 * Initialise the forced match capability
 */
static void start_force(screen,plate)
struct screen * screen;
struct template * plate;
{
    char * x;
    int i;
    if ((x = find_member( &(plate->field_cntrl),0)) != (char *) NULL)
    {
        i = atoi(x);
        if (i != 0)
            ini_force(i);
    }
    return;
}
