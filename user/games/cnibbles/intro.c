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

/************************************************************************/
//This file is a complete hack, you're probably better off not looking at
//it :-). If you want to do an intro, you should probably start from 
//scratch.
/************************************************************************/

#include "intro.h"
#include <assert.h>
#include <string.h>
#include <sys/time.h>

//must have an even number of chars
const char *prgrm_name = "cNibbles";
const char *ver_str = _TEXT_VERSION_;
const char *copyright_str =
     "Copyright (C) 2003 by Daniel Aarno - All rights reserved";
const char *license_str =
     "Licensed under the Academic Free License version 1.2";
const char *ported_str =
     "(Ported by Agilo for DSLinux) ";

void
show_intro (WINDOW * w[])
{
     // The grid, to make it easy to tell where the worm is
     bool grid[GRIDX][GRIDY];

     // The worm history
     short int hx[HISTORYSIZE];
     short int hy[HISTORYSIZE];
     short step = 0;
     short mode = 0;
     short str_idx = 0;

     // Worm variables
     int length = 1;		// the length of the worm
     int extralength = 6;	// how much the worm is going to grow
     int curx, cury;		// current position of the worm
     int dir = RIGHT;		// direction the worm is facing
     int lastdir = RIGHT;	// last direction

     struct timeval theTime;	//used to get the current time
     struct timeval loopTime;	//used to calc the exec time of the main loop
     int tick=0;			// current tick
     unsigned int tickFix = 0;	//adjust the sleep time acording to the time
     //required to execute main loop, this helps
     //equalize the speed over different machines
     int missedDeadlines = 0;	//increase by one if the main loop takes longer
     //than the supposed sleep time. Makes it 
     //possible to detect too slow machines

     int tmp;

     // clear the grid
     memset (grid, 0, sizeof (grid));

     // Initialize and draw the worm
     curx = 0;
     cury = (GRIDY >> 1) - 5;

     // Main loop
     for (step = 0, dir = RIGHT; step < (GRIDX + length) * 3; step++) {
	  lastdir = dir;

	  tmp = (GRIDX - strlen (prgrm_name)) >> 1;

	  if ((curx - length + 1 >
	       (GRIDX - (int) strlen (prgrm_name) / 2) >> 1)
	      && (curx - length + 1 <=
		  (GRIDX + (int) strlen (prgrm_name) / 2) >> 1)
	      && dir == RIGHT && mode == 0) {
	       mvwprintw (w[Grid], cury, (curx - length) * 2 - 1, "%c",
			  prgrm_name[str_idx++]);
	       mvwprintw (w[Grid], cury, (curx - length) * 2, "%c",
			  prgrm_name[str_idx++]);

	  }

	  if (curx + length + 1 < (GRIDX + (int) strlen (ver_str) / 2) >> 1 &&
	      curx + length + 1 >= (GRIDX - (int) strlen (ver_str) / 2) >> 1
	      && dir == LEFT && mode == 1) {

	       mvwprintw (w[Grid], cury, (curx + length) * 2 + 1,
			  "%c", ver_str[str_idx--]);
	       mvwprintw (w[Grid], cury, (curx + length) * 2,
			  "%c", ver_str[str_idx--]);

	  }

	  if (curx >= GRIDX - 1 && mode == 0) {
	       curx--;
	       if (step >= GRIDX + length) {
		    length = 1;
		    extralength = 6;
		    dir = LEFT;
		    curx = GRIDX;
		    cury += 2;
		    str_idx = strlen (ver_str) - 1;
		    mode = 1;
	       }
	  }

	  if (curx == 0 && mode == 1) {
	       curx++;
	       if (step >= (GRIDX + length) * 2) {
		    cury = (GRIDY >> 1) - 6;
		    curx = 0;
		    dir = RIGHT;
		    length = 6;
		    mode = 2;
	       }

	  }

	  if (curx == (GRIDX + strlen (prgrm_name) / 2) / 2 && mode == 2) {
	       extralength = strlen (prgrm_name) * 2 + 5 - length;
	  }

	  if (curx == (GRIDX + strlen (prgrm_name) / 2) / 2 && mode == 2) {
	       dir = DOWN;
	       mode = 3;
	  }

	  if (mode >= 3 && mode <= 5 + 3) {
	       mode++;
	       if (mode == 5 + 3)
		    dir = LEFT;
	  }

	  if (mode >= 3 && dir == LEFT) {
	       if (curx < (GRIDX - strlen (prgrm_name) / 2) / 2)
		    break;
	  }
	  // modify the new position based on the direction
	  switch (dir) {
	  case LEFT:
	       curx--;
	       break;
	  case RIGHT:
	       curx++;
	       break;
	  case UP:
	       cury--;
	       break;
	  case DOWN:
	       cury++;
	       break;
	  }

	  drawblock (w, curx, cury);	// let's draw the new block
	  grid[curx][cury] = true;	// and update the grid

	  // update the history array
	  hx[tick] = curx;
	  hy[tick] = cury;

	  // if the worm still has some growing to do..
	  if (extralength > 0) {
	       extralength--;
	       length++;
	  } else {
	       // otherwise, clear the last block
	       clearblock (w, hx[tick - length], hy[tick - length]);
	       grid[hx[tick - length]][hy[tick - length]] = 0;
	  }

	  wrefresh (w[Grid]);

	  gettimeofday (&loopTime, NULL);	//Get current time
	  loopTime.tv_sec -= theTime.tv_sec;	//How many sec since loop start
	  loopTime.tv_usec -= theTime.tv_usec;	//How many usec since loop start
	  //Get the time in usec that have elapsed sice we woke from sleep
	  tickFix = loopTime.tv_sec * 1000000 + loopTime.tv_usec;

	  if (tickFix > TICKSPEED - ((5 - 2) * TICKSPEED / 4)) {
	       //If we are here the time limit for the main loop was
	       //exeded! Set tickFix so we don't sleep and inc missedDeadlines
	       tickFix = TICKSPEED - (5 - 2) * TICKSPEED / 4;
	       missedDeadlines++;
	  } else
	       missedDeadlines = 0;

	  //sleep our sleeptime
	  usleep (TICKSPEED - ((5 - 2) * TICKSPEED / 4) - tickFix);

	  gettimeofday (&theTime, NULL);

	  tick++;

	  // if the history array is full, copy the length of the worm
	  // to the beginning of the array and decrease the tick counter
	  if (tick == HISTORYSIZE) {
	       memcpy (hx, hx + HISTORYSIZE - length,
		       length * sizeof (short int));
	       memcpy (hy, hy + HISTORYSIZE - length,
		       length * sizeof (short int));
	       tick = length;
	  }
	  // ready for next tick!
     }

//Display copyright...

     mvwprintw (w[Grid], GRIDY / 2 + 2, (GRIDX - strlen (copyright_str) / 2),
		"%s", copyright_str);
     wrefresh (w[Grid]);
     usleep (1000000);
     mvwprintw (w[Grid], GRIDY / 2 + 3, (GRIDX - strlen (license_str) / 2),
		"%s", license_str);
     wrefresh (w[Grid]);
     usleep (1000000);
     mvwprintw (w[Grid], GRIDY / 2 + 4, (GRIDX - strlen (ported_str) / 2),
                "%s", ported_str);
     wrefresh (w[Grid]);
     usleep (2000000);
     clear ();
     refresh ();
}
