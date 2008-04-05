/*
 * $Id: background.c,v 1.2 2002/12/31 09:34:34 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle background graphics.
 *
 * @version $Revision: 1.2 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "noiz2sa.h"
#include "screen.h"
#include "vector.h"
#include "background.h"

static Board board[BOARD_MAX];

void initBackground() {
  int i;
  for ( i=0 ; i<BOARD_MAX ; i++ ) {
    board[i].width = NOT_EXIST;
  }
}

static int bdIdx;
static int boardMx, boardMy;
static int boardRepx, boardRepy;
static int boardRepXn, boardRepYn;

static void addBoard(int x, int y, int z, int width, int height) {
  if ( bdIdx >= BOARD_MAX ) return;
  board[bdIdx].x = x;
  board[bdIdx].y = y;
  board[bdIdx].z = z;
  board[bdIdx].width = width/z;
  board[bdIdx].height = height/z;
  bdIdx++;
}

void setStageBackground(int bn) {
  int i, j, k;
  bdIdx = 0;

  switch ( bn ) {
  case 0:
  case 6:
    addBoard(9000, 9000, 500, 25000, 25000);
    for ( i=0 ; i<4 ; i++ ) {
      for ( j=0 ; j<4 ; j++ ) {
	if ( i > 1 || j > 1 ) {
	  addBoard(i*16384, j*16384, 500, 10000+(i*12345)%3000, 10000+(j*54321)%3000);
	}
      }
    }
    for ( j=0 ; j<8 ; j++ ) {
      for ( i=0 ; i<4 ; i++ ) {
	addBoard(0, i*16384, 500-j*50, 20000-j*1000, 12000-j*500);
      }
    }
    for ( i=0 ; i<8 ; i++ ) {
      addBoard(0, i*8192, 100, 20000, 6400);
    }
    if ( bn == 0 ) {
      boardMx = 40; boardMy = 300;
    } else {
      boardMx = -40; boardMy = 480;
    }
    boardRepx = boardRepy = 65536;
    boardRepXn = boardRepYn = 4;
    break;
  case 1:
    addBoard(12000, 12000, 400, 48000, 48000);
    addBoard(12000, 44000, 400, 48000, 8000);
    addBoard(44000, 12000, 400, 8000, 48000);
    for ( i=0 ; i<16 ; i++ ) {
      addBoard(0, 0, 400-i*10, 16000, 16000);
      if ( i < 6 ) {
	addBoard(9600, 16000, 400-i*10, 40000, 16000);
      }
    }
    boardMx = 128; boardMy = 512;
    boardRepx = boardRepy = 65536;
    boardRepXn = boardRepYn = 4;
    break;
  case 2:
    for ( i=0 ; i<16 ; i++ ) {
      addBoard(7000+i*3000, 0, 1600-i*100, 24000, 5000);
      addBoard(7000+i*3000, 50000, 1600-i*100, 4000, 10000);
      addBoard(-7000-i*3000, 0, 1600-i*100, 24000, 5000);
      addBoard(-7000-i*3000, 50000, 1600-i*100, 4000, 10000);
    }
    boardMx = 0; boardMy = 1200;
    boardRepx = 0;
    boardRepy = 65536;
    boardRepXn = 1;
    boardRepYn = 10;
    break;
  case 3:
    addBoard(9000, 9000, 500, 30000, 30000);
    for ( i=0 ; i<4 ; i++ ) {
      for ( j=0 ; j<4 ; j++ ) {
	if ( i > 1 || j > 1 ) {
	  addBoard(i*16384, j*16384, 500, 12000+(i*12345)%3000, 12000+(j*54321)%3000);
	}
      }
    }
    for ( i=0 ; i<4 ; i++ ) {
      for ( j=0 ; j<4 ; j++ ) {
	if ( (i > 1 || j > 1) && (i+j)%3 == 0 ) {
	  addBoard(i*16384, j*16384, 480, 9000+(i*12345)%3000, 9000+(j*54321)%3000);
	}
      }
    }
    addBoard(9000, 9000, 480, 20000, 20000);
    addBoard(9000, 9000, 450, 20000, 20000);
    addBoard(32768, 40000, 420, 65536, 5000);
    addBoard(30000, 32768, 370, 4800, 65536);
    addBoard(32768, 0, 8, 65536, 10000);
    boardMx = 10; boardMy = 100;
    boardRepx = boardRepy = 65536;
    boardRepXn = boardRepYn = 4;
    break;
  case 4:
    addBoard(32000, 12000, 160, 48000, 48000);
    addBoard(32000, 44000, 160, 48000, 8000);
    addBoard(64000, 12000, 160, 8000, 48000);
    for ( i=0 ; i<16 ; i++ ) {
      addBoard(20000, 0, 160-i*10, 16000, 16000);
      if ( i < 6 ) {
	addBoard(29600, 16000, 160-i*10, 40000, 16000);
      }
    }
    boardMx = 0; boardMy = 128;
    boardRepx = boardRepy = 65536;
    boardRepXn = 2; boardRepYn = 2;
    break;
  case 5:
    for ( k=0 ; k<5 ; k++ ) {
      j = 0;
      for ( i=0 ; i<16 ; i++ ) {
	addBoard(j, i*4096, 200-k*10, 16000, 4096);
	addBoard(j+16000-j*2, i*4096, 200-k*10, 16000, 4096);
	if ( i < 4 ) j += 2000;
	else if ( i < 6 ) j -= 3500;
	else if ( i < 12 ) j += 1500;
	else j -= 2000;
      }
    }
    boardMx = -10; boardMy = 25;
    boardRepx = boardRepy = 65536;
    boardRepXn = boardRepYn = 2;
    break;
  }
}

void moveBackground() {
  int i;
  Board *bd;
  for ( i=0 ; i<BOARD_MAX ; i++ ) {
    if ( board[i].width == NOT_EXIST ) continue;
    bd = &(board[i]);
    bd->x += boardMx; bd->y += boardMy;
    bd->x &= (boardRepx-1); bd->y &= (boardRepy-1);
  }
}

void drawBackground() {
  int i;
  Board *bd;
  int ox, oy, osx, osy, rx, ry;
  osx = -boardRepx * (boardRepXn/2);
  osy = -boardRepy * (boardRepYn/2);
  for ( i=0 ; i<BOARD_MAX ; i++ ) {
    if ( board[i].width == NOT_EXIST ) continue;
    bd = &(board[i]);
    ox = osx; 
    for ( rx = 0 ; rx < boardRepXn ; rx++, ox += boardRepx ) {
      oy = osy;
      for ( ry = 0 ; ry < boardRepYn ; ry++, oy += boardRepy ) {
	drawBox((bd->x+ox)/bd->z+LAYER_WIDTH/2, (bd->y+oy)/bd->z+LAYER_HEIGHT/2, 
		 bd->width, bd->height, 1, 3, l1buf);
      }
    }
  }
}
