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
// DESCRIPTION:
//    Nil.
//    
//-----------------------------------------------------------------------------


#ifndef __M_BBOX__
#define __M_BBOX__

#include "doomtype.h"

#include "m_fixed.h"


// Bounding box coordinate storage.
enum
{
    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT
};	// bbox coordinates

// Bounding box functions.
void M_ClearBox (fixed_t*	box);

void
M_AddToBox
( fixed_t*	box,
  fixed_t	x,
  fixed_t	y );


#endif
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
