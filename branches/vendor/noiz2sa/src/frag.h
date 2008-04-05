/*
 * $Id: frag.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Fragment data.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "vector.h"

typedef struct {
  Vector pos, vel;
  int width, height;
  int cnt;
  int spc;
} Frag;

#define FRAG_MAX 64

void initFrags();
void moveFrags();
void drawFrags();
void addShotFrag(Vector *pos);
void addEnemyFrag(Vector *p, int mx, int my, int type);
void addShipFrag(Vector *p);
void addClearFrag(Vector *p, Vector *v);
