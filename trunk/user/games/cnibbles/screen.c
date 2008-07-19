//          _       _________ ______   ______   _        _______  _______  _
//         ( (    /|\__   __/(  ___ \ (  ___ \ ( \      (  ____ \(  ____ \( )
//  ______ |  \  ( |   ) (   | (   ) )| (   ) )| (      | (    \/| (    \/| |
// (  ___ \|   \ | |   | |   | (__/ / | (__/ / | |      | (__    | (_____ | |
// | (   \/| (\ \) |   | |   |  __ (  |  __ (  | |      |  __)   (_____  )| |
// | |     | | \   |   | |   | (  \ \ | (  \ \ | |      | (            ) |(_)
// | (___/\| )  \  |___) (___| )___) )| )___) )| (____/\| (____/\/\____) | _
// (______/|/    )_)\_______/|/ \___/ |/ \___/ (_______/(_______/\_______)(_)
//                                                              Version 2.0.0
//  
// Copyright (C), Daniel Aarno <macbishop@users.sf.net> - All rights reserved 
//         Licensed under the Academic Free License version 1.2 

// cNibbles is a curses based version of the old nibbles game (also known as 
// snake). Your object is to control the worm and help it eat apples 
// distributed on the playing area. 
//
// Use the arrow keys (h,j,k,l alternatively) to move the worm. The m and z 
// keys can be used for relative movement. The q key quits the game and the p 
// key pauses the game. 


#include "misc.h"
#include "screen.h"
#include "options.h"

bool g_hasColors;

void
endgraphics (WINDOW * w[])
{
     delwin (w[Main]);
     delwin (w[Grid]);
     delwin (w[Status]);

     if (endwin () == ERR)
	  exit (-1);
}

void
startgraphics (WINDOW * w[])
{
     // to put the nibbles-window in the center of the screen
     int offsetx, offsety;
     int loop, lx, ly;

     clear ();
     refresh ();

     if ((COLS < WINDOWX) || (LINES < WINDOWY)) {
	  endwin ();
	  fprintf (stderr,
		   "A terminal with at least %d lines and %d columns is required.\n",
		   WINDOWY, WINDOWX);
	  exit (1);
     }

     offsetx = (COLS - WINDOWX) >> 1;
     offsety = (LINES - WINDOWY) >> 1;

     w[Main] = newwin (WINDOWY, WINDOWX, offsety, offsetx);
     w[Grid] = newwin (GRIDY, GRIDX * 2, offsety + 1, offsetx + 1);
     w[Status] = newwin (1, WINDOWX - 2, offsety + WINDOWY - 2, offsetx + 1);
     keypad (w[Grid], TRUE);
     nonl ();
     cbreak ();
     noecho ();
     nodelay (w[Grid], TRUE);
     curs_set (FALSE);

     g_hasColors = has_colors ();
     if (!g_opts.useColor)
	  g_hasColors = false;

     if (!g_hasColors) {
	  fprintf (stderr,
		   "Colors not available, falling back on monochrome. \n");
     }
     if (g_hasColors) {
	  start_color ();

	  // configure colors
	  init_pair (WORMCOLORPAIR, WORMCOLOR, BACKGROUND);
	  init_pair (LINECOLORPAIR, LINECOLOR, BACKGROUND);
	  init_pair (STATUSCOLORPAIR, STATUSCOLOR, BACKGROUND);
	  init_pair (DOTCOLORPAIR, DOTCOLOR, BACKGROUND);
     }
     // draw pretty lines in w[Main]
     if (g_hasColors)
	  wattrset (w[Main], COLOR_PAIR (LINECOLORPAIR) | A_BOLD);
     else
	  wattrset (w[Grid], A_BOLD);

     mvwaddch (w[Main], 0, 0, ACS_ULCORNER);
     mvwaddch (w[Main], 0, WINDOWX - 1, ACS_URCORNER);
     mvwaddch (w[Main], WINDOWY - 1, 0, ACS_LLCORNER);
     mvwaddch (w[Main], WINDOWY - 1, WINDOWX - 1, ACS_LRCORNER);
     for (loop = 1; loop < WINDOWY - 1; loop++) {
	  mvwaddch (w[Main], loop, 0, ACS_VLINE);
	  mvwaddch (w[Main], loop, WINDOWX - 1, ACS_VLINE);
     }
     for (loop = 1; loop < WINDOWX - 1; loop++) {
	  mvwaddch (w[Main], 0, loop, ACS_HLINE);
	  mvwaddch (w[Main], WINDOWY - 3, loop, ACS_HLINE);
	  mvwaddch (w[Main], WINDOWY - 1, loop, ACS_HLINE);
     }
     mvwaddch (w[Main], WINDOWY - 3, 0, ACS_LTEE);
     mvwaddch (w[Main], WINDOWY - 3, WINDOWX - 1, ACS_RTEE);
     wrefresh (w[Main]);

     // clear the status screen
     if (g_hasColors)
	  wattrset (w[Status], COLOR_PAIR (STATUSCOLORPAIR) | A_BOLD);
     else
	  wattrset (w[Grid], A_BOLD);

     for (loop = 0; loop <= GRIDX; loop++) {
	  clearblock (w, loop, 0);
     }
     wrefresh (w[Status]);

     // clear the grid screen
     if (g_hasColors)
	  wattrset (w[Grid], COLOR_PAIR (WORMCOLORPAIR) | A_BOLD);
     else
	  wattrset (w[Grid], A_BOLD);

     for (lx = 0; lx <= GRIDX; lx++) {
	  for (ly = 0; ly <= GRIDY; ly++) {
	       clearblock (w, lx, ly);
	  }
     }
     wrefresh (w[Grid]);
}

void
drawblock (WINDOW * w[], int x, int y)
{
     mvwaddch (w[Grid], y, x * 2, ACS_BLOCK);
     mvwaddch (w[Grid], y, x * 2 + 1, ACS_BLOCK);
}

void
drawdot (WINDOW * w[], int x, int y)
{
     char c;

     if (ACS_BLOCK == '#')
	  c = '0';
     else
	  c = '#';

     if (g_hasColors)
	  wattrset (w[Grid], COLOR_PAIR (DOTCOLORPAIR) | A_BOLD);
     else
	  wattrset (w[Grid], A_BOLD);

     mvwaddch (w[Grid], y, x * 2, c);
     mvwaddch (w[Grid], y, x * 2 + 1, c);

     if (g_hasColors)
	  wattrset (w[Grid], COLOR_PAIR (WORMCOLORPAIR) | A_BOLD);
     else
	  wattrset (w[Grid], A_BOLD);
}

void
clearblock (WINDOW * w[], int x, int y)
{
     mvwaddch (w[Grid], y, x * 2, ' ');
     mvwaddch (w[Grid], y, x * 2 + 1, ' ');
}

void
newdot (WINDOW * w[], bool grid[GRIDX][GRIDY], int *x, int *y)
{
     do {
	  *x = randint (0, GRIDX - 1);
	  *y = randint (0, GRIDY - 1);
     } while (grid[*x][*y]);

     drawdot (w, *x, *y);

}
