/*
 * $Id: rand.c,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Make random number.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "rand.h"

static unsigned int multiplier = 8513;
static unsigned int addend = 179;

unsigned int nextRandInt(unsigned int *v) {
  *v = *v * multiplier + addend;
  return *v;
}
