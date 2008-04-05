/*
 * $Id: shot.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Shot data.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "vector.h"

typedef struct {
  Vector pos;
  int cnt;
} Shot;

#define SHOT_MAX 16

#define SHOT_SPEED 4096

#define SHOT_WIDTH 8
#define SHOT_HEIGHT 24

extern Shot shot[];

void initShots();
void moveShots();
void drawShots();
void addShot(Vector *pos);
