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
// $Log$
// Revision 1.1  2006/10/03 11:26:18  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.2  2003/09/08 22:34:28  jasonk
// Updated files because this fucker won't build for no fucking good reason.
//
// Revision 1.1.1.1  2003/09/04 21:08:13  jasonk
// Initial import
//
// Revision 1.1  2000/12/08 21:07:53  jeffw
// nxdoom initial entry -- No nxdoom/Makefile so it won't build automatically
//
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Random number LUT.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id$";


#ifdef __GNUG__
#pragma implementation "m_bbox.h"
#endif
#include "m_bbox.h"




void M_ClearBox (fixed_t *box)
{
    box[BOXTOP] = box[BOXRIGHT] = MININT;
    box[BOXBOTTOM] = box[BOXLEFT] = MAXINT;
}

void
M_AddToBox
( fixed_t*	box,
  fixed_t	x,
  fixed_t	y )
{
    if (x<box[BOXLEFT])
	box[BOXLEFT] = x;
    else if (x>box[BOXRIGHT])
	box[BOXRIGHT] = x;
    if (y<box[BOXBOTTOM])
	box[BOXBOTTOM] = y;
    else if (y>box[BOXTOP])
	box[BOXTOP] = y;
}





