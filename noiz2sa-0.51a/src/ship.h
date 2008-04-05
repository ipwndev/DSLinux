/*
 * $Id: ship.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Player data.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "vector.h"

typedef struct {
  Vector pos;
  int cnt, shotCnt;
  int speed;
  int invCnt;
} Ship;

extern Ship ship;

void initShip();
void moveShip();
void drawShip();
void destroyShip();
int getPlayerDeg(int x, int y);
