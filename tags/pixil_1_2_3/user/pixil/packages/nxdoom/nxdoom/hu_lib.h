// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------

#ifndef __HULIB__
#define __HULIB__

// We are referring to patches.
#include "r_defs.h"


// background and foreground screen numbers
// different from other modules.
#define BG			1
#define FG			0

// font stuff
#define HU_CHARERASE	KEY_BACKSPACE

#define HU_MAXLINES		4
#define HU_MAXLINELENGTH	80

//
// Typedefs of widgets
//

// Text Line widget
//  (parent of Scrolling Text and Input Text widgets)
typedef struct
{
    // left-justified position of scrolling text window
    int		x;
    int		y;
    
    patch_t**	f;			// font
    int		sc;			// start character
    char	l[HU_MAXLINELENGTH+1];	// line of text
    int		len;		      	// current line length

    // whether this line needs to be udpated
    int		needsupdate;	      

} hu_textline_t;



// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
    hu_textline_t	l[HU_MAXLINES];	// text lines to draw
    int			h;		// height in lines
    int			cl;		// current line number

    // pointer to boolean stating whether to update window
    boolean*		on;
    boolean		laston;		// last value of *->on.

} hu_stext_t;



// Input Text Line widget
//  (child of Text Line widget)
typedef struct
{
    hu_textline_t	l;		// text line to input on

     // left margin past which I am not to delete characters
    int			lm;

    // pointer to boolean stating whether to update window
    boolean*		on; 
    boolean		laston; // last value of *->on;

} hu_itext_t;


//
// Widget creation, access, and update routines
//

// initializes heads-up widget library
void HUlib_init(void);

//
// textline code
//

// clear a line of text
void	HUlib_clearTextLine(hu_textline_t *t);

void	HUlib_initTextLine(hu_textline_t *t, int x, int y, patch_t **f, int sc);

// returns success
boolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);

// returns success
boolean HUlib_delCharFromTextLine(hu_textline_t *t);

// draws tline
void	HUlib_drawTextLine(hu_textline_t *l, boolean drawcursor);

// erases text line
void	HUlib_eraseTextLine(hu_textline_t *l); 


//
// Scrolling Text window widget routines
//

// ?
void
HUlib_initSText
( hu_stext_t*	s,
  int		x,
  int		y,
  int		h,
  patch_t**	font,
  int		startchar,
  boolean*	on );

// add a new line
void HUlib_addLineToSText(hu_stext_t* s);  

// ?
void
HUlib_addMessageToSText
( hu_stext_t*	s,
  char*		prefix,
  char*		msg );

// draws stext
void HUlib_drawSText(hu_stext_t* s);

// erases all stext lines
void HUlib_eraseSText(hu_stext_t* s); 

// Input Text Line widget routines
void
HUlib_initIText
( hu_itext_t*	it,
  int		x,
  int		y,
  patch_t**	font,
  int		startchar,
  boolean*	on );

// enforces left margin
void HUlib_delCharFromIText(hu_itext_t* it);

// enforces left margin
void HUlib_eraseLineFromIText(hu_itext_t* it);

// resets line and left margin
void HUlib_resetIText(hu_itext_t* it);

// left of left-margin
void
HUlib_addPrefixToIText
( hu_itext_t*	it,
  char*		str );

// whether eaten
boolean
HUlib_keyInIText
( hu_itext_t*	it,
  unsigned char ch );

void HUlib_drawIText(hu_itext_t* it);

// erases all itext lines
void HUlib_eraseIText(hu_itext_t* it); 

#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2006/10/03 11:26:17  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.2  2003/09/08 22:34:28  jasonk
// Updated files because this fucker won't build for no fucking good reason.
//
// Revision 1.1.1.1  2003/09/04 21:08:12  jasonk
// Initial import
//
// Revision 1.1  2000/12/08 21:07:53  jeffw
// nxdoom initial entry -- No nxdoom/Makefile so it won't build automatically
//
//
//-----------------------------------------------------------------------------
