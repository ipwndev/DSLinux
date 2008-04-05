/*
 * $Id: bonus.c,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Bonus item.
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
#include "ship.h"
#include "bonus.h"
#include "soundmanager.h"
#include "attractmanager.h"

Bonus bonus[BONUS_MAX];
int bonusScore;

void resetBonusScore() {
  bonusScore = 10;
  showScore();
}

static void getBonus() {
  addScore(bonusScore);
  if ( bonusScore < 1000 ) bonusScore += 10;
}

static void missBonus() {
  bonusScore /= 20;
  bonusScore *= 10;
  if ( bonusScore < 10 ) bonusScore = 10;
  showScore();
}

void initBonuses() {
  int i;
  for ( i=0 ; i<BONUS_MAX ; i++ ) {
    bonus[i].cnt = NOT_EXIST;
  }
  resetBonusScore();
}

static int bonusIdx = BONUS_MAX;

void addBonus(Vector *pos, Vector *vel) {
  int i;
  for ( i=0 ; i<BONUS_MAX ; i++ ) {
    bonusIdx--; if ( bonusIdx < 0 ) bonusIdx = BONUS_MAX-1;
    if ( bonus[i].cnt == NOT_EXIST ) break;
  }
  if ( i >= BONUS_MAX ) return;
  bonus[i].pos = *pos;
  bonus[i].vel = *vel;
  bonus[i].cnt = 0;
  bonus[i].down = 1;
}

#define BONUS_SPEED 400
#define BONUS_INHALE_WIDTH 24000
#define BONUS_ACQUIRE_WIDTH 8000

void moveBonuses() {
  int i, d;
  Bonus *bn;
  for ( i=0 ; i<BONUS_MAX ; i++ ) {
    if ( bonus[i].cnt == NOT_EXIST ) continue;
    bn = &(bonus[i]);
    bn->pos.x += bn->vel.x;
    bn->pos.y += bn->vel.y;
    bn->vel.x -= bn->vel.x>>6;
    if ( bn->pos.x < SCAN_WIDTH_8/8 ) {
      bn->pos.x = SCAN_WIDTH_8/8;
      if ( bn->vel.x < 0 ) bn->vel.x = -bn->vel.x;
    } else if ( bn->pos.x > SCAN_WIDTH_8/8*7 ) {
      bn->pos.x = SCAN_WIDTH_8/8*7;
      if ( bn->vel.x > 0 ) bn->vel.x = -bn->vel.x;
    }
    if ( bn->down ) {
      bn->vel.y += (BONUS_SPEED-bn->vel.y)>>6;
      if ( bn->pos.y > SCAN_HEIGHT_8 ) {
	bn->down = 0;
	bn->pos.y = SCAN_HEIGHT_8;
	bn->vel.y = -bn->vel.y;
      }
    } else {
      bn->vel.y += (-BONUS_SPEED-bn->vel.y)>>6;
      if ( bn->pos.y < 0 ) {
	missBonus();
	bn->cnt = NOT_EXIST;
	continue;
      }
    }
    
    d = vctDist(&(ship.pos), &(bn->pos));
    if ( d < BONUS_ACQUIRE_WIDTH ) {
      getBonus();
      playChunk(5);
      bn->cnt = NOT_EXIST;
      continue;
    } else if ( d < BONUS_INHALE_WIDTH ) {
      bn->vel.x += (long)(ship.pos.x - bn->pos.x)*(BONUS_INHALE_WIDTH-d)>>20;
      bn->vel.y += (long)(ship.pos.y - bn->pos.y)*(BONUS_INHALE_WIDTH-d)>>20;
    }
    bn->cnt++;
  }
}

#define BONUS_COLOR_1 16*3-3
#define BONUS_COLOR_2 16*4-7
#define BONUS_DRAW_WIDTH 8

void drawBonuses() {
  int x, y, ox, oy, d;
  int i;
  Bonus *bn;
  for ( i=0 ; i<BONUS_MAX ; i++ ) {
    if ( bonus[i].cnt == NOT_EXIST ) continue;
    bn = &(bonus[i]);
    d = (bn->cnt*8)&(DIV-1);
    x = (bn->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    y = (bn->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    ox = sctbl[d]>>5;
    oy = sctbl[d+DIV/4]>>5;
    drawBox(x+ox, y+oy, BONUS_DRAW_WIDTH, BONUS_DRAW_WIDTH,
	    BONUS_COLOR_1, BONUS_COLOR_2, l1buf);
    drawBox(x-ox, y-oy, BONUS_DRAW_WIDTH, BONUS_DRAW_WIDTH,
	    BONUS_COLOR_1, BONUS_COLOR_2, l1buf);
    drawBox(x+oy, y-ox, BONUS_DRAW_WIDTH, BONUS_DRAW_WIDTH,
	    BONUS_COLOR_1, BONUS_COLOR_2, l1buf);
    drawBox(x-oy, y+ox, BONUS_DRAW_WIDTH, BONUS_DRAW_WIDTH,
	    BONUS_COLOR_1, BONUS_COLOR_2, l1buf);
  }
}
