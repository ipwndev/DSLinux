/*
 * $Id: ship.c,v 1.5 2003/02/09 07:34:16 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle player.
 *
 * @version $Revision: 1.5 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "noiz2sa.h"
#include "screen.h"
#include "vector.h"
#include "ship.h"
#include "shot.h"
#include "bonus.h"
#include "foe_mtd.h"
#include "degutil.h"
#include "brgmng_mtd.h"
#include "frag.h"
#include "soundmanager.h"
#include "attractmanager.h"

Ship ship;

#define SHIP_SPEED 1280
#define SHIP_SLOW_SPEED 640
#define SHIP_SLOW_DOWN 64

#define SHIP_INVINCIBLE_CNT_BASE 240

void initShip() {
  ship.pos.x = (SCAN_WIDTH/2)<<8; ship.pos.y = (SCAN_HEIGHT/5*4)<<8;
  ship.cnt = 0; ship.shotCnt = -1;
  ship.speed = SHIP_SPEED;
  ship.invCnt = SHIP_INVINCIBLE_CNT_BASE*(100-scene)/100;
  if ( ship.invCnt < 0 ) ship.invCnt = 0;
}

#define SHOT_INTERVAL 3

#define SHIP_SCAN_WIDTH 1024
#define SHIP_SCREEN_EDGE_WIDTH 3

static int shipMv[8][2] = {
  {0, -256}, {181, -181}, {256, 0}, {181, 181}, {0, 256}, {-181, 181}, {-256, 0}, {-181, -181},  
};

void moveShip() {
  int pad = getPadState();
  int btn = getButtonState();
  int sd = -1;
  if ( pad & PAD_RIGHT ) {
    sd = 2;
  }
  if ( pad & PAD_LEFT ) {
    sd = 6;
  }
  if ( pad & PAD_DOWN ) {
    switch ( sd ) {
    case 2:
      sd = 3;
      break;
    case 6:
      sd = 5;
      break;
    default:
      sd = 4;
      break;
    }
  }
  if ( pad & PAD_UP ) {
    switch ( sd ) {
    case 2:
      sd = 1;
      break;
    case 6:
      sd = 7;
      break;
    default:
      sd = 0;
      break;
    }
  }
  if ( btn & PAD_BUTTON1 ) {
    if ( ship.shotCnt < 0 && status == IN_GAME ) {
      addShot(&(ship.pos));
      ship.shotCnt = SHOT_INTERVAL;
    }
  }
  ship.shotCnt--;
  if ( btn & PAD_BUTTON2 ) {
    if ( ship.speed > SHIP_SLOW_SPEED ) {
      ship.speed -= SHIP_SLOW_DOWN;
    }
  } else {
    if ( ship.speed < SHIP_SPEED ) {
      ship.speed += SHIP_SLOW_DOWN;
    }
  }

  if ( sd >= 0 ) {
    ship.pos.x += (ship.speed*shipMv[sd][0])>>8;
    ship.pos.y += (ship.speed*shipMv[sd][1])>>8;
    if ( ship.pos.x < SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH ) {
      ship.pos.x = SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH;
    } else if ( ship.pos.x > SCAN_WIDTH_8-SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH ) {
      ship.pos.x = SCAN_WIDTH_8-SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH;
    }
    if ( ship.pos.y < SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH ) {
      ship.pos.y = SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH;
    } else if ( ship.pos.y > SCAN_HEIGHT_8-SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH ) {
      ship.pos.y = SCAN_HEIGHT_8-SHIP_SCAN_WIDTH*SHIP_SCREEN_EDGE_WIDTH;
    }
  }

  ship.cnt++;
  if ( ship.invCnt > 0 ) ship.invCnt--;
}

#define SHIP_DRAW_WIDTH 6
#define SHIP_DRUM_WIDTH 15
#define SHIP_DRUM_SIZE 4

void drawShip() {
  int x, y, d;
  int i;
  int ic;
  x = (ship.pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  y = (ship.pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  d = (ship.cnt*8)&(DIV/8-1); d -= DIV/4;
  ic = ship.invCnt&31;
  if ( ic > 0 && ic < 16 ) {
    drawBox(x, y, SHIP_DRAW_WIDTH, SHIP_DRAW_WIDTH, 16*2-1, 16*4-5, buf);
    return;
  }
  for ( i=0 ; i<4 ; i++ ) {
    d &= (DIV-1);
    drawBox(x+((sctbl[d]*SHIP_DRUM_WIDTH)>>8), y-((sctbl[d+DIV/4]*SHIP_DRUM_WIDTH)>>10), 
	    SHIP_DRUM_SIZE, SHIP_DRUM_WIDTH*2, 16*3-10, 16*3-12, buf);
    d += DIV/8;
  }
  drawBox(x, y, SHIP_DRAW_WIDTH, SHIP_DRAW_WIDTH, 16*2-1, 16*4-5, buf);
  for ( i=0 ; i<4 ; i++ ) {
    d &= (DIV-1);
    drawBox(x+((sctbl[d]*SHIP_DRUM_WIDTH)>>8), y-((sctbl[d+DIV/4]*SHIP_DRUM_WIDTH)>>10), 
	    SHIP_DRUM_SIZE, SHIP_DRUM_WIDTH*2, 16*3-7, 16*4-11, buf);
    d += DIV/8;
  }
}

void destroyShip() {
  if ( status != IN_GAME || ship.invCnt > 0 ) return;
  addShipFrag(&(ship.pos));
  playChunk(4);
  resetBonusScore();
  if ( decrementShip() ) {
    initGameover();
  } else {
    ship.invCnt = SHIP_INVINCIBLE_CNT_BASE*(100-scene)/100;
    if ( ship.invCnt < 0 ) ship.invCnt = 0;
    clearFoesZako();
  }
}

int getPlayerDeg(int x, int y) {
  return getDeg(ship.pos.x - x, ship.pos.y - y);
}
