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


#include <stdlib.h>
#include <sys/time.h>

#include "misc.h"

void
randomize (void)
{
     struct timeval *tv = (struct timeval *) malloc (sizeof (struct timeval));
     struct timezone *tz =
	  (struct timezone *) malloc (sizeof (struct timezone));
     gettimeofday (tv, tz);
     srand (tv->tv_usec);
     free (tv);
     free (tz);
}

int
randint (int min, int max)
{
     return (min + (int) ((max + 1) * (rand () / (RAND_MAX + 1.0))));
}
