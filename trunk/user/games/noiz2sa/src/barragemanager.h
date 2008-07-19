/*
 * $Id: barragemanager.h,v 1.2 2003/08/10 04:09:46 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Barrage data.
 *
 * @version $Revision: 1.2 $
 */
#ifndef BARRAGEMANAGER_H_
#define BARRAGEMANAGER_H_

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"

#define BARRAGE_TYPE_NUM 3
#define BARRAGE_MAX 16

#define BOSS_TYPE 3

typedef struct {
  BulletMLParser *bulletml;
  double maxRank, rank;
  int type;
  int frq;
} Barrage;

extern "C" {
#include "brgmng_mtd.h"
}
#endif
