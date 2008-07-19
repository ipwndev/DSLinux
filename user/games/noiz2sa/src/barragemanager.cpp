/*
 * $Id: barragemanager.cc,v 1.4 2003/02/09 07:34:15 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle stage data.
 *
 * @version $Revision: 1.4 $
 */
extern "C" {
#include "SDL.h"
#include <sys/types.h>
#include <dirent.h>
#include "noiz2sa.h"
#include "degutil.h"
#include "vector.h"
#include "screen.h"
#include "rand.h"
#include "brgmng_mtd.h"
#include "soundmanager.h"
#include "attractmanager.h"
}

#include "barragemanager.h"
#include "foe.h"

#define BARRAGE_PATTERN_MAX 32
#define SHARE_LOC "/usr/share/games/noiz2sa/"

static Barrage barragePattern[BARRAGE_TYPE_NUM][BARRAGE_PATTERN_MAX];
static Barrage *barrageQueue[BARRAGE_TYPE_NUM][BARRAGE_PATTERN_MAX];
static int barragePatternNum[BARRAGE_TYPE_NUM];

static Barrage *barrage[BARRAGE_MAX];

static const char *BARRAGE_DIR_NAME[] = {
  "zako", "middle", "boss"
};

static int readBulletMLFiles(const char *dirPath, Barrage brg[]) {
  DIR *dp;
  struct dirent *dir;
  int i = 0;
  char fileName[256];
  char fullDirPath [128];

  strcpy(fullDirPath, SHARE_LOC);
  strcat(fullDirPath, dirPath);

  if ( (dp = opendir(fullDirPath)) == NULL ) {
    fprintf(stderr, "Can't open directory: %s\n", dirPath);
    exit(1);
  }
  while ((dir = readdir(dp)) != NULL) {
    if ( strcmp(strrchr(dir->d_name, '.'), ".xml") != 0 ) continue; // Read .xml files.
    strcpy(fileName, fullDirPath);
    strcat(fileName, "/");
    strcat(fileName, dir->d_name);
    brg[i].bulletml = new BulletMLParserTinyXML(fileName);
    brg[i].bulletml->build(); i++;
    printf("%s\n", fileName);
  }
  closedir(dp);
  return i;
}

static unsigned int rnd;

void initBarragemanager() {
  for ( int i=0 ; i<BARRAGE_TYPE_NUM ; i++ ) {
    barragePatternNum[i] = readBulletMLFiles(BARRAGE_DIR_NAME[i], barragePattern[i]);
    printf("--------\n");
    for ( int j=0 ; j<barragePatternNum[i] ; j++ ) {
      barragePattern[i][j].type = i;
    }
  }
}

void closeBarragemanager() {
  for ( int i=0 ; i<BARRAGE_TYPE_NUM ; i++ ) {
    for ( int j=0 ; j<barragePatternNum[i] ; j++ ) {
      delete barragePattern[i][j].bulletml;
    }
  }
}

int scene;
int endless, insane;
static int sceneCnt;
static float level, levelInc;

void initBarrages(int seed, float startLevel, float li) {
  int n1, n2, rn;

  for ( int i=0 ; i<BARRAGE_TYPE_NUM ; i++ ) {
    for ( int j=0 ; j<barragePatternNum[i] ; j++ ) {
      barrageQueue[i][j] = &(barragePattern[i][j]);
    }
  }

  processSpeedDownBulletsNum = DEFAULT_SPEED_DOWN_BULLETS_NUM;
  if ( seed >= 0 ) {
    rnd = seed;
    endless = 0;
    insane = 0;
  } else {
    rnd = (unsigned int)SDL_GetTicks();
    endless = 1;
    if ( seed == -2 ) insane = 1;
    else insane = 0;
    if ( seed == -3 ) processSpeedDownBulletsNum = EASY_SPEED_DOWN_BULLETS_NUM;
    else if ( seed == -4 ) processSpeedDownBulletsNum = HARD_SPEED_DOWN_BULLETS_NUM;
  }
  // Shuffle.
  for ( int i=0 ; i<BARRAGE_TYPE_NUM ; i++ ) {
    int bn = barragePatternNum[i];
    rn = 60+nextRandInt(&rnd)%4;
    for ( int j=0 ; j<rn ; j++ ) {
      n1 = nextRandInt(&rnd)%bn; n2 = nextRandInt(&rnd)%bn;
      Barrage* tb = barrageQueue[i][n1];
      barrageQueue[i][n1] = barrageQueue[i][n2];
      barrageQueue[i][n2] = tb;
    }
    for ( int j=0 ; j<bn ; j++ ) {
      barrageQueue[i][j]->maxRank = (float)(nextRandInt(&rnd)%70)/100 + 0.3f;
      barrageQueue[i][j]->frq = 1;
    }
  }

  scene = -1;
  sceneCnt = 0;
  level = startLevel;
  levelInc = li;
}

/**
 * Roll the barrage queue after the new barrage pattern is set.
 */
static void rollBarragePattern(Barrage *br[], int brNum) {
  Barrage *tbr;
  int n = (int)((float)brNum/((float)(nextRandInt(&rnd)%32)/32+1)+0.5f);
  if ( n == 0 ) return;
  if ( n > brNum ) n = brNum;
  tbr = br[0];
  for ( int i=0 ; i<n-1 ; i++ ) {
    br[i] = br[i+1];
  }
  br[n-1] = tbr;
  br[0]->maxRank *= 2;
  while ( br[0]->maxRank > 1 ) br[0]->maxRank -= 0.7f;
}

static int barrageNum;
static int bossMode;

static int pax;
static int pay;
static int quickAppType;

/**
 * Make the barrage pattern of this scene.
 */
void setBarrages(float level, int bm, int midMode) {
  int bpn = 0, bn, i;
  int barrageMax, addFrqLoop = 0;

  barrageNum = 0;
  barrageMax = nextRandInt(&rnd)%3+4;
  bossMode = bm;
  if ( !midMode ) {
    bpn = 0;
  } else {
    bpn = 1;
  }
  quickAppType = bpn;
  for ( bn = 0 ;  ; bn++ ) {
    if ( bn == 0 && level < 0 ) break;
    if ( bossMode ) {
      if ( bn == 0 ) bpn = 0;
      else bpn = 2;
      if ( bn >= BARRAGE_MAX ) break;
    } else {
      if ( bn >= barrageMax ) {
	bn = 0;
	addFrqLoop = 1;
      }
    }
    if ( addFrqLoop ) {
      barrage[bn]->frq++;
      level -= 1+barrage[bn]->rank;
      if ( level < 0 ) break;
    } else {
      barrageNum++;
      rollBarragePattern(barrageQueue[bpn], barragePatternNum[bpn]);
      barrage[bn] = barrageQueue[bpn][0];
      barrage[bn]->frq = 1;
      if ( level < barrageQueue[bpn][0]->maxRank ) {
	if ( level < 0 ) level = 0;
	barrage[bn]->rank = level;
	if ( !bossMode || bn > 0 ) break;
      }
      barrage[bn]->rank = barrageQueue[bpn][0]->maxRank;
      if ( !bossMode ) {
	level -= 1+barrageQueue[bpn][0]->maxRank;
      } else {
	if ( bn > 0 ) level -= 4+(barrageQueue[bpn][0]->maxRank*6);
      }

      bpn++;
      if ( bpn >= BARRAGE_TYPE_NUM ) {
	if ( !midMode ) {
	  bpn = 0;
	} else {
	  bpn = 1;
	}
      }
    }
  }

  pax = (nextRandInt(&rnd)%(SCAN_WIDTH_8*2/3) + (SCAN_WIDTH_8/6));
  pay = (nextRandInt(&rnd)%(SCAN_HEIGHT_8/6) + (SCAN_HEIGHT_8/10));

  scene++;
}

#define SCENE_TERM 1000
#define SCENE_END_TERM 100
#define ZAKO_APP_TERM 1500
static int zakoAppCnt;

static int appFreq[] = {90, 360, 800};
static int shield[] = {3, 6, 9};

static Foe *bossBullet;

/**
 * Add enemies.
 */
void addBullets() {
  int x, y, i;
  int type, frq;

  // Scene time control.
  sceneCnt--;
  if ( sceneCnt < 0 ) {
    if ( !insane ) clearFoes();
    if ( scene >= 0 && !endless ) setClearScore();
    if ( scene%10 == 8 ) {
      sceneCnt = 999999;
      zakoAppCnt = ZAKO_APP_TERM;
      setBarrages(level, true, false);
      addBossBullet();
    } else {
      sceneCnt = SCENE_TERM;
      if ( scene%10 == 3 ) {
	setBarrages(level, false, true);
      } else {
	setBarrages(level, false, false);
      }
    }
    level += levelInc;
    if ( status == IN_GAME ) {
      drawRPanel();
    } else {
      sceneCnt = 999999;
    }
  }

  if ( sceneCnt < SCENE_END_TERM ) return;

  for ( i=0 ; i<barrageNum ; i++ ) {
    if ( bossMode ) {
      if ( i > 0 ) break;
      if ( zakoAppCnt <= 0 ) break;
      zakoAppCnt--;
    }
    type = barrage[i]->type;
    // An additional enemy appears when there is no enemy of the same type.
    if ( type == quickAppType && enNum[type] == 0 ) {
      x = pax; y = pay;
      addFoe(x, y, barrage[i]->rank, 512, 0, type, shield[type], barrage[i]->bulletml);
    }

    frq = appFreq[type]/barrage[i]->frq;
    if ( frq < 2 ) frq = 2;
    if ( (nextRandInt(&rnd)%frq) == 0 ) {
      x = nextRandInt(&rnd)%(SCAN_WIDTH_8*2/3) + (SCAN_WIDTH_8/6);
      y = nextRandInt(&rnd)%(SCAN_HEIGHT_8/6) + (SCAN_HEIGHT_8/10);
      if ( type == quickAppType ) {
	pax = x; pay = y;
      }
      addFoe(x, y, barrage[i]->rank, 512, 0, type, shield[type], barrage[i]->bulletml);
    }
  }
}

void bossDestroied() {
  if ( !endless ) {
    setClearScore();
    addLeftBonus();
    initStageClear();
  }
  clearFoes();
  sceneCnt = 180;
  zakoAppCnt = 0;
}

#define BOSS_SHIELD 128

void addBossBullet() {
  Foe *bl;
  bossBullet = NULL;

  for ( int i=0 ; i<barrageNum ; i++ ) {
    if ( barrage[i]->type != 2 ) continue;
    if ( bossBullet == NULL ) {
      bl = addFoe(SCAN_WIDTH_8/2, SCAN_HEIGHT_8/5, barrage[i]->rank, 512, 0,
		  BOSS_TYPE, BOSS_SHIELD, barrage[i]->bulletml);
      bossBullet = bl;
    } else {
      bl = addFoeBossActiveBullet(SCAN_WIDTH_8/2, SCAN_HEIGHT_8/5, barrage[i]->rank, 512, 0,
				  barrage[i]->bulletml);
    }
  }
}
