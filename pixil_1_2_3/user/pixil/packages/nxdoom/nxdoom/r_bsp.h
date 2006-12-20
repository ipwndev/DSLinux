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
//	Refresh module, BSP traversal and handling.
//
//-----------------------------------------------------------------------------


#ifndef __R_BSP__
#define __R_BSP__

#ifdef __GNUG__
#pragma interface
#endif


extern seg_t*		curline;
extern side_t*		sidedef;
extern line_t*		linedef;
extern sector_t*	frontsector;
extern sector_t*	backsector;

extern int		rw_x;
extern int		rw_stopx;

extern boolean		segtextured;

// false if the back side is the same plane
extern boolean		markfloor;		
extern boolean		markceiling;

extern boolean		skymap;

extern drawseg_t	drawsegs[MAXDRAWSEGS];
extern drawseg_t*	ds_p;

extern lighttable_t**	hscalelight;
extern lighttable_t**	vscalelight;
extern lighttable_t**	dscalelight;


typedef void (*drawfunc_t) (int start, int stop);


// BSP?
void R_ClearClipSegs (void);
void R_ClearDrawSegs (void);


void R_RenderBSPNode (int bspnum);


#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2006/10/03 11:26:21  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.2  2003/09/08 22:34:30  jasonk
// Updated files because this fucker won't build for no fucking good reason.
//
// Revision 1.1.1.1  2003/09/04 21:08:13  jasonk
// Initial import
//
// Revision 1.1  2000/12/08 21:07:54  jeffw
// nxdoom initial entry -- No nxdoom/Makefile so it won't build automatically
//
//
//-----------------------------------------------------------------------------
