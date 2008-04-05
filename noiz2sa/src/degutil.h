/*
 * $Id: degutil.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Degutil header file.
 *
 * @version $Revision: 1.1.1.1 $
 */
#define DIV 1024
#define TAN_TABLE_SIZE 1024
#define SC_TABLE_SIZE 1024

extern int sctbl[];

void initDegutil();
int getDeg(int x, int y);
int getDistance(int x, int y);
