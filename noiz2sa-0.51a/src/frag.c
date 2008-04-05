/*
 * $Id: frag.c,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Fragments.
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
#include "frag.h"

Frag frag[FRAG_MAX];

void initFrags() {
  int i;
  for ( i=0 ; i<FRAG_MAX ; i++ ) {
    frag[i].cnt = 0;
  }
}

static int fragIdx = FRAG_MAX;

static void addFrag(Vector *pos, Vector *vel, int spc, int size) {
  int i;
  for ( i=0 ; i<FRAG_MAX ; i++ ) {
    fragIdx--; if ( fragIdx < 0 ) fragIdx = FRAG_MAX-1;
    if ( frag[i].cnt <= 0 ) break;
  }
  if ( i >= FRAG_MAX ) return;
  frag[i].pos = *pos;
  frag[i].vel = *vel;
  switch ( spc ) {
  case 0:
    frag[i].width = 5+randN(10);
    frag[i].height = 5+randN(10);
    frag[i].cnt = 4+randN(8);
    break;
  case 1:
    frag[i].width = size*5+randN(size*3);
    frag[i].height = size*5+randN(size*3);
    frag[i].cnt = 12+randN(12);
    break;
  case 2:
    frag[i].width = 4;
    frag[i].height = 4;
    frag[i].cnt = 10+randN(4);
    break;
  }
  frag[i].spc = spc;
}

void addShotFrag(Vector *p) {
  Vector pos, vel;
  pos.x = (p->x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  pos.y = (p->y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  vel.x = randNS(SHOT_SPEED>>11)*LAYER_WIDTH/SCAN_WIDTH;
  vel.y = (-(SHOT_SPEED>>8) + randNS(SHOT_SPEED>>11))*LAYER_HEIGHT/SCAN_HEIGHT;
  addFrag(&pos, &vel, 0, 0);
}

void addEnemyFrag(Vector *p, int mx, int my, int type) {
  Vector pos, vel;
  int cmx, cmy;
  int i;
  pos.x = (p->x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  pos.y = (p->y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  cmx = (mx/SCAN_WIDTH*LAYER_WIDTH)>>8;
  cmy = (my/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  type = type*2+1;
  for ( i=0 ; i<type+randN(type*2) ; i++ ) {
    vel.x = randNS(16);
    vel.y = randNS(16);
    addFrag(&pos, &vel, 0, 0);
  }
  for ( i=0 ; i<type*2+randN(type) ; i++ ) {
    vel.x = cmx+randNS(3);
    vel.y = cmy+randNS(3);
    addFrag(&pos, &vel, 1, 2+type);
  }
}

void addShipFrag(Vector *p) {
  Vector pos, vel;
  int cmx, cmy;
  int i;
  pos.x = (p->x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  pos.y = (p->y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  for ( i=0 ; i<48 ; i++ ) {
    vel.x = randNS(24);
    vel.y = randNS(24);
    addFrag(&pos, &vel, 0, 0);
  }
  for ( i=0 ; i<32 ; i++ ) {
    vel.x = randNS(4);
    vel.y = randNS(4);
    addFrag(&pos, &vel, 1, 1+randN(6));
  }
}

void addClearFrag(Vector *p, Vector *v) {
  Vector pos, vel;
  pos.x = (p->x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  pos.y = (p->y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  vel.x = (v->x/SCAN_WIDTH*LAYER_WIDTH)>>8;
  vel.y = (v->y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
  addFrag(&pos, &vel, 2, 0);
}


void moveFrags() {
  int i;
  Frag *fr;
  for ( i=0 ; i<FRAG_MAX ; i++ ) {
    if ( frag[i].cnt <= 0 ) continue;
    fr = &(frag[i]);
    fr->pos.x += fr->vel.x;
    fr->pos.y += fr->vel.y;
    fr->cnt--;
  }
}

static int fragColor[3][2][2] = {
  {{16*8-7, 16*2-2}, {16*2-7, 16*8-2}},
  {{16*5-7, 16*2-2}, {16*2-7, 16*5-2}},
  {{16*1-10, 16*1-5}, {16*1-5, 16*1-10}},
};

void drawFrags() {
  int x, y, c;
  int i;
  Frag *fr;
  for ( i=0 ; i<FRAG_MAX ; i++ ) {
    if ( frag[i].cnt <= 0 ) continue;
    fr = &(frag[i]);
    c = fr->cnt&1;
    drawBox(fr->pos.x, fr->pos.y, fr->width, fr->height, 
	    fragColor[fr->spc][c][0],fragColor[fr->spc][c][1], l2buf);
  }
}
