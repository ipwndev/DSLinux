/*
 * $Id: noiz2sa.h,v 1.4 2003/02/09 07:34:16 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Noiz2sa header file.
 *
 * @version $Revision: 1.4 $
 */
#define randN(N) (rand()%(N))
#define randNS(N) (rand()%(N<<1)-N)
#define randNS2(N) ((rand()%(N)-(N>>1)) + (rand()%(N)-(N>>1)))
#define absN(a) ((a) < 0 ? - (a) : (a))

#define INTERVAL_BASE 16

#define CAPTION "Noiz2sa"
#define VERSION_NUM 50

#define NOT_EXIST -999999

extern int status;
extern int interval;
extern int tick;

#define TITLE 0
#define IN_GAME 1
#define GAMEOVER 2
#define STAGE_CLEAR 3
#define PAUSE 4

void quitLast();
void initTitleStage(int stg);
void initTitle();
void initGame(int stg);
void initGameover();
void initStageClear();
