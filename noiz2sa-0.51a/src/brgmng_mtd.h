/*
 * $Id: brgmng_mtd.h,v 1.2 2002/12/31 09:34:34 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Stage data.
 *
 * @version $Revision: 1.2 $
 */
void initBarragemanager();
void closeBarragemanager();
void initBarrages(int seed, float startLevel, float li);
void setBarrages(float level, int bm, int midMode);
void addBullets();
void addBossBullet();
void bossDestroied();

extern int scene;
extern int endless, insane;
