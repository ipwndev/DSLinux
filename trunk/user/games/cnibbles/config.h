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

#ifndef __CONFIG_H_
#define __CONFIG_H_

// Size of the window used by nibbles
#define		WINDOWX			64
#define		WINDOWY			25

// Number of microseconds between each tick
#define		TICKSPEED		100000

// The length of a new worm, don't set this too high!
#define		STARTLENGTH		5

// How many ticks the worm will continue to grow after start
#define		STARTELENGTH	0

// How much the worm grows when it eats
#define		WORMGROWTH		5

// Color settings - see the ncurses manpage for other colors
#define		WORMCOLOR		COLOR_YELLOW
#define		LINECOLOR		COLOR_WHITE
#define		STATUSCOLOR		COLOR_WHITE
#define		DOTCOLOR		COLOR_GREEN
#define		BACKGROUND		COLOR_BLACK

// How many moves we make room for in the history array - the higher, the
// better. When HISTORYSIZE ticks have passed, the last bytes (number equal
// to the length of the worm) are copied to the start of the array.
#define		HISTORYSIZE		8192




// Don't edit below this line!





#define		WORMCOLORPAIR	1
#define		LINECOLORPAIR	2
#define		STATUSCOLORPAIR	3
#define		DOTCOLORPAIR	4

#define		LEFT			1
#define		RIGHT			2
#define		UP				3
#define		DOWN			4

#define		GRIDX			((WINDOWX-2)>>1)
#define		GRIDY			(WINDOWY-4)

#define		Main			0
#define		Grid			1
#define		Status			2

#define		VERSION			1.1

#endif /* __CONFIG_H__ */
