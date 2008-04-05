/*
 * $Id: letterrender.h,v 1.1.1.1 2002/11/03 11:08:24 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Letter render header file.
 *
 * @version $Revision: 1.1.1.1 $
 */

void drawLetterBuf(int idx, int lx, int ly, int ltSize, int d,
		LayerBit color1, LayerBit color2, LayerBit *buf, int panel);
void drawLetter(int idx, int lx, int ly, int ltSize, int d,
		LayerBit color1, LayerBit color2, LayerBit *buf);
void drawStringBuf(char *str, int lx, int ly, int ltSize, int d, 
		LayerBit color1, LayerBit color2, LayerBit *buf, int panel);
void drawString(char *str, int lx, int ly, int ltSize, int d, 
		LayerBit color1, LayerBit color2, LayerBit *buf);
