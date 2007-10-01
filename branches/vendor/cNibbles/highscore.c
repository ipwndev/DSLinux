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

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#include "highscore.h"
#include "misc.h"
#include "screen.h"
#include "options.h"
#include "replay.h"

typedef struct {
     char name[32];
     short score;
} HighscoreEntry;

const char highscoreFileName[] = "/hghscr";
const char replayFileName[] = "/replay";
const char incWithTimeSuffix[] = "_time";
const char highscoreDirName[] = "/.cNibbles";

HighscoreEntry hi[4 * NUM_HIGHSCORE_ENTRY];

void
do_highscore (short pnts)
{
     char c, *rply_path, *homeDir;
     int i, place;

     read_highscore ();		//Load hiscore from file

     place = get_highscore_place (pnts);	//Get position in hi, and prepare hi for 
     //writing at position place
     clear ();

     if (place < 0) {		//We didn't make the hiscr
	  show_highscore ();
	  printw ("\n\nYour score was: %d\n\n", pnts);
	  refresh ();

	  return;
     }
     //////////////////////////
     //Write out some user info
     //////////////////////////
     clear ();
//     echo ();
     printw ("Congratulations, you made the highscore!\n\n");
     printw ("Please enter your name [????]:> ");
     nocbreak ();
     refresh ();

     /////////////////////////////
     //Read the name of the player
     /////////////////////////////
     i = 0;
     while ((c = getch ()) != '\n' && i < 31) {
	  if (c < '!' || c > 126)	//ignore unwritable chars
	       continue;
	  hi[place].name[i++] = c;
     }
     hi[place].name[i] = '\0';

     cbreak ();

     if (hi[place].name[0] == '\0')
	  strcpy (hi[place].name, "????");	//Set default name, if none was given

     hi[place].score = pnts;

     write_highscore (0);	//Flush our hiscore to disk

     if (place == NUM_HIGHSCORE_ENTRY * g_opts.speed - 1) {
	  homeDir = getenv ("HOME");
	  if (homeDir == NULL) {
	       clear ();
	       refresh ();
	       printw ("Could not locate users home directory!\n");
	       quit (-1);
	  }
	  rply_path = calloc (sizeof (char) * (strlen (homeDir)
					       + strlen (highscoreDirName) +
					       strlen (replayFileName) + 1),
			      1);

	  memcpy (rply_path, homeDir, strlen (homeDir) * sizeof (char));
	  memcpy (rply_path + strlen (homeDir), highscoreDirName,
		  strlen (highscoreDirName) * sizeof (char));
	  memcpy (rply_path + strlen (homeDir) +
		  strlen (highscoreDirName), replayFileName,
		  strlen (replayFileName) * sizeof (char));
	  save_replay (rply_path);
	  free (rply_path);
     }

     printw ("\n\n");

     show_highscore ();		//Display the highscore

     printw ("\n\nYour score was: %d\n\n", pnts);
     refresh ();

     return;
}

//NOTE: This function also shifts the hiscr table to make room for a new
//entry, however it is not flushed to disk so it is possible to reload it.
short
get_highscore_place (short score)
{
     int i, offset;

     offset = NUM_HIGHSCORE_ENTRY * (g_opts.speed - 1);
     for (i = offset; i < NUM_HIGHSCORE_ENTRY + offset; i++) {
	  if (hi[i].score < score) {
	       //shift hi entry down, if it is not the "lowest" one
	       if (i == offset)
		    continue;

	       memcpy (&hi[i - 1], &hi[i], sizeof (HighscoreEntry));
	  } else
	       return (i <= offset ? -1 : i - 1);
     }

     return i - 1;
}

void
show_highscore ()
{
     HighscoreEntry *hiEnt;
     int i, j, n;
     char buf[9];

//     endwin ();
     read_highscore ();

     printw ("-*-*-*-*-*-*   HIGHSCORE   *-*-*-*-*-*-\n");
     if (g_opts.growsWithTime)
	  printw ("             T* LEVEL %d *T\n\n", g_opts.speed);
     else
	  printw ("                LEVEL %d\n\n", g_opts.speed);

     printw ("Name                              Score\n");
     printw ("----                              -----\n");
     for (i = 0; i < NUM_HIGHSCORE_ENTRY; i++) {
	  hiEnt = &hi[NUM_HIGHSCORE_ENTRY * (g_opts.speed) - i - 1];
	  printw ("%s", hiEnt->name);
	  sprintf (buf, "%d", hiEnt->score);
	  n = strlen (hiEnt->name) + strlen (buf);
	  for (j = 39; j > n; j--)
	       printw (" ");
	  printw ("%s\n", buf);
     }

     printw ("\n");

     refresh ();

     /*
        char buf[9], spc[40] = {' '};
        int i, j, k, l;

        endgraphics();
        read_highscore();

        printf("Level 1           Level 2\n");
        printf("-------           -------\n");

        for(j = 0; j < NUM_HIGHSCORE_ENTRY; j++) {
        ///////////////////////
        //Print level 1
        //////////////////////
        printf("%s", hi[j].name);  //print the name of level 1[i]
        sprintf(buf, "%d", hi[j].score);  //put the score in buf
        k = strlen(hi[j].name) + strlen(buf);
        k = 38 - k;
        spc[k] = '\0';
        printf("%s%s  ", hi[j].name, buf);
        spc[k] = ' ';

        /////////////////////////
        //Begin level 2
        /////////////////////////
        j += NUM_HIGHSCORE_ENTRY;  //ofset into level 2
        printf("%s", hi[j].name);  //print the name of level 1[i]
        sprintf(buf, "%d", hi[j].score);  //put the score in buf
        k = strlen(hi[j].name) + strlen(buf);
        k = 38 - k;
        spc[k] = '\0';
        printf("%s%s  ", hi[j].name, buf);
        spc[k] = ' ';
        j -= NUM_HIGHSCORE_ENTRY;  //change ofset back to level 1
        }
      */
}

void
write_highscore (int flags)
{
     char *hsFilePath;
     int fd, n;

     hsFilePath = get_highscore_path ();

     fd = open (hsFilePath, O_WRONLY | flags, S_IRWXU);
     if (fd <= 0) {
	  perror ("Could not write highscore file");
	  quit (errno);
     }

     n = write (fd, hi, sizeof (HighscoreEntry) * 4 * NUM_HIGHSCORE_ENTRY);
     if (n != sizeof (HighscoreEntry) * 4 * NUM_HIGHSCORE_ENTRY) {
	  perror ("Error writing to highscore file");
	  quit (errno);
     }

     free (hsFilePath);
     close (fd);
}

void
read_highscore (void)
{
     int highFd;
     char *hsFilePath;
     int n;

     hsFilePath = get_highscore_path ();	//Get path to the hiscr file

     highFd = open (hsFilePath, O_RDONLY);	//Try to open our hiscr file

     if (highFd <= 0) {		//We could not open the file
	  if (errno == ENOENT) {	//no such file, create dir and blank highscore
	       //convert hiscr file path to hiscr dir path
	       n = strlen (hsFilePath) - strlen (highscoreFileName);
	       if (g_opts.growsWithTime)
		    n -= strlen (incWithTimeSuffix);
	       hsFilePath[n] = '\0';
	       if (mkdir (hsFilePath, S_IRWXU)) {	//create the dir and chk for error
		    //if it exists, assume it's a dir and cont. if not, exit
		    if (errno != EEXIST) {
			 perror ("Could not create directory");
			 quit (0);
		    }
	       }
	       //The hiscr dir is now created, let's create the hiscr file
	       blank_highscore ();
	       free (hsFilePath);

	       hsFilePath = get_highscore_path ();	//Get path to the hiscr file
	       highFd = open (hsFilePath, O_RDONLY);	//Try to open our hiscr file again
	       if (highFd <= 0) {	//if this fails here we are FUBAR and must exit
		    perror ("Error opening highscore file");
		    quit (0);
	       }
	  } else {		//Some other reason we could not open the file, exit
	       perror ("Error opening highscore file");
	       quit (0);
	  }
     }
     //Our hiscr file is now open in RW mode
     n = read (highFd, hi, sizeof (HighscoreEntry) * 4 * NUM_HIGHSCORE_ENTRY);
     if (n != sizeof (HighscoreEntry) * 4 * NUM_HIGHSCORE_ENTRY) {
	  fprintf (stderr, "Could not read highscore file! Try deleting %s\n",
		   hsFilePath);
     }

     free (hsFilePath);
     close (highFd);
}

//NOTE: The caller is responsible for freeing the pointer returned
char *
get_highscore_path (void)
{
     char *homeDir;
     char *hsFilePath;

     /////////////////////////////
     //get our home directory path
     /////////////////////////////
     homeDir = getenv ("HOME");
     if (homeDir == NULL) {
	  endwin ();
	  fprintf (stderr, "Could not locate your home directory! %s",
		   "Make sure the HOME environment variable is set.\n");
	  quit (3);
     }
     //////////////////////////
     //Build path to higscore file form components
     //////////////////////////
     hsFilePath = (char *) malloc (sizeof (char) *
				   (strlen (homeDir) +
				    strlen (highscoreFileName) +
				    strlen (highscoreDirName) +
				    strlen (incWithTimeSuffix) + 1));
     strcpy (hsFilePath, homeDir);
     if (hsFilePath[strlen (hsFilePath) - 1] == '/')
	  hsFilePath[strlen (hsFilePath) - 1] = '\0';

     strcat (hsFilePath, highscoreDirName);
     strcat (hsFilePath, highscoreFileName);
     if (g_opts.growsWithTime) {
	  strcat (hsFilePath, incWithTimeSuffix);
     }

     return hsFilePath;
}

void
blank_highscore ()
{
     int i;

     HighscoreEntry hiEntr = { "????", 0 };

     for (i = 0; i < 4 * NUM_HIGHSCORE_ENTRY; i++) {
	  hi[i] = hiEntr;
     }

     write_highscore (O_CREAT);
}
