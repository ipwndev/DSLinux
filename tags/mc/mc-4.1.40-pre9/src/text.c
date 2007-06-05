/*
 * Text edition support code
 *
 *
 */
#include <config.h>
#include <stdio.h>

#define WANT_WIDGETS
#include "win.h"
#include "tty.h"
#include "key.h"
#include "widget.h"
#include "main.h"
#include "cons.saver.h"

char *default_edition_colors =
"normal=lightgray,black:"
"selected=black,cyan:"
"marked=yellow,black:"
"markselect=yellow,cyan:"
"errors=white,red:"
"menu=black,lightgray:"
"reverse=black,lightgray:"
"dnormal=black,lightgray:"
"dfocus=black,cyan:"
"dhotnormal=blue,lightgray:"
"dhotfocus=blue,cyan:"
"viewunderline=white,brown:"
"menuhot=red,lightgray:"
"menusel=black,green:"
"menuhotsel=red,green:"
"helpnormal=black,lightgray:"
"helpitalic=magenta,lightgray:"
"helpbold=blue,lightgray:"
"helplink=red,lightgray:"
"helpslink=yellow,red:"
"gauge=white,black:"
"input=black,cyan:"
"inputdef=gray,cyan:"
"directory=white,black:"
"execute=brightgreen,black:"
"link=blue,black:"
"device=magenta,black:"
"core=red,black:"
"special=brightcyan,blue:"
"hidden=gray,black:"
"doc=green,black:"
"temp=brightred,black:"
"archive=brightmagenta,black:"
"source=cyan,black:"
"media=brown,black:"
"graph=brightblue,black:"
"database=brightcyan,black:"
"editnormal=brightcyan,black:"
"editbold=yellow,black:"
"editmarked=black,cyan:"
"statmountpoint=brightcyan,black:"
"statavail=brightgreen,black:"
"stattotal=green,black";

char *default_edition_colors_classic =
"normal=lightgray,blue:"
"selected=black,cyan:"
"marked=yellow,blue:"
"markselect=yellow,cyan:"
"errors=white,red:"
"menu=white,cyan:"
"reverse=black,lightgray:"
"dnormal=black,lightgray:"
"dfocus=black,cyan:"
"dhotnormal=blue,lightgray:"
"dhotfocus=yellow,cyan:"
"viewunderline=brightred,blue:"
"menuhot=yellow,cyan:"
"menusel=white,black:"
"menuhotsel=yellow,black:"
"helpnormal=black,lightgray:"
"helpitalic=red,lightgray:"
"helpbold=blue,lightgray:"
"helplink=black,cyan:"
"helpslink=yellow,blue:"
"gauge=white,black:"
"input=black,cyan:"
"inputdef=gray,cyan:"
"directory=white,blue:"
"execute=brightgreen,blue:"
"link=lightgray,blue:"
"device=magenta,blue:"
"core=red,blue:"
"special=brightcyan,black:"
"hidden=black,blue:"
"doc=green,blue:"
"temp=brightred,blue:"
"archive=brightmagenta,blue:"
"source=cyan,blue:"
"media=brown,blue:"
"graph=brightblue,blue:"
"database=brightcyan,blue:"
"editnormal=lightgray,blue:"
"editbold=yellow,blue:"
"editmarked=black,cyan:"
"statmountpoint=brightcyan,blue:"
"statavail=brightgreen,blue:"
"stattotal=green,blue";


void
edition_post_exec (void)
{
    do_enter_ca_mode ();

    /* FIXME: Missing on slang endwin? */
    reset_prog_mode ();
    flushinp ();

    keypad (stdscr, TRUE);
    mc_raw_mode ();
    channels_up ();
    if (use_mouse_p)
	init_mouse ();
    if (alternate_plus_minus)
        application_keypad_mode ();
}

void
edition_pre_exec (void)
{
    if (clear_before_exec)
	clr_scr ();
    else {
	if (!(console_flag || xterm_flag))
	    printf ("\n\n");
    }

    channels_down ();
    if (use_mouse_p)
	shut_mouse ();

    reset_shell_mode ();
    keypad (stdscr, FALSE);
    endwin ();
    
    numeric_keypad_mode ();
    
    /* on xterms: maybe endwin did not leave the terminal on the shell
     * screen page: do it now.
     *
     * Do not move this before endwin: in some systems rmcup includes
     * a call to clear screen, so it will end up clearing the sheel screen.
     */
    if (!status_using_ncurses) {
	do_exit_ca_mode ();
    }
}

void
clr_scr (void)
{
    standend ();
    dlg_erase (midnight_dlg);
    mc_refresh ();
    doupdate ();
}

