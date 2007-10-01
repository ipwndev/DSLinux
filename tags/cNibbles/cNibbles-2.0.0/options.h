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

#ifndef _OPTIONS_
#define _OPTIONS_

typedef struct {
     char useColor;
     char growsWithTime;
     unsigned char speed;
} Options;

extern Options g_opts;

void print_help ();

void print_version (void);

void set_opt_defaults (void);

void parse_options (int argc, const char *argv[]);

char long_to_short_opt (const char arg[]);

#endif
