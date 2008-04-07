/*
 * $Id: background.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Backgournd graphics data.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "vector.h"

typedef struct {
  int x, y, z;
  int width, height;
} Board;

#define BOARD_MAX 256

void initBackground();
void setStageBackground(int stage);
void moveBackground();
void drawBackground();
