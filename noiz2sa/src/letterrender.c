/*
 * $Id: letterrender.c,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Letter render.
 *
 * @version $Revision: 1.1.1.1 $
 */
#include "screen.h"
#include "letterrender.h"
#include "letterdata.h"

void drawLetterBuf(int idx, int lx, int ly, int ltSize, int d,
		LayerBit color1, LayerBit color2, LayerBit *buf, int panel) {
  int i;
  float x, y, length, size, t;
  int deg;
  for ( i=0 ; ; i++ ) {
    deg = (int)spData[idx][i][4];
    if ( deg > 99990 ) break;
    x = spData[idx][i][0]; y = spData[idx][i][1];
    size = spData[idx][i][2]; length = spData[idx][i][3];
    size *= 1.3f; length *= 1.1f;
    switch ( d ) {
    case 0:
      x = -x; y = y;
      break;
    case 1:
      t = x; x = -y; y = -t;
      deg += 90;
      break;
    case 2:
      x = x; y = -y;
      deg += 180;
      break;
    case 3:
      t = x; x = y; y = t;
      deg += 270;
      break;
    }
    deg %= 180;
    if ( panel ) {
      if ( deg < 45 || deg > 135 ) {
	drawBoxPanel((int)(x*ltSize)+lx, (int)(y*ltSize)+ly, 
		     (int)(size*ltSize), (int)(length*ltSize), color1, color2, buf);
      } else {
	drawBoxPanel((int)(x*ltSize)+lx, (int)(y*ltSize)+ly, 
		     (int)(length*ltSize), (int)(size*ltSize), color1, color2, buf);
      }
    } else {
      if ( deg <= 45 || deg > 135 ) {
	drawBox((int)(x*ltSize)+lx, (int)(y*ltSize)+ly, 
		     (int)(size*ltSize), (int)(length*ltSize), color1, color2, buf);
      } else {
	drawBox((int)(x*ltSize)+lx, (int)(y*ltSize)+ly, 
		     (int)(length*ltSize), (int)(size*ltSize), color1, color2, buf);
      }
    }
  }
}
		       
void drawLetter(int idx, int lx, int ly, int ltSize, int d,
		LayerBit color1, LayerBit color2, LayerBit *buf) {
  drawLetterBuf(idx, lx, ly, ltSize, d, color1, color2, buf, 1);
}

void drawStringBuf(char *str, int lx, int ly, int ltSize, int d, 
		LayerBit color1, LayerBit color2, LayerBit *buf, int panel) {
  int x = lx, y = ly;
  int i, c, idx;
  for ( i=0 ; ; i++ ) {
    if ( str[i] == '\0' ) break;
    c = str[i];
    if ( c != ' ' ) {
      if ( c >= '0' && c <='9' ) {
	idx = c-'0';
      } else if ( c >= 'A' && c <= 'Z' ) {
	idx = c-'A'+10;
      } else if ( c >= 'a' && c <= 'z' ) {
	idx = c-'a'+10;
      } else if ( c == '.' ) {
	idx = 36;
      } else if ( c == '-' ) {
	idx = 38;
      } else if ( c == '+' ) {
	idx = 39;
      } else {
	idx = 37;
      }
      drawLetterBuf(idx, x, y, ltSize, d, color1, color2, buf, panel);
    }
    switch ( d ) {
    case 0:
      x -= ltSize*1.7f;
      break;
    case 1:
      y -= ltSize*1.7f;
      break;
    case 2:
      x += ltSize*1.7f;
      break;
    case 3:
      y += ltSize*1.7f;
      break;
    }
  }
}

void drawString(char *str, int lx, int ly, int ltSize, int d, 
		LayerBit color1, LayerBit color2, LayerBit *buf) {
  drawStringBuf(str, lx, ly, ltSize, d, color1, color2, buf, 1);
}

