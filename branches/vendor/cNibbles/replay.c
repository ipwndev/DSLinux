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


#include "replay.h"
#include <assert.h>
#include <string.h>
#include <sys/time.h>

static short g_step = 0;
static ReplayNode *g_first = NULL;

extern const char highscoreDirName[];
extern const char replayFileName[];

void
show_replay (WINDOW * w[])
{
     ReplayNode *current = g_first;

     // The grid, to make it easy to tell where the worm is
     bool grid[GRIDX][GRIDY];

     // The worm history
     short int hx[HISTORYSIZE];
     short int hy[HISTORYSIZE];
     short step = 0;

     // Worm variables
     int length = STARTLENGTH;	// the length of the worm
     int extralength = STARTELENGTH;	// how much the worm is going to grow
     int curx, cury;		// current position of the worm
     int dir = RIGHT;		// direction the worm is facing
     int lastdir = RIGHT;	// last direction
     int score = 0;
     int ch;			//User input

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
     //  newdot (w, grid, &dotx, &doty);
     ch = next_dir (current, step++);
     dotx = next_dir (current, step++);
     doty = next_dir (current, step++);
     drawdot (w, dotx, doty);
     refresh ();

     sleep (1);			//Wait so the user gets ready

     // Main loop
     while (current != NULL) {
	  lastdir = dir;

	  dir = next_dir (current, step++);
	  if (dir == -1)
	       quit (0);

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
	       return;
	  }
	  // check whether or not the worm has crashed with itself
	  if (grid[curx][cury]) {
	       return;
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
	       //      newdot (w, grid, &dotx, &doty);
	       if (next_dir (current, step++) != -1)
		    quit (-1);

	       dotx = next_dir (current, step++);
	       doty = next_dir (current, step++);
	       drawdot (w, dotx, doty);
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

	  if (step >= 100) {
	       current = current->next;
	       step -= 100;
	  }
	  // ready for next tick!
     }
}

int
next_dir (ReplayNode * cur, short step)
{
     if (step >= 100) {
	  return cur->next->dir[step - 100];
     }

     return cur->dir[step];
}

void
save_replay (char *fileName)
{
     FILE *fp;
     long version = _VERSION_;
     ReplayNode *next, *current;

     assert (fileName != NULL);
     assert (g_first != NULL);

     fp = fopen (fileName, "wb");
     if (fp == NULL) {
	  clear ();
	  refresh ();
	  printw ("Could not open file %s for writing\n", fileName);
	  quit (errno);
     }

     fwrite (&version, sizeof (long), 1, fp);	//write version info

     current = g_first;

     while (current != NULL) {
	  next = current->next;
	  current->next = NULL;
	  fwrite (current, sizeof (ReplayNode), 1, fp);
	  current->next = next;
	  current = next;
     }

     fclose (fp);

}

void
load_replay (char *fileName)
{
     FILE *fp;
     long version;
     char buf[30] = { 0 };
     ReplayNode *current, *next, *prev;

     assert (fileName != NULL);

     reset_history ();
     fp = fopen (fileName, "rb");
     if (fp == NULL) {
	  clear ();
	  printw ("Could not open file %s for reading\n", fileName);
	  return;		//ignore errors
     }

     fread (&version, sizeof (long), 1, fp);
     if (version > _VERSION_) {
	  sprintf (buf, "%ld", (version & 0xFF0000));
	  buf[strlen (buf)] = '.';
	  sprintf (buf + strlen (buf), "%ld", (version & 0x00FF00));
	  buf[strlen (buf)] = '.';
	  sprintf (buf + strlen (buf), "%ld", (version & 0x0000FF));

	  clear ();
	  printw ("Replay file from newer version (v%s) %s",
		  buf, "than this (v%s) exists\n", _TEXT_VERSION_);
     }

     g_first = (ReplayNode *) malloc (sizeof (ReplayNode));
     current = g_first;
     prev = NULL;
     next = NULL;

     while (fread (current, sizeof (ReplayNode), 1, fp) == 1) {
	  prev = current;
	  next = (ReplayNode *) malloc (sizeof (ReplayNode));
	  next->next = NULL;
	  current->next = next;
	  current = next;
     }

     free (current);
     prev->next = NULL;

}

void
reset_history (void)
{
     g_step = 0;

     if (g_first != NULL)
	  delete_history ();

     g_first = NULL;
}

void
delete_history (void)
{
     ReplayNode *current, *next;

     current = g_first;

     while (current != NULL) {
	  next = current->next;
	  free (current);
	  current = next;
     }
}

void
add_to_history (char dir)
{
     static ReplayNode *current = NULL;

     if (g_first == NULL) {
	  g_first = (ReplayNode *) calloc (sizeof (ReplayNode), 1);	//use calloc for 
	  //automatic lazy 
	  //zero of memory
	  current = g_first;
     }

     if (g_step >= 100) {
	  current->next = (ReplayNode *) calloc (sizeof (ReplayNode), 1);
	  current = current->next;
	  g_step = 0;
     }

     current->dir[g_step++] = dir;
}

void
add_dot_to_history (char dotx, char doty)
{
     add_to_history (-1);
     add_to_history (dotx);
     add_to_history (doty);
}

void
show_best_replay (WINDOW * w[])
{
     char *path, *homeDir;

     homeDir = getenv ("HOME");

     path = malloc (sizeof (char) * (strlen (homeDir) +
				     strlen (highscoreDirName) +
				     strlen (replayFileName) + 1));

     strcpy (path, homeDir);
     strcpy (path + strlen (homeDir), highscoreDirName);
     strcpy (path + strlen (homeDir) + strlen (highscoreDirName),
	     replayFileName);

     reset_history ();
     load_replay (path);
     free (path);
     show_replay (w);
}
