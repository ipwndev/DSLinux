/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */

#include <pixil_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#ifdef CONFIG_PAR
#include <par/par.h>
#endif

#define MWINCLUDECOLORS

#include "nano-X.h"
#include "nxsnake.h"

#include "text.h"
#include "levels.h"

#define BGCOLOR WHITE
#define FGCOLOR (GR_RGB(0x00, 0x66, 0xCC))
#define TXTCOLOR BLACK

static GR_WINDOW_ID tileset_pixmap = 0;

static GR_REGION_ID level_clipmask = 0;
static int draw_fullscreen = 1;

/* This matrix indicates where the snake, border and nibbles are */
/* This is how the engine determines where the snake, nibbles and */
/* borders are */

unsigned char playground[YUNITS][XUNITS];

snake_t global_snake;
GR_WINDOW_ID swindow, offscreen;

static int game_state = 0;
static int current_level = 0;

#define SNAKE_START_SPEED 110
static unsigned long game_speed;
static unsigned long start_speed = SNAKE_START_SPEED;

struct
{
    int active;
    int x;
    int y;
}
nibble;

int skipped = 0;

#define MOVE_ILLEGAL 0
#define MOVE_LEGAL   1
#define MOVE_NIBBLE  2

inline void
SET_SNAKE_DIRECTION(unsigned short *value, unsigned char dir)
{
    *value &= 0x0FFF;
    *value |= ((dir & 0xF) << 12);
}

inline void
SET_SNAKE_OFFSET(unsigned short *value, unsigned short off)
{
    *value &= 0xF000;
    *value |= (off & 0xFFF);
}

void
load_tileset(char *file)
{
    char path[256];
    GR_GC_ID gc = GrNewGC();

#ifdef CONFIG_PAR
    db_handle *par_db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (par_db) {
	par_getScreentopDir(par_db, "icondir", path, sizeof(path) - 1);
	db_closeDB(par_db);
	strcat(path, "/");
	strcat(path, file);
    } else
#endif
    {
	getcwd(path, 128);
	strcat(path, "/");
	strcat(path, file);
    }

    tileset_pixmap = GrNewPixmap(XUNITSIZE * TILE_COUNT, YUNITSIZE, 0);

    GrDrawImageFromFile(tileset_pixmap, gc, 0, 0,
			XUNITSIZE * TILE_COUNT, YUNITSIZE, path, 0);

    GrDestroyGC(gc);
}

void
draw_tile(int tile, int x, int y)
{
    GR_GC_ID gc = GrNewGC();
    GrCopyArea(offscreen, gc, x, y, XUNITSIZE, YUNITSIZE,
	       tileset_pixmap, tile * XUNITSIZE, 0, MWROP_SRCCOPY);

    GrDestroyGC(gc);
}

void
init_level(int l)
{
    global_snake.speed = 1;
    global_snake.headx = level[l].startx;
    global_snake.heady = level[l].starty;

    global_snake.tailpointer = 0;

    global_snake.length = 5;
    global_snake.growth = 0;

    bzero(global_snake.body, 256 * sizeof(unsigned short));

    SET_SNAKE_DIRECTION(&global_snake.body[0], level[l].dir);
    SET_SNAKE_OFFSET(&global_snake.body[0], global_snake.length);
}

int
advance_snake(void)
{
    unsigned short netoff = 0;
    int pos = 0;

    unsigned char dir;
    unsigned short off;

    short newx;
    short newy;

    int ret;

    if (!global_snake.body[0])
	return (MOVE_LEGAL);

    dir = GET_SNAKE_DIRECTION(global_snake.body[0]);

    newx = global_snake.headx;
    newy = global_snake.heady;


    switch (dir) {
    case SNAKE_DIR_LEFT:
	newx = global_snake.headx - 1;
	if (newx < 0)
	    newx = XUNITS - 1;
	break;

    case SNAKE_DIR_RIGHT:
	newx = global_snake.headx + 1;
	if (newx >= XUNITS)
	    newx = 0;
	break;

    case SNAKE_DIR_UP:
	newy = global_snake.heady - 1;
	if (newy < 0)
	    newy = YUNITS - 1;
	break;

    case SNAKE_DIR_DOWN:
	newy = global_snake.heady + 1;
	if (newy >= YUNITS)
	    newy = 0;
	break;
    }

    /* Now check the new position */
    switch (playground[newy][newx]) {
    case PLAYGROUND_EMPTY:
    case PLAYGROUND_TAIL:
	ret = MOVE_LEGAL;
	break;

    case PLAYGROUND_BORDER:
    case PLAYGROUND_SNAKE:
	return (MOVE_ILLEGAL);

    case PLAYGROUND_NIBBLE:
	/* Yummy... */
	global_snake.growth += 5;
	global_snake.score++;

	game_speed--;

	nibble.active = 0;
	ret = MOVE_NIBBLE;
	break;
    }

    /* Now advance the head.  That means advancing the absolute x and y */
    /* and adding one to the current offset */

    global_snake.headx = newx;
    global_snake.heady = newy;

    /* Finally, handle the tail */

    /* If we are growing, then handle that here (the tail doesn't move) */

    if (global_snake.growth) {
	global_snake.growth--;
	global_snake.length++;

	off = GET_SNAKE_OFFSET(global_snake.body[0]);
	SET_SNAKE_OFFSET(&global_snake.body[0], off + 1);
	return (ret);
    }

    /* If there is only one segment, then handle that here */

    if (!global_snake.tailpointer) {
	SET_SNAKE_OFFSET(&global_snake.body[0], global_snake.length);
	return (ret);
    }

    /* Otherwise, we need to move the whole snake */

    off = GET_SNAKE_OFFSET(global_snake.body[0]);
    SET_SNAKE_OFFSET(&global_snake.body[0], off + 1);

    while (global_snake.body[pos]) {
	off = GET_SNAKE_OFFSET(global_snake.body[pos]);

	/* If the size ends here, then clear the rest of the */
	/* snake */

	if (netoff + off >= global_snake.length) {
	    if (global_snake.tailpointer > pos + 1)
		printf
		    ("OOPS!  You have a gap between the length and the tail\n");

	    global_snake.body[pos + 1] = 0;
	    global_snake.tailpointer = pos;

	    SET_SNAKE_OFFSET(&global_snake.body[pos],
			     global_snake.length - netoff);
	    break;
	}

	netoff += off;
	pos++;
    }

    return (ret);
}

int
redirect_snake(GR_EVENT_KEYSTROKE event)
{
    unsigned char newdir = 0;
    int i = 0;

    unsigned char dir = GET_SNAKE_DIRECTION(global_snake.body[0]);

    switch (event.ch) {
    case MWKEY_UP:
    case MWKEY_KP8:
	newdir = SNAKE_DIR_UP;
	break;

    case MWKEY_DOWN:
    case MWKEY_KP2:
	newdir = SNAKE_DIR_DOWN;
	break;

    case MWKEY_LEFT:
    case MWKEY_KP4:
	newdir = SNAKE_DIR_LEFT;
	break;

    case MWKEY_RIGHT:
    case MWKEY_KP6:
	newdir = SNAKE_DIR_RIGHT;
	break;

    default:
	return (0);
    }

    if (dir == newdir)
	return (0);

    if (dir == SNAKE_DIR_LEFT && newdir == SNAKE_DIR_RIGHT)
	return (0);
    if (dir == SNAKE_DIR_RIGHT && newdir == SNAKE_DIR_LEFT)
	return (0);

    if (dir == SNAKE_DIR_UP && newdir == SNAKE_DIR_DOWN)
	return (0);
    if (dir == SNAKE_DIR_DOWN && newdir == SNAKE_DIR_UP)
	return (0);

    for (i = global_snake.tailpointer + 1; i > 0; i--)
	global_snake.body[i] = global_snake.body[i - 1];

    global_snake.tailpointer++;

    SET_SNAKE_DIRECTION(&global_snake.body[0], newdir);
    SET_SNAKE_OFFSET(&global_snake.body[0], 0);

    /* No bounds checking, that will be done on the next advance */
    return (1);
}

void
show_buffer(void)
{
    GR_GC_ID gc = GrNewGC();

#ifdef NOTUSED
    if (draw_fullscreen == 0 && level_clipmask)
	GrSetGCRegion(gc, level_clipmask);
#endif

    GrCopyArea(swindow, gc, 0, 0, WWIDTH, WHEIGHT,
	       offscreen, 0, 0, MWROP_SRCCOPY);

    GrDestroyGC(gc);
}

void
draw_string(char *array, int mode)
{
    char *txtptr = array;

    int xpos, ypos;

    int maxwidth = 0, maxheight = 0;

    int i = 0;
    int count = 0;

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    GrSetGCFont(gc, font);

    /* Count how many lines we have */
    while (txtptr) {
	int tw, th, tb;

	if (strlen(txtptr) == 0)
	    break;

	GrGetGCTextSize(gc, txtptr, -1, 0, &tw, &th, &tb);

	if (maxwidth < tw)
	    maxwidth = tw;
	if (maxheight < th)
	    maxheight = th;

	txtptr += 50;
	count++;
    }

    txtptr = array;
    maxwidth += 10;
    maxheight += 10;

    /* mode == 1, clear the screen,
       mode == 2, draw a box
     */

    if (mode == 1) {
	GrSetGCForeground(gc, BGCOLOR);
	GrFillRect(offscreen, gc, 0, 0, WWIDTH, WHEIGHT);

	GrSetGCForeground(gc, TXTCOLOR);
	GrSetGCBackground(gc, BGCOLOR);

    } else {
	xpos = (PWIDTH / 2) - (maxwidth / 2);
	ypos = (PHEIGHT / 2) - ((count * maxheight) / 2);

	/* Draw a box */
	GrSetGCForeground(gc, FGCOLOR);

	GrFillRect(offscreen, gc, xpos, ypos, maxwidth, (count * maxheight));

	GrSetGCForeground(gc, BGCOLOR);

	GrRect(offscreen, gc,
	       xpos + 3, ypos + 3, maxwidth - 6, (count * maxheight) - 6);

	GrSetGCBackground(gc, FGCOLOR);
    }

    ypos = (WHEIGHT / 2) - ((count * maxheight) / 2) + 5;

    while (1) {
	int tw, th, tb;
	if (strlen(txtptr) == 0)
	    break;

	GrGetGCTextSize(gc, txtptr, -1, 0, &tw, &th, &tb);

	GrText(offscreen, gc, (WWIDTH / 2) - (tw / 2),
	       ypos + (i * th), txtptr, -1, 0);
	i++;
	txtptr += 50;
    }

    GrDestroyGC(gc);
    GrDestroyFont(font);
}

/* Make a region that should clip out border */

void
make_clipmask(int l)
{

    unsigned char *ptr = level[l].bitmap;

    GR_RECT rect;
    int x, y;

    if (level_clipmask)
	GrDestroyRegion(level_clipmask);

    level_clipmask = GrNewRegion();

    for (y = 0; y < YUNITS; y++) {
	int state = 0;

	for (x = 0; x < XUNITS; x++) {
	    if (*ptr != PLAYGROUND_BORDER && !state) {
		rect.x = (x * XUNITSIZE);
		rect.y = (y * YUNITSIZE);
		rect.height = YUNITSIZE;

		state = 1;
	    }

	    if (*ptr == PLAYGROUND_BORDER && state) {
		rect.width = (x * XUNITSIZE) - rect.x;
		GrUnionRectWithRegion(level_clipmask, &rect);
		state = 0;
	    }

	    ptr++;
	}

	if (state) {
	    rect.width = (x * XUNITSIZE) - rect.x;
	    GrUnionRectWithRegion(level_clipmask, &rect);
	}
    }
}

void
draw_screen(int full)
{
    int bstart, bend;
    int x = 0, y = 0;

    GR_GC_ID gc = GrNewGC();

    /* Avoid drawing the background if we don't need to */

    if (draw_fullscreen == 0 && level_clipmask) {
	GrSetGCRegion(gc, level_clipmask);
    }

    for (y = 0; y < YUNITS; y++) {
	bstart = -1;
	bend = 0;

	for (x = 0; x < XUNITS; x++) {
	    if (playground[y][x] == PLAYGROUND_EMPTY) {
		if (bstart == -1)
		    bend = bstart = x;
		else
		    bend++;
		continue;
	    }

	    /* Draw the background block */

	    if (bstart != -1) {
		GrSetGCForeground(gc, BGCOLOR);

		GrFillRect(offscreen, gc, (bstart * XUNITSIZE),
			   (y * YUNITSIZE), (bend - bstart + 1) * XUNITSIZE,
			   YUNITSIZE);

		skipped += (bend - bstart);

		bstart = -1;
		bend = 0;
	    }

	    if (!full && playground[y][x] == PLAYGROUND_BORDER)
		continue;

	    switch (playground[y][x]) {
	    case PLAYGROUND_BORDER:
		draw_tile(TILE_BORDER, (x * XUNITSIZE), (y * YUNITSIZE));
		/* GrSetGCForeground(gc, BLUE); */
		break;

	    case PLAYGROUND_SNAKE:
	    case PLAYGROUND_TAIL:
		draw_tile(TILE_SNAKE, (x * XUNITSIZE), (y * YUNITSIZE));
		/* GrSetGCForeground(gc, RED); */
		break;

	    case PLAYGROUND_NIBBLE:
		draw_tile(TILE_NIBBLE, (x * XUNITSIZE), (y * YUNITSIZE));
		/* GrSetGCForeground(gc, BLACK); */
		break;

	    }

#ifdef NOTUSED
	    GrFillRect(offscreen, gc, (x * XUNITSIZE),
		       (y * YUNITSIZE), XUNITSIZE, YUNITSIZE);
#endif

#ifdef NOTUSED
	    if (playground[y][x] == PLAYGROUND_NIBBLE && nibble.active) {
		GR_POINT points[4];

		points[0].x = (x * XUNITSIZE) + 1;
		points[0].y = points[2].y =
		    (y * YUNITSIZE) + 1 + ((YUNITSIZE - 2) / 2);
		points[1].x = points[3].x =
		    (x * YUNITSIZE) + 1 + ((XUNITSIZE - 2) / 2);
		points[1].y = (y * YUNITSIZE) + 1;
		points[2].x = points[0].x + (XUNITSIZE - 2);
		points[3].y = points[1].y + (YUNITSIZE - 2);

		GrFillPoly(offscreen, gc, 4, points);
	    }


	    if (playground[y][x] == PLAYGROUND_NIBBLE && nibble.active) {
		int xpos = (x * XUNITSIZE) + (XUNITSIZE / 2);
		int ypos = (y * YUNITSIZE) + (YUNITSIZE / 2);

		GrSetGCForeground(gc, YELLOW);

		GrFillEllipse(offscreen, gc, xpos, ypos,
			      (XUNITSIZE / 2) - 1, (YUNITSIZE / 2) - 1);
	    }
#endif
	}


	/* If we have background clear up to the edge, handle that here */

	if (bstart != -1) {
	    GrSetGCForeground(gc, BGCOLOR);
	    GrFillRect(offscreen, gc, (bstart * XUNITSIZE), (y * YUNITSIZE),
		       (bend - bstart + 1) * XUNITSIZE, YUNITSIZE);

	    bend = bstart = 0;
	}
    }

    GrDestroyGC(gc);
}

void
draw_nibble(void)
{
    /* If there is no nibble assigned, then pick an new spot */

    if (!nibble.active) {
	while (1) {
	    int x = 0 + (int) (rand() % (XUNITS - 1));
	    int y = 0 + (int) (rand() % (YUNITS - 1));

	    if (playground[y][x] != PLAYGROUND_EMPTY)
		continue;

	    if (y > 0 && playground[y - 1][x] != PLAYGROUND_EMPTY)
		continue;

	    if (y < (YUNITS - 1) && playground[y + 1][x] != PLAYGROUND_EMPTY)
		continue;

	    if (x > 0 && playground[y][x - 1] != PLAYGROUND_EMPTY)
		continue;

	    if (x < (XUNITS - 1) && playground[y][x + 1] != PLAYGROUND_EMPTY)
		continue;

	    /* For now, make sure that the nibble doesn't */
	    /* show up near the border */


	    nibble.x = x;
	    nibble.y = y;
	    nibble.active = 1;
	    break;
	}
    }

    playground[nibble.y][nibble.x] = PLAYGROUND_NIBBLE;
}

void
draw_border(void)
{
#ifdef NOTUSED
    int y = 0;

    /* FIXME:  This should be more dynamic, eh? */
    /* For now, just a square box */

    memset(&playground[0][0], PLAYGROUND_BORDER, XUNITS);
    memset(&playground[YUNITS - 1][0], PLAYGROUND_BORDER, XUNITS);

    for (y = 0; y < YUNITS; y++) {
	playground[y][0] = PLAYGROUND_BORDER;
	playground[y][XUNITS - 1] = PLAYGROUND_BORDER;
    }
#endif

    memcpy(playground, level[current_level].bitmap, XUNITS * YUNITS);
}

/* This just draws the snake into the matrix */
/* draw_playground() actually puts it on the screen */

void
draw_snake(void)
{
    int i = 0;

    int sx = global_snake.headx;
    int sy = global_snake.heady;

    int ex = 0;
    int ey = 0;

    int pos = 0;

    while (global_snake.body[pos]) {
	unsigned char dir = GET_SNAKE_DIRECTION(global_snake.body[pos]);
	unsigned short off = GET_SNAKE_OFFSET(global_snake.body[pos]);

	switch (dir) {
	case SNAKE_DIR_RIGHT:

	    if (sx - off < 0) {
		int remainder;

		/* Split the line */
		memset(&playground[sy][0], PLAYGROUND_SNAKE, sx);

		remainder = off - sx - 1;

		memset(&playground[sy][XUNITS - remainder], PLAYGROUND_SNAKE,
		       remainder);

		ex = XUNITS - 1 - remainder;
	    } else {
		/* We can just memset the line, because it goes horizontal */
		memset(&playground[sy][sx - off], PLAYGROUND_SNAKE, off);
		ex = sx - off;
	    }

	    ey = sy;

	    break;

	case SNAKE_DIR_LEFT:

	    if (sx + off > (XUNITS - 1)) {
		int remainder;

		/* Split the line */
		memset(&playground[sy][sx], PLAYGROUND_SNAKE, XUNITS - sx);

		remainder = off - ((XUNITS - 1) - sx) - 1;

		memset(&playground[sy][0], PLAYGROUND_SNAKE, remainder);

		ex = remainder;
	    } else {
		/* We can just memset the line, because it goes horizontal */
		memset(&playground[sy][sx], PLAYGROUND_SNAKE, off);
		ex = sx + off;
	    }

	    ey = sy;
	    break;

	case SNAKE_DIR_DOWN:

	    ex = sx;

	    if (sy - off < 0) {
		int remainder;
		remainder = off - sy - 1;

		ey = YUNITS - 1 - remainder;

		for (i = 0; i <= sy; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;

		for (i = ey; i <= YUNITS - 1; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;
	    } else {
		ey = sy - off;

		for (i = ey; i <= sy; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;
	    }

	    break;

	case SNAKE_DIR_UP:
	    ex = sx;

	    if (sy + off > (YUNITS - 1)) {
		int remainder;

		for (i = sy; i <= YUNITS - 1; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;

		remainder = off - ((YUNITS - 1) - sy) - 1;

		for (i = 0; i <= remainder; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;

		ey = remainder;
	    } else {
		ey = sy + off;

		for (i = sy; i <= ey; i++)
		    playground[i][sx] = PLAYGROUND_SNAKE;

	    }

	    break;
	}

	sx = ex;
	sy = ey;
	pos++;
    }
}

void
draw_score(void)
{
    int tw, th, tb;

    char text[100];

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    GrSetGCForeground(gc, BGCOLOR);
    GrFillRect(offscreen, gc, 0, PHEIGHT + 1, WWIDTH, WHEIGHT - PHEIGHT);

    GrSetGCFont(gc, font);
    GrSetGCBackground(gc, BGCOLOR);
    GrSetGCForeground(gc, TXTCOLOR);

    sprintf(text, "Level: %d", current_level + 1);

    GrGetGCTextSize(gc, text, -1, GR_TFTOP, &tw, &th, &tb);

    GrText(offscreen, gc, 10,
	   PHEIGHT + ((WHEIGHT - PHEIGHT) / 2) - (th / 2),
	   text, -1, GR_TFTOP);

    sprintf(text, "Snakes: %d", global_snake.lives);

    GrGetGCTextSize(gc, text, -1, GR_TFTOP, &tw, &th, &tb);

    GrText(offscreen, gc, (WWIDTH / 2) - (tw / 2),
	   PHEIGHT + ((WHEIGHT - PHEIGHT) / 2) - (th / 2),
	   text, -1, GR_TFTOP);

    sprintf(text, "Score: %d", global_snake.score);

    GrGetGCTextSize(gc, text, -1, GR_TFTOP, &tw, &th, &tb);

    GrText(offscreen, gc, (WWIDTH - 10 - tw),
	   PHEIGHT + ((WHEIGHT - PHEIGHT) / 2) - (th / 2),
	   text, -1, GR_TFTOP);

    GrDestroyFont(font);
    GrDestroyGC(gc);
}

void
do_frame(int full)
{
    /* Clear it out */
    memset(playground, PLAYGROUND_EMPTY, XUNITS * YUNITS);

    /* Fill the matrix */
    draw_border();
    draw_snake();

    draw_nibble();

    /* Only draw the score if we need to */
    /* draw_score(); */

    /* Draw it to the offscreen buffer */
    draw_screen(full);

    /* And finally, show it on the screen */
    show_buffer();

    if (draw_fullscreen)
	draw_fullscreen = 0;
}

void
start_level(int l)
{
    init_level(l);		/* Initalize the snake */
    game_state = SNAKE_PLAYING;

    nibble.active = 0;
    nibble.x = 0;
    nibble.y = 0;

    /* Construct a clip mask to avoid drawing the border so many times */
    make_clipmask(l);

    /* Set us up to draw the full screen at least once */
    draw_fullscreen = 1;

    draw_score();
}

void
start_game(void)
{
    srand(time(0));
    current_level = 0;

    global_snake.lives = 3;
    global_snake.score = 0;

    game_speed = start_speed;

    start_level(current_level);
    draw_score();
    do_frame(1);
}

void
end_game(void)
{
    game_state = SNAKE_DONE;
    /* draw_score(); */
    draw_string((char *) welcome, 1);
    show_buffer();
}

void
do_snake_advance(void)
{
    switch (advance_snake()) {
    case MOVE_LEGAL:
	do_frame(0);
	break;

    case MOVE_NIBBLE:
	if ((global_snake.score % LEVEL_SCORE) == 0) {
	    current_level++;
	    game_state = SNAKE_NEXTLEVEL;
	    draw_string((char *) nextlevel, 2);
	    show_buffer();
	} else {
	    draw_score();
	}

	break;

    case MOVE_ILLEGAL:
	global_snake.lives--;

	if (!global_snake.lives)
	    end_game();
	else {
	    game_state = SNAKE_NEXTLEVEL;
	    draw_string((char *) snakedied, 2);
	    show_buffer();
	}

	break;
    }
}

void
handle_event(GR_EVENT * event)
{
    switch (event->type) {
    case GR_EVENT_TYPE_KEY_DOWN:

	/* Allow Q to quit no matter where we are */

	if (event->keystroke.ch == 'Q' || event->keystroke.ch == 'q') {
	    GrClose();
	    exit(0);
	}

	switch (game_state) {
	case SNAKE_START:
	case SNAKE_INSTRUCTIONS:
	case SNAKE_DONE:

	    switch (event->keystroke.ch) {
	    case 'I':
	    case 'i':
	    case MWKEY_APP1:
		if (game_state != SNAKE_INSTRUCTIONS) {
		    game_state = SNAKE_INSTRUCTIONS;
		    draw_string((char *) instructions, 1);
		    show_buffer();
		    break;
		}

	    default:
		start_game();
		break;
	    }

	    break;

	case SNAKE_PAUSED:
	    if (event->keystroke.ch == 'p' || event->keystroke.ch == 'P') {
		draw_score();
		do_frame(1);

		game_state = SNAKE_PLAYING;
	    }

	    break;

	case SNAKE_NEXTLEVEL:

	    if (current_level >= LEVELCOUNT)
		current_level = 0;

	    start_level(current_level);
	    draw_score();
	    do_frame(1);	/* and show the first frame */

	    break;

	case SNAKE_PLAYING:
	    if (event->keystroke.ch == 'p' ||
		event->keystroke.ch == 'P' ||
		event->keystroke.ch == MWKEY_RECORD) {
		game_state = SNAKE_PAUSED;
		break;
	    }

	    if (redirect_snake(event->keystroke))
		do_snake_advance();

	    break;
	}

	break;

    case GR_EVENT_TYPE_EXPOSURE:
	show_buffer();
	break;
    }
}

void
handle_idle()
{
    switch (game_state) {
    case SNAKE_START:
    case SNAKE_INSTRUCTIONS:
    case SNAKE_DONE:
    case SNAKE_NEXTLEVEL:
    case SNAKE_PAUSED:
	/* nothing to do here */
	break;

    case SNAKE_PLAYING:
	do_snake_advance();
	break;
    }
}


int
main(int argc, char **argv)
{
    extern char *optarg;

    while (1) {
	signed char c = getopt(argc, argv, "s::");

	if (c == -1)
	    break;

	if (c == 's') {
	    start_speed = atoi(optarg);
	    game_speed = start_speed;

	    printf("Setting the start speed to %ld\n", start_speed);
	}
    }

    if (GrOpen() < 0)
	exit(-1);

    load_tileset("snake.xpm");

    game_state = SNAKE_START;

    /* Make the window */

    swindow = GrNewWindowEx(WM_PROPS, "Pixil Snake", GR_ROOT_WINDOW_ID,
			    0, 0, WWIDTH, WHEIGHT, BGCOLOR);

    GrSelectEvents(swindow, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_CLOSE_REQ |
		   GR_EVENT_MASK_KEY_DOWN);

    offscreen = GrNewPixmap(WWIDTH, WHEIGHT, 0);

    GrMapWindow(swindow);

    /* Draw the instructions into the buffer */
    draw_string((char *) welcome, 1);

    while (1) {
	GR_EVENT event;

	/* We start at 130ms, but it goes down every */
	/* time a nibble is eaten */

	/* If they get this far, then they rock! */
	if (game_speed < 1)
	    game_speed = 1;

	GrGetNextEventTimeout(&event, game_speed);

	switch (event.type) {
	case GR_EVENT_TYPE_EXPOSURE:
	case GR_EVENT_TYPE_KEY_DOWN:
	    handle_event(&event);
	    break;

	case GR_EVENT_TYPE_TIMEOUT:
	    handle_idle();
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);

	}
    }
}
