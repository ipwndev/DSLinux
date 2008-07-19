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


#include <signal.h>
#include <sys/time.h>
#include <string.h>		//memset, memcpy

#include "misc.h"
#include "screen.h"
#include "options.h"
#include "highscore.h"
#include "replay.h"
#include "intro.h"

FILE *errFp;

int
quit (int error)
{
     clear ();
     refresh ();
     endwin ();
     exit (error);
}

void
sig_handler (int signal)
{
     switch (signal) {
     case SIGTERM:
	  quit (0);
	  break;

     default:
	  fprintf (errFp, "Caught an unknown signal %d, exiting\n", signal);
	  break;
     }
}

int
get_dir (int ch, int lastdir)
{
     int dir = lastdir;

     switch (ch) {
     case 'z':
	  switch (lastdir) {
	  case UP:
	       dir = LEFT;
	       break;

	  case DOWN:
	       dir = RIGHT;
	       break;

	  case LEFT:
	       dir = DOWN;
	       break;

	  case RIGHT:
	       dir = UP;
	       break;
	  }
	  break;

     case 'm':
	  switch (lastdir) {
	  case UP:
	       dir = RIGHT;
	       break;

	  case DOWN:
	       dir = LEFT;
	       break;

	  case LEFT:
	       dir = UP;
	       break;

	  case RIGHT:
	       dir = DOWN;
	       break;
	  }
	  break;

     case KEY_LEFT:
     case 'h':			// accept vi-style directions as well
	  if ((lastdir == UP) || (lastdir == DOWN)) {
	       dir = LEFT;
	  }
	  break;
     case KEY_RIGHT:
     case 'l':
	  if ((lastdir == UP) || (lastdir == DOWN)) {
	       dir = RIGHT;
	  }
	  break;
     case KEY_UP:
     case 'k':
	  if ((lastdir == LEFT) || (lastdir == RIGHT)) {
	       dir = UP;
	  }
	  break;
     case KEY_DOWN:
     case 'j':
	  if ((lastdir == LEFT) || (lastdir == RIGHT)) {
	       dir = DOWN;
	  }
	  break;
     }

     return dir;
}

int
game_loop (WINDOW * w[])
{
     // The grid, to make it easy to tell where the worm is
     bool grid[GRIDX][GRIDY];

     // The worm history
     short int hx[HISTORYSIZE];
     short int hy[HISTORYSIZE];

     // Worm variables
     int length = STARTLENGTH;	// the length of the worm
     int extralength = STARTELENGTH;	// how much the worm is going to grow
     int curx, cury;		// current position of the worm
     int dir = RIGHT;		// direction the worm is facing
     int lastdir = RIGHT;	// last direction
     int score = 0;
     bool paused = false;
     int ch = ERR;		//User input

     int dotx, doty;		// position of the dot (worm food)

     struct timeval theTime;	//used to get the current time
     struct timeval loopTime;	//used to calc the exec time of the main loop
     int tick;			// current tick
     unsigned int tickFix = 0;	//adjust the sleep time acording to the time
     //required to execute main loop, this helps
     //equalize the speed over different machines
     int missedDeadlines = 0;	//increase by one if the main loop takes longer
     //than the supposed sleep time. Makes it 
     //possible to detect too slow machines

     // clear the grid
     memset (grid, 0, sizeof (grid));

     // Initialize and draw the worm
     curx = (GRIDX + length) >> 1;
     cury = GRIDY >> 1;
     for (tick = 0; tick < length; tick++) {
	  hx[tick] = curx - (length - tick) + 1;
	  hy[tick] = cury;
	  grid[curx - (length - tick) + 1][cury] = true;
	  drawblock (w, curx - (length - tick) + 1, cury);
     }

     // Initialize and draw the dot (worm food)
     newdot (w, grid, &dotx, &doty);
     add_dot_to_history (dotx, doty);
     wrefresh (w[Grid]);

     sleep (1);			//Wait so the user gets ready

     // Main loop
     while (1) {
	  lastdir = dir;

	  if (paused) {
	       ch = wgetch (w[Grid]);
	       while (ch != ERR) {
		    switch (ch) {
		    case 'p':
			 paused = false;
			 mvwprintw (w[Status], 0,
				    WINDOWX -
				    2 /* because of the border lines */  -
				    7 /* strlen("PAUSED ") */ , "      ");
			 break;
		    case 'q':
//                       do_highscore (score);
			 return score;
			 break;
		    }
		    ch = wgetch (w[Grid]);
	       }
	       mvwprintw (w[Status], 0, 0, "Score:  %d ", score);
	       mvwprintw (w[Status], 0,
			  WINDOWX - 2 /* because of the border lines */  -
			  7 /* strlen("PAUSED ") */ , "Paused");
	       wrefresh (w[Status]);
	       usleep (TICKSPEED);
	       continue;
	  }			//END PAUSE

	  // catch user input
	  ch = wgetch (w[Grid]);
	  while (ch != ERR) {
	       switch (ch) {
	       case 'q':
//                  do_highscore (score);
		    return score;
		    break;

	       case 'p':
		    paused = true;
		    break;

	       default:
		    dir = get_dir (ch, lastdir);
		    break;
	       }
	       ch = wgetch (w[Grid]);
	  }

	  add_to_history (dir);

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

	  // check whether or not the worm has crashed in a wall
	  if ((curx < 0) || (curx >= GRIDX) || (cury < 0) || (cury >= GRIDY)) {
//             do_highscore (score);
	       return score;
	  }
	  // check whether or not the worm has crashed with itself
	  if (grid[curx][cury]) {
//             do_highscore (score);
	       return score;
	  }
	  // the worm is still on the grid, and it seems healty :)

	  drawblock (w, curx, cury);	// let's draw the new block
	  grid[curx][cury] = true;	// and update the grid

	  // update the history array
	  hx[tick] = curx;
	  hy[tick] = cury;

	  // has the worm eaten food?
	  if ((dotx == curx) && (doty == cury)) {
	       beep ();
	       score++;
	       extralength += WORMGROWTH;
	       newdot (w, grid, &dotx, &doty);
	       add_dot_to_history (dotx, doty);
	  }

	  if (g_opts.growsWithTime && !(tick % 25)) {
	       extralength = 1;
	  }
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

	  // finally, update the status line
	  mvwprintw (w[Status], 0, 0, "Score:  %d", score);
	  mvwprintw (w[Status], 0,
		     GRIDX - 2 /* because of the border lines */  -
		     7 /* strlen("PAUSED ") */ , "       ");
	  wrefresh (w[Status]);

	  gettimeofday (&loopTime, NULL);	//Get current time
	  loopTime.tv_sec -= theTime.tv_sec;	//How many sec since loop start
	  loopTime.tv_usec -= theTime.tv_usec;	//How many usec since loop start
	  //Get the time in usec that have elapsed sice we woke from sleep
	  tickFix = loopTime.tv_sec * 1000000 + loopTime.tv_usec;

	  if (tickFix > TICKSPEED - ((g_opts.speed - 2) * TICKSPEED / 4)) {
	       //If we are here the time limit for the main loop was
	       //exeded! Set tickFix so we don't sleep and inc missedDeadlines
	       tickFix = TICKSPEED - (g_opts.speed - 2) * TICKSPEED / 4;
	       missedDeadlines++;
	  } else
	       missedDeadlines = 0;

	  if (missedDeadlines > 10) {
	       fprintf (stderr,
			"Your machine is too slow to play this game!\n");
	       quit (2);
	  }
	  //sleep our sleeptime
	  usleep (TICKSPEED - ((g_opts.speed - 2) * TICKSPEED / 4) - tickFix);

	  gettimeofday (&theTime, NULL);

	  //      add_to_history(dir);
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
}

int
main (int argc, const char *argv[])
{
     bool done = false;
     int i, score;
     WINDOW *w[3];		// The tree windows (Main,Grid,Status in that order) 
     //used by ncurses

     errFp = stderr;
     parse_options (argc, argv);

     signal (SIGTERM, sig_handler);

     randomize ();		// Seed the random number generator

     initscr ();		//Initialize ncurses

     clear ();
     refresh ();
     nodelay (stdscr, TRUE);
     cbreak ();

//Run intro

     startgraphics (w);
     show_intro (w);
     endgraphics (w);

//Start game

     while (!done) {
	  while (getch () != ERR)	//Clear buffer
	       ;
	  printw ("\nWhat difficulty do you want (1-4) [%d]? ", g_opts.speed);
	  while ((i = getch ()) == ERR) {
	       usleep (100000);
	  }

	  switch (i) {
	  case '1':
	       g_opts.speed = 1;
	       break;

	  case '2':
	       g_opts.speed = 2;
	       break;

	  case '3':
	       g_opts.speed = 3;
	       break;

	  case '4':
	       g_opts.speed = 4;
	       break;
	  }


/////////////////////////////////////////////////////////
	  noecho ();
	  startgraphics (w);	// Initialize the windows

	  score = game_loop (w);

	  endgraphics (w);
	  echo ();
//////////////////////////////////////////////////////////

	  nodelay (stdscr, FALSE);
//      nocbreak();

	  do_highscore (score);

	  cbreak ();
	  nodelay (stdscr, TRUE);

	  while (getch () != ERR)	//Clear buffer
	       ;
	  printw ("\nDo you want to see the replay [y/N]? ");


	  while ((i = getch ()) == ERR) {
	       usleep (100000);
	  }

	  if (i == 'y' || i == 'Y') {
	       startgraphics (w);	// Initialize the windows
	       show_replay (w);
	       endgraphics (w);	//Destroy the windows
	       clear ();
	       refresh ();
	  }


	  while (getch () != ERR)	//Clear buffer
	       ;

	  printw ("\nDo you want to see the BEST replay [y/N]? ");

	  while ((i = getch ()) == ERR) {
	       usleep (100000);
	  }

	  if (i == 'Y' || i == 'y') {
	       startgraphics (w);	// Initialize the windows
	       show_best_replay (w);
	       endgraphics (w);	//Destroy the windows
	       clear ();
	       refresh ();
	  }


	  reset_history ();
	  while (getch () != ERR)	//Clear buffer
	       ;
	  printw ("\nDo you want to play again [Y/n]? ");

	  while ((i = getch ()) == ERR) {
	       usleep (100000);
	  }
	  if (i == 'n' || i == 'N')
	       quit (0);

//    endgraphics(w);  //Destroy the windows
//    startgraphics (w);  // Initialize the windows

//    show_replay(w);
//    reset_history();

//        endgraphics (w);      //Destroy the windows
     }

     clear ();
     refresh ();
     quit (0);

     return 0;
}
