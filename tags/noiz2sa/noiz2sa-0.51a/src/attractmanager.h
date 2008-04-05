/*
 * $Id: attractmanager.h,v 1.3 2003/02/09 07:34:15 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Attraction manager header file.
 *
 * @version $Revision: 1.3 $
 */
#define STAGE_NUM 10
#define ENDLESS_STAGE_NUM 4
#define SCENE_NUM 10

typedef struct {
  int stageScore[STAGE_NUM+ENDLESS_STAGE_NUM];
  int sceneScore[STAGE_NUM][SCENE_NUM];
  int stage;
} HiScore;

extern int score, left, stage;

void loadPreference();
void savePreference();
void initGameState(int stg);
void addScore(int s);
int extendShip();
int decrementShip();
void addLeftBonus();
void setClearScore();
void setHiScore();
void showScore();
void drawScore();
void drawRPanel();
void initAttractManager();
int initTitleAtr();
void drawTitle();
void drawTitleMenu();
void initGameoverAtr();
void moveGameover();
void drawGameover();
void initStageClearAtr();
void moveStageClear();
void drawStageClear();
void moveTitleMenu();
void movePause();
void drawPause();
void drawTitle();
