/*
 * xrick/include/draw.h
 *
 * Copyright (C) 1998-2002 BigOrno (bigorno@bigorno.net). All rights reserved.
 *
 * The use and distribution terms for this software are contained in the file
 * named README, which can be found in the root of this distribution. By
 * using this software in any fashion, you are agreeing to be bound by the
 * terms of this license.
 *
 * You must not remove this notice, or any other, from this software.
 */

#ifndef _DRAW_H
#define _DRAW_H

#include "system.h"

#include "rects.h"
#include "img.h"

/* map coordinates of the screen */
#define DRAW_XYMAP_SCRLEFT (-0x0020)
#define DRAW_XYMAP_SCRTOP (0x0040)
/* map coordinates of the top of the hidden bottom of the map */
#define DRAW_XYMAP_HBTOP (0x0100)

extern U8 *draw_tllst;
#ifdef GFXPC
extern U16 draw_filter;
#endif
extern U8 draw_tilesBank;
extern rect_t draw_STATUSRECT;
extern rect_t draw_SCREENRECT;

extern void draw_setfb(U16, U16);
extern U8 draw_clipms(S16 *, S16 *, U16 *, U16 *);
extern void draw_tilesList(void);
extern void draw_tilesListImm(U8 *);
extern U8 draw_tilesSubList(void);
extern void draw_tile(register U8);
extern void draw_sprite(U8, U16, U16);
extern void draw_sprite2(U8, U16, U16, U8);
extern void draw_spriteBackground(U16, U16);
extern void draw_map(void);
extern void draw_drawStatus(void);
extern void draw_clearStatus(void);
extern void draw_pic(U16, U16, U16, U16, U32 *);
extern void draw_infos(void);
extern void draw_img(img_t *);

#endif

/* eof */
