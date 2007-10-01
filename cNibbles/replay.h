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

#ifndef _REPLAY_
#define _REPLAY_

#include "misc.h"
#include "screen.h"
#include "options.h"
/*
typedef struct {
     long versionInfo;
     char speed;
     char dotx, doty;
} ReplayHeader;
*/
typedef struct ReplayNode_t {
     signed char dir[100];
     struct ReplayNode_t *next;
} ReplayNode;

void show_replay (WINDOW * w[]);
void show_best_replay (WINDOW * w[]);
int next_dir (ReplayNode * cur, short step);

void reset_history (void);
void delete_history (void);
void add_to_history (char dir);
void add_dot_to_history (char dotx, char doty);

void save_replay (char *fileName);
void load_replay (char *fileName);


#endif //_REPLAY_
