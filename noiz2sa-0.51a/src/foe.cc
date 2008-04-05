/*
 * $Id: foe.cc,v 1.5 2003/01/03 05:24:21 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle enemy.
 *
 * @version $Revision: 1.5 $
 */
#include "barragemanager.h"
#include "foe.h"

extern "C" {
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "noiz2sa.h"
#include "screen.h"
#include "vector.h"
#include "degutil.h"
#include "ship.h"
#include "shot.h"
#include "frag.h"
#include "bonus.h"
#include "soundmanager.h"
#include "attractmanager.h"
#include "brgmng_mtd.h"
}

#define FOE_MAX 1024
#define FOE_TYPE_MAX 4

static Foe foe[FOE_MAX];
int foeCnt, enNum[FOE_TYPE_MAX];

static void removeFoeForcedNoDeleteCmd(Foe *fe) {
  if ( fe->spc == FOE ) {
    foeCnt--; enNum[fe->type]--;
  }
  fe->spc = NOT_EXIST;
}

static void removeFoeForced(Foe *fe) {
  removeFoeForcedNoDeleteCmd(fe);
  if ( fe->cmd ) {
    delete fe->cmd;
    fe->cmd = NULL;
  }
}

void removeFoe(Foe *fe) {
  if ( fe->type == BOSS_TYPE ) return;
  removeFoeForcedNoDeleteCmd(fe);
}

void initFoes() {
  int i, j;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    removeFoeForced(&(foe[i]));
  }
  foeCnt = 0;
  for ( i=0 ; i<FOE_TYPE_MAX ; i++ ) {
    enNum[i] = 0;
  }
}

void closeFoes() {
  int i, j;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].cmd ) delete foe[i].cmd;
  }
}

static int foeIdx = FOE_MAX;

static Foe* getNextFoe() {
  int i;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    foeIdx--; if ( foeIdx < 0 ) foeIdx = FOE_MAX-1;
    if ( foe[i].spc == NOT_EXIST ) break;
  }
  if ( i >= FOE_MAX ) return NULL;
  return &(foe[i]);
}

Foe* addFoe(int x, int y, double rank, int d, int spd, int type, int shield, 
	    BulletMLParser *parser) {
  int i;
  Foe *fe = getNextFoe();
  if ( !fe ) return NULL;

  fe->parser = parser;

  fe->cmd = new FoeCommand(parser, fe);

  fe->pos.x = x; fe->pos.y = y;
  fe->spos = fe->ppos = fe->pos;
  fe->vel.x = fe->vel.y = 0;
  fe->rank = rank;
  fe->d = d; fe->spd = spd;
  fe->spc = FOE;
  fe->type = type;
  fe->shield = shield;
  fe->cnt = 0;
  fe->color = 0;
  fe->hit = 0;

  foeCnt++; enNum[type]++;

  return fe;
}

Foe* addFoeBossActiveBullet(int x, int y, double rank, 
			    int d, int spd, BulletMLParser *parser) {
  Foe *fe = addFoe(x, y, rank, d, spd, BOSS_TYPE, 0, parser);
  if ( !fe ) return NULL;
  foeCnt--; enNum[BOSS_TYPE]--;
  fe->spc = BOSS_ACTIVE_BULLET;
  return fe;
}

void addFoeActiveBullet(Vector *pos, double rank, 
			int d, int spd, int color, BulletMLState *state) {
  Foe *fe = getNextFoe();
  if ( !fe ) return;
  fe->cmd = new FoeCommand(state, fe);
  fe->spos = fe->ppos = fe->pos = *pos;
  fe->vel.x = fe->vel.y = 0;
  fe->rank = rank;
  fe->d = d; fe->spd = spd;
  fe->spc = ACTIVE_BULLET;
  fe->type = 0;
  fe->cnt = 0;
  fe->color = color;
}

void addFoeNormalBullet(Vector *pos, double rank, int d, int spd, int color) {
  Foe *fe = getNextFoe();
  if ( !fe ) return;
  fe->cmd = NULL;
  fe->spos = fe->ppos = fe->pos = *pos;
  fe->vel.x = fe->vel.y = 0;
  fe->rank = rank;
  fe->d = d; fe->spd = spd;
  fe->spc = BULLET;
  fe->type = 0;
  fe->cnt = 0;
  fe->color = color;
}


#define BULLET_WIPE_WIDTH 7200

static void wipeBullets(Vector *pos, int width) {
  int i;
  Foe *fe;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc != ACTIVE_BULLET && foe[i].spc != BULLET ) continue;
    fe = &(foe[i]);
    if ( vctDist(pos, &(fe->pos)) < width ) {
      addBonus(&(fe->pos), &(fe->mv));
      removeFoeForced(fe);
    }
  }
}

static int foeSize[] = {30, 40, 56, 96};
static int foeScanSize[] = {
  foeSize[0]*256*SCAN_WIDTH/LAYER_WIDTH/4*3, foeSize[1]*256*SCAN_WIDTH/LAYER_WIDTH/4*3, 
  foeSize[2]*256*SCAN_WIDTH/LAYER_WIDTH/4*3, foeSize[3]*256*SCAN_WIDTH/LAYER_WIDTH/4*3, 
};
#define SHOT_SCAN_HEIGHT (SHOT_HEIGHT*256*SCAN_HEIGHT/LAYER_HEIGHT/2)
static int enemyScore[] = {500, 1000, 5000, 50000};

#define SHIP_HIT_WIDTH 512*512

int processSpeedDownBulletsNum = DEFAULT_SPEED_DOWN_BULLETS_NUM;
int nowait = 0;

void moveFoes() {
  int i, j;
  Foe *fe;
  int foeNum = 0;
  int mx, my;
  int wl;
  Vector bmv, sofs;
  float ht, hd, inab, inaa;

  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc == NOT_EXIST ) continue;
    fe = &(foe[i]);
    if ( fe->cmd ) {
      if ( fe->type == BOSS_TYPE ) {
	if ( fe->cmd->isEnd() ) {
	  delete fe->cmd;
	  fe->cmd = new FoeCommand(fe->parser, fe);
	  //fe->cmd->reset();
	}
      }
      fe->cmd->run();
      if ( fe->spc == NOT_EXIST ) {
	if ( fe->cmd ) {
	  delete fe->cmd;
	  fe->cmd = NULL;
	}
	continue;
      }
    }
    mx =  ((sctbl[fe->d]*fe->spd)>>8) + fe->vel.x;
    my = -((sctbl[fe->d+256]*fe->spd)>>8) + fe->vel.y;
    fe->pos.x += mx;
    fe->pos.y += my;
    fe->mv.x = mx;
    fe->mv.y = my;
    wl = 2;
    if ( fe->cnt < 4 ) wl = 0;
    else if ( fe->cnt < 8 ) wl = 1;
    fe->ppos.x = fe->pos.x - (mx<<wl);
    fe->ppos.y = fe->pos.y - (my<<wl);
    fe->cnt++;

    if ( fe->spc == FOE ) {
      fe->hit = 0;
      // Check if the shot hits the foe.
      for ( j=0 ; j<SHOT_MAX ; j++ ) {
	if ( shot[j].cnt != NOT_EXIST ) {
	  if ( absN(fe->pos.x-shot[j].pos.x) < foeScanSize[fe->type] &&
	       absN(fe->pos.y-shot[j].pos.y) < foeScanSize[fe->type]+SHOT_SCAN_HEIGHT ) {
	    shot[j].cnt = NOT_EXIST;
	    fe->shield--; fe->hit = 1;
	    addShotFrag(&shot[j].pos);
	    if ( fe->shield <= 0 ) {
	      addScore(enemyScore[fe->type]);
	      wipeBullets(&(fe->pos), BULLET_WIPE_WIDTH*(fe->type+1));
	      addEnemyFrag(&(fe->pos), mx, my, fe->type);
	      if ( fe->type == BOSS_TYPE ) {
		bossDestroied();
		playChunk(3);
	      } else {
		playChunk(2);
	      }
	      removeFoeForced(fe);
	      continue;
	    }
	    playChunk(1);
	  }
	}
      }
    } else {
      // Check if the bullet hits the ship.
      bmv = fe->pos;
      vctSub(&bmv, &(fe->ppos));
      inaa = vctInnerProduct(&bmv, &bmv);
      if ( inaa > 1.0f ) {
	sofs = ship.pos;
	vctSub(&sofs, &(fe->ppos));
	inab = vctInnerProduct(&bmv, &sofs);
	ht =  inab / inaa;
	if ( ht > 0.0f && ht < 1.0f ) {
	  hd = vctInnerProduct(&sofs, &sofs) - inab*inab/inaa/inaa;
	  if ( hd >= 0 && hd < SHIP_HIT_WIDTH ) {
	    destroyShip();
	  }
	}
      }
    }
    if ( fe->ppos.x < 0 || fe->ppos.x >= SCAN_WIDTH_8 ||
	 fe->ppos.y < 0 || fe->ppos.y >= SCAN_HEIGHT_8 ) {
      removeFoeForced(fe);
      continue;
    }
    foeNum++;
  }

  // A game speed becomes slow as many bullets appears.
  interval = INTERVAL_BASE;
  if ( !insane && !nowait && foeNum > processSpeedDownBulletsNum ) {
    interval += (foeNum-processSpeedDownBulletsNum) * INTERVAL_BASE / 
      processSpeedDownBulletsNum;
    if ( interval > INTERVAL_BASE*2 ) interval = INTERVAL_BASE*2;
  }
}

void clearFoes() {
  int i;
  Foe *fe;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc == NOT_EXIST ) continue;
    fe = &(foe[i]);
    addClearFrag(&(fe->pos), &(fe->mv));
    removeFoeForced(fe);
  }
}

void clearFoesZako() {
  int i;
  Foe *fe;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc == NOT_EXIST || 
	 foe[i].type == BOSS_TYPE || foe[i].spc == BOSS_ACTIVE_BULLET ) continue;
    fe = &(foe[i]);
    addClearFrag(&(fe->pos), &(fe->mv));
    removeFoeForced(fe);
  }
}


void drawBulletsWake() {
  int i;
  Foe *fe;
  int x, y, sx, sy;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc == NOT_EXIST || foe[i].spc == FOE || foe[i].cnt >= 64 ) continue;
    fe = &(foe[i]);
    x = (fe->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    y = (fe->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    sx = (fe->spos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    sy = (fe->spos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    drawLine(x, y, sx, sy, 13-fe->cnt/5, 1, l1buf);
  }
}

static int foeColor[][2] = {
  {16*4-1, 16*10-7}, {16*2-1, 16*8-7}, {16*6-1, 16*12-7}, {16*1-11, 16*1-4}
};

#define FOE_HIT_COLOR 16*3-1

void drawFoes() {
  int i, j;
  Foe *fe;
  int x, y, px, py;
  int sz, cl1, cl2;
  int d, md, di;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc != FOE ) continue;
    fe = &(foe[i]);
    x = (fe->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    y = (fe->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    if ( fe->cnt < 16 ) {
      sz = (foeSize[fe->type]*fe->cnt)>>4;
    } else {
      sz = foeSize[fe->type];
    }
    cl1 = foeColor[fe->type][0]; cl2 = foeColor[fe->type][1];
    if ( fe->hit ) {
      cl1 = FOE_HIT_COLOR;
    }
    drawBox(x, y, sz, sz, cl1, cl2, l1buf);
    d = (fe->cnt*8)<<4; md = (DIV<<4)/fe->shield;
    sz /= 3;
    for ( j=0 ; j<fe->shield ; j++, d+=md ) {
      di = (d>>4)&(DIV-1);
      drawBox(x+((sctbl[di]*sz)>>7), y+((sctbl[di+DIV/4]*sz)>>7), sz, sz, cl1, cl2, l1buf);
    }
  }
}

#define BULLET_COLOR_NUM 3

static int bulletColor[BULLET_COLOR_NUM][2] = {
  {16*14-1, 16*2-1}, {16*16-1, 16*4-1}, {16*12-1, 16*6-1}, 
};

#define BULLET_WIDTH 6
 
void drawBullets() {
  int i;
  Foe *fe;
  int x, y, px, py;
  int bc;
  for ( i=0 ; i<FOE_MAX ; i++ ) {
    if ( foe[i].spc == NOT_EXIST || foe[i].spc == FOE ) continue;
    fe = &(foe[i]);
    x = (fe->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    y = (fe->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    px = (fe->ppos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
    py = (fe->ppos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
    bc = fe->color%BULLET_COLOR_NUM;
    drawThickLine(x, y, px, py, bulletColor[bc][0], bulletColor[bc][1], BULLET_WIDTH);
  }
}
