/*
 * xrick/src/scr_imain.c
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

#include <stdio.h>  /* sprintf */

#include "system.h"
#include "game.h"
#include "screens.h"

#include "draw.h"
#include "pics.h"
#include "control.h"

/*
 * Main introduction
 *
 * return: SCREEN_RUNNING, SCREEN_DONE, SCREEN_EXIT
 */
U8
screen_introMain(void)
{
  static U8 seq = 0;
  static U8 seen = 0;
  static U8 first = TRUE;
  static U8 period = 0;
  static U32 tm = 0;
	U8 i, s[32];

  if (seq == 0) {
    draw_tilesBank = 0;
    if (first == TRUE)
      seq = 1;
    else
      seq = 4;
    period = game_period;
    game_period = 50;
    game_rects = &draw_SCREENRECT;
#ifdef ENABLE_SOUND
	game_setmusic("sounds/tune5.wav", -1);
#endif
  }

  switch (seq) {
  case 1:  /* dispay hall of fame */
    sysvid_clear();
    tm = sys_gettime();

#ifdef GFXPC
    /* Rick Dangerous title */
    draw_tllst = (U8 *)screen_imainrdt;
    draw_setfb(32, 16);
    draw_filter = 0xaaaa;
    draw_tilesList();

    /* Core Design copyright + press space to start */
    draw_tllst = (U8 *)screen_imaincdc;
    draw_setfb(64, 80);
    draw_filter = 0x5555;
    draw_tilesList();
#endif

#ifdef GFXST
    draw_pic(0, 0, 0x140, 0xc8, pic_splash);
#endif

    seq = 2;
    break;

  case 2:  /* wait for key pressed or timeout */
    if (control_status & CONTROL_FIRE)
      seq = 3;
    else if (sys_gettime() - tm > SCREEN_TIMEOUT) {
      seen++;
      seq = 4;
    }
    break;

  case 3:  /* wait for key released */
    if (!(control_status & CONTROL_FIRE)) {
      if (seen++ == 0)
	seq = 4;
      else
	seq = 7;
    }
    break;

  case 4:  /* display Rick Dangerous title and Core Design copyright */
    sysvid_clear();
    tm = sys_gettime();

    /* hall of fame title */
#ifdef GFXPC
    draw_tllst = (U8 *)screen_imainhoft;
    draw_setfb(32, 0);
    draw_filter = 0xaaaa;
    draw_tilesList();
#endif
#ifdef GFXST
    draw_pic(0, 0, 0x140, 0x20, pic_haf);
#endif

    /* hall of fame content */
    draw_setfb(56, 40);
#ifdef GFXPC
    draw_filter = 0x5555;
#endif
    for (i = 0; i < 8; i++) {
      sprintf((char *)s, "%06d@@@....@@@%s",
	      game_hscores[i].score, game_hscores[i].name);
      s[26] = '\377'; s[27] = '\377'; s[28] = '\376';
      draw_tllst = s;
      draw_tilesList();
    }

    seq = 5;
    break;

  case 5:  /* wait for key pressed or timeout */
    if (control_status & CONTROL_FIRE)
      seq = 6;
    else if (sys_gettime() - tm > SCREEN_TIMEOUT) {
      seen++;
      seq = 1;
    }
    break;

  case 6:  /* wait for key released */
    if (!(control_status & CONTROL_FIRE)) {
      if (seen++ == 0)
	seq = 1;
      else
	seq = 7;
    }
    break;
  }

  if (control_status & CONTROL_EXIT)  /* check for exit request */
    return SCREEN_EXIT;

  if (seq == 7) {  /* we're done */
    sysvid_clear();
    seq = 0;
    seen = 0;
    first = FALSE;
    game_period = period;
    return SCREEN_DONE;
  }
  else
    return SCREEN_RUNNING;
}

/* eof */


