/*
 * $Id: shot.c,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle player's shots.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "noiz2sa.h"
#include "screen.h"
#include "vector.h"
#include "degutil.h"
#include "shot.h"
#include "soundmanager.h"

Shot shot[SHOT_MAX];

void initShots() {
  int i;
  for ( i=0 ; i<SHOT_MAX ; i++ ) {
    shot[i].cnt = NOT_EXIST;
  }
}

static int shotIdx = SHOT_MAX;

void addShot(Vector *pos) {
  int i;
  for ( i=0 ; i<SHOT_MAX ; i++ ) {
    shotIdx--; if ( shotIdx < 0 ) shotIdx = SHOT_MAX-1;
    if ( shot[i].cnt == NOT_EXIST ) break;
  }
  if ( i >= SHOT_MAX ) return;
  shot[i].pos = *pos;
  shot[i].cnt = 0;
  playChunk(0);
}

void moveShots() {
  int i;
  Shot *st;
  for ( i=0 ; i<SHOT_MAX ; i++ ) {
    if ( shot[i].cnt == NOT_EXIST ) continue;
    st = &(shot[i]);
    st->pos.y -= SHOT_SPEED;
    st->cnt++;
    if ( st->pos.y < 0 ) {
      st->cnt = NOT_EXIST;
      continue;
    }
  }
}

void drawShots() {
  int x, y, d;
  int i;
  Shot *st;
  for ( i=0 ; i<SHOT_MAX ; i++ ) {
    if ( shot[i].cnt == NOT_EXIST ) continue;
    st = &(shot[i]);
    x = (st->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    y = (st->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    d = (st->cnt*16)&(DIV/8-1);
    drawBox(x+((sctbl[d]*SHOT_WIDTH)>>8), y, SHOT_WIDTH, SHOT_HEIGHT, 16*7-8, 16*7-1, buf);
    drawBox(x-((sctbl[d]*SHOT_WIDTH)>>8), y, SHOT_WIDTH, SHOT_HEIGHT, 16*7-8, 16*7-1, buf);
  }
}

