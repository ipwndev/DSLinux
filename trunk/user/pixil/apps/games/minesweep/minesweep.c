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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <getopt.h>
#include <nano-X.h>
#include <nxcolors.h>
#include <unistd.h>

#ifdef CONFIG_PAR
#include <par/par.h>
#endif

#include "minesweep.h"

#define WM_PROPS    (GR_WM_PROPS_MAXIMIZE | GR_WM_PROPS_BORDER |\
		    GR_WM_PROPS_CAPTION |\
                    GR_WM_PROPS_CLOSEBOX)

/* The actual playing field */
static unsigned char minefield[14][14];

/* Various pixmaps and windows */
static GR_WINDOW_ID main_wid, minefield_wid, button_wid, scores_wid;
static GR_WINDOW_ID minefield_pixmap = 0, tileset_pixmap = 0;

/* Global variables */
static GR_FONT_ID g_font;

static int g_width, g_height, g_bombs;

/* The game info structure */

struct
{
    unsigned char state;
    unsigned char flags_set;
    unsigned char tiles_cleared;
    int start_time;
    int timer;
}
game_info;

/* The number colors will alternate between these three colors */
static GR_COLOR text_colors[3] =
  { GR_COLOR_BLUE, GR_COLOR_GREEN, GR_COLOR_RED };

#define KEY_UP 0x01

static unsigned short g_keyflags = 0;

typedef struct
{
    unsigned long date;
    int seconds;
}
score_t;

static score_t g_scores[10];

struct
{
    char str[25];
    GR_WINDOW_ID wid;
}
menus[] =
{
    {
    "Scores", 0}
    ,
	/*  { "Difficulty", 0 }, */
    {
    "", 0}
};

/* Advance declarations */

void start_game(void);
void clear_tile(int x, int y);
void draw_button(int state);

/* Read the top 10 high scores from the file */

void
load_scores(char *filename, score_t * scores)
{
    int i = 0;

    FILE *stream = fopen(filename, "r");
    if (!stream)
	return;

    for (i = 0; i < 10; i++)
	fscanf(stream, "%ld %d\n", &scores[i].date, &scores[i].seconds);

    fclose(stream);
}

void
save_scores(char *filename, score_t * scores)
{
    int i = 0;

    FILE *stream = fopen(filename, "w+");
    if (!stream)
	return;

    for (i = 0; i < 10; i++)
	fprintf(stream, "%ld %d\n", scores[i].date, scores[i].seconds);

    fclose(stream);
}

void
draw_scores(void)
{

    int ypos = 25;
    int i = 0;

    GR_GC_ID gc = GrNewGC();
    GrSetGCFont(gc, g_font);

    GrSetGCForeground(gc, GR_COLOR_BLACK);
    GrSetGCBackground(gc, GR_COLOR_WHITE);

    GrText(scores_wid, gc, 5, 5, "Minesweeper High Scores", -1, GR_TFTOP);

    for (i = 0; i < 10; i++, ypos += 15) {
	char str[50];
	char *ptr = str;

	if (i < 9)
	    ptr += sprintf(ptr, " %d. ", i + 1);
	else
	    ptr += sprintf(ptr, "%d. ", i + 1);

	GrText(scores_wid, gc, 10, ypos, str, -1, GR_TFTOP);

	ptr = str;

	if (g_scores[i].seconds) {
	    struct tm *tm = localtime(&g_scores[i].date);

	    ptr += sprintf(ptr, "%2.2d/%2.2d/%2.2d %2.2d:%2.2d",
			   tm->tm_mon + 1, tm->tm_mday,
			   (tm->tm_year + 1900) % 1000, tm->tm_hour,
			   tm->tm_min);

	    ptr += sprintf(ptr, "    %2.2d:%2.2d",
			   g_scores[i].seconds / 60,
			   g_scores[i].seconds % 60);
	} else
	    ptr += sprintf(ptr, "---------- --:--");

	GrText(scores_wid, gc, 30, ypos, str, -1, GR_TFTOP);
    }

    ypos += 15;
    GrText(scores_wid, gc, 5, ypos, "Click the window to close...", -1,
	   GR_TFTOP);

    GrDestroyGC(gc);
}

void
show_scores(void)
{

    if (!scores_wid) {
	scores_wid =
	    GrNewWindowEx(GR_WM_PROPS_NODECORATE, "Scores", main_wid, 0, 0,
			  240, 300, GR_COLOR_WHITE);

	GrSelectEvents(scores_wid,
		       GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);
    }

    GrMapWindow(scores_wid);
}


void
load_tileset(char *filename)
{
    GR_GC_ID gc = GrNewGC();
    char path[256];

#ifdef CONFIG_PAR
    db_handle *par_db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (par_db) {
	par_getScreentopDir(par_db, "icondir", path, sizeof(path) - 1);
	db_closeDB(par_db);
	strcat(path, "/");
	strcat(path, filename);
    } else
#endif
    {
	getcwd(path, sizeof(path));
	strcat(path, "/");
	strcat(path, filename);
    }

    /* Create the pixmap if needed */

    if (!tileset_pixmap)
	tileset_pixmap = GrNewPixmap(TILE_WIDTH * 6,
				     TILE_HEIGHT + BUTTON_HEIGHT, 0);

    /* Load the image directly from the file */
    printf("Trying to draw the image from %s\n", path);

    GrDrawImageFromFile(tileset_pixmap, gc, 0, 0, -1, -1, path, 0);
    GrDestroyGC(gc);
}

void
init_field(void)
{
    int i, x, y;
    int field_size = g_height * g_width;

    /* Reset each tile to be bomb free and regular */

    for (y = 0; y < g_height; y++)
	for (x = 0; x < g_width; x++) {
	    minefield[y][x] = 0;
	    SET_TILE(minefield[y][x], TILE_REGULAR);
	}

    /* Add the given number of bombs to the field  */

    for (i = 0; i < g_bombs; i++) {
	int pos;

	do
	    pos = rand() % field_size;
	while ((minefield[pos / g_height][pos % g_width]) & BOMB_FLAG);

	minefield[pos / g_height][pos % g_width] |= BOMB_FLAG;
    }
}

void
draw_menus(void)
{
    int mx = 5;
    int i = 0;

    GR_GC_ID gc = GrNewGC();

    GrSetGCForeground(gc, GR_COLOR_BLACK);
    GrSetGCBackground(gc, GR_COLOR_WHITE);

    GrSetGCFont(gc, g_font);

    while (strlen(menus[i].str)) {
	int tw, th, tb;

	GrGetGCTextSize(gc, menus[i].str, -1, GR_TFTOP, &tw, &th, &tb);
	GrText(main_wid, gc, mx, 5, menus[i].str, -1, GR_TFTOP);
	GrLine(main_wid, gc, mx, 6 + tb, mx + tw, 6 + tb);

	if (!menus[i].wid) {
	    menus[i].wid = GrNewInputWindow(main_wid, mx, 5, tw, th);
	    GrSelectEvents(menus[i].wid, GR_EVENT_MASK_BUTTON_DOWN);
	    GrMapWindow(menus[i].wid);
	}

	mx += (tw + 10);
	i++;
    }
}

/* Draw the main window */

void
draw_main(void)
{
    char flagstr[32];
    char timestr[32];

    int w = (240 - TILE_WIDTH * g_width) / 2;
    int h =
	((300 - TILE_HEIGHT * g_height) / 2) + (TILE_HEIGHT * g_height) + 5;

    int tw, th, tb;
    GR_GC_ID gc = GrNewGC();

    GrSetGCForeground(gc, GR_COLOR_BLACK);
    GrSetGCBackground(gc, GR_COLOR_WHITE);

    GrSetGCFont(gc, g_font);

    sprintf(flagstr, "Flags: %2.2d/%2.2d", game_info.flags_set, g_bombs);
    GrText(main_wid, gc, w, h, flagstr, -1, GR_TFTOP);

    sprintf(timestr, "Time: %2.2d:%2.2d",
	    game_info.timer / 60, game_info.timer % 60);

    GrGetGCTextSize(gc, timestr, -1, GR_TFTOP, &tw, &th, &tb);

    GrText(main_wid, gc, w + (TILE_WIDTH * g_width) - tw, h,
	   timestr, -1, GR_TFTOP);

    GrDestroyGC(gc);

    draw_menus();
}

void
draw_tile(int tile, int x, int y)
{

    GR_GC_ID gc = GrNewGC();

    GrCopyArea(minefield_pixmap, gc, (x * TILE_WIDTH), (y * TILE_HEIGHT),
	       TILE_WIDTH, TILE_HEIGHT, tileset_pixmap,
	       (tile - 1) * TILE_WIDTH, 0, MWROP_SRCCOPY);

    GrDestroyGC(gc);
}

int
check_neighbors(int x, int y)
{

    int count = 0;

    if (x > 0) {
	if (minefield[y][x - 1] & BOMB_FLAG)
	    count++;

	if (y > 0)
	    if (minefield[y - 1][x - 1] & BOMB_FLAG)
		count++;

	if (y < (g_height) - 1)
	    if (minefield[y + 1][x - 1] & BOMB_FLAG)
		count++;
    }

    if (x < (g_width) - 1) {

	if (minefield[y][x + 1] & BOMB_FLAG)
	    count++;

	if (y > 0)
	    if (minefield[y - 1][x + 1] & BOMB_FLAG)
		count++;

	if (y < (g_height) - 1)
	    if (minefield[y + 1][x + 1] & BOMB_FLAG)
		count++;
    }

    if (y > 0)
	if (minefield[y - 1][x] & BOMB_FLAG)
	    count++;

    if (y < (g_height) - 1)
	if (minefield[y + 1][x] & BOMB_FLAG)
	    count++;

    return (count);
}

void
clear_neighbors(int x, int y)
{

    if (x > 0) {
	clear_tile(x - 1, y);
	if (y > 0)
	    clear_tile(x - 1, y - 1);
	if (y < g_height - 1)
	    clear_tile(x - 1, y + 1);
    }

    if (x < g_width - 1) {
	clear_tile(x + 1, y);
	if (y > 0)
	    clear_tile(x + 1, y - 1);
	if (y < g_height - 1)
	    clear_tile(x + 1, y + 1);
    }

    if (y > 0)
	clear_tile(x, y - 1);
    if (y < g_height - 1)
	clear_tile(x, y + 1);
}

void
draw_empty(int x, int y)
{

    int count;
    GR_GC_ID gc = GrNewGC();

    GrSetGCForeground(gc, GR_RGB(0xC5, 0xC2, 0xC5));

    draw_tile(TILE_DEPRESSED, x, y);

    /* GrFillRect(minefield_pixmap, gc, 
       (x * TILE_WIDTH), (y * TILE_HEIGHT),
       TILE_WIDTH, TILE_HEIGHT); */

    count = check_neighbors(x, y);

    /* If there are bombs around us, then print how many there are */

    if (count) {
	char ch;
	int w, h, b;

	GrSetGCForeground(gc, text_colors[count % 3]);
	GrSetGCUseBackground(gc, 0);
	GrSetGCFont(gc, g_font);

	ch = 48 + count;

	GrGetGCTextSize(gc, &ch, 1, GR_TFTOP, &w, &h, &b);

	GrText(minefield_pixmap, gc,
	       (x * TILE_WIDTH) + ((TILE_WIDTH - w) / 2),
	       (y * TILE_HEIGHT) + ((TILE_HEIGHT - h) / 2), &ch, 1, GR_TFTOP);
    }

    GrDestroyGC(gc);
}

void
draw_field(void)
{

    int x, y;

    GR_GC_ID gc = GrNewGC();

    for (y = 0; y < g_height; y++) {
	for (x = 0; x < g_width; x++) {

	    switch (GET_TILE(minefield[y][x])) {
	    case TILE_REGULAR:
	    case TILE_BOMB:
	    case TILE_FLAG:
	    case TILE_WRONG:
		draw_tile(GET_TILE(minefield[y][x]), x, y);
		break;

	    case TILE_EMPTY:
		draw_empty(x, y);
		break;
	    }
	}
    }

    GrCopyArea(minefield_wid, gc, 0, 0,
	       TILE_WIDTH * g_width,
	       TILE_HEIGHT * g_height, minefield_pixmap, 0, 0, MWROP_SRCCOPY);

    GrDestroyGC(gc);
}

void
set_bomb(int x, int y)
{
    CLEAR_TILE(minefield[y][x]);
    SET_TILE(minefield[y][x], TILE_BOMB);
}

void
set_wrong(int x, int y)
{

    CLEAR_TILE(minefield[y][x]);
    SET_TILE(minefield[y][x], TILE_WRONG);
}

void
set_empty(int x, int y)
{
    CLEAR_TILE(minefield[y][x]);
    SET_TILE(minefield[y][x], TILE_EMPTY);
    game_info.tiles_cleared++;
}

void
clear_tile(int x, int y)
{

    /* Only proceed if this tile has been unchecked */
    if (GET_TILE(minefield[y][x]) == TILE_EMPTY ||
	GET_TILE(minefield[y][x]) == TILE_FLAG)
	return;

    /* If this tile is hiding a bomb, then return */
    if (minefield[y][x] & BOMB_FLAG)
	return;

    set_empty(x, y);

    if (check_neighbors(x, y) != 0)
	return;

    /* Otherwise, see if we can't clear our any of our neighbors too */
    clear_neighbors(x, y);
}

int
is_bomb(int x, int y)
{
    if (minefield[y][x] & BOMB_FLAG)
	if (GET_TILE(minefield[y][x]) != TILE_FLAG)
	    return (1);

    return (0);
}

/* Immediately set the user up as teh winner */

void
handle_cheat(void)
{
    int r, s;

    game_info.tiles_cleared = 0;
    game_info.flags_set = 0;

    for (r = 0; r < g_height; r++)
	for (s = 0; s < g_width; s++) {

	    if (minefield[r][s] & BOMB_FLAG) {
		CLEAR_TILE(minefield[r][s]);
		SET_TILE(minefield[r][s], TILE_FLAG);
		game_info.flags_set++;
	    } else {
		CLEAR_TILE(minefield[r][s]);
		SET_TILE(minefield[r][s], TILE_EMPTY);
		game_info.tiles_cleared++;
	    }
	}

    draw_field();
}

void
handle_minefield_down(GR_EVENT_BUTTON * button)
{

    int x = button->x / TILE_WIDTH;
    int y = button->y / TILE_HEIGHT;

    if (game_info.state == GAME_OVER)
	return;

    /* On the X86 demo, we use a right button press here, otherwise, 
       on the PDAs, we use the key */

#ifdef CONFIG_PLATFORM_X86DEMO
    if (button->buttons & MWBUTTON_R) 
#else
    if ((g_keyflags & KEY_UP) == KEY_UP) 
#endif
      {
	switch (GET_TILE(minefield[y][x])) {
	case TILE_REGULAR:
	    if (game_info.flags_set == g_bombs)
		return;

	    CLEAR_TILE(minefield[y][x]);
	    SET_TILE(minefield[y][x], TILE_FLAG);
	    game_info.flags_set++;

	    break;

	case TILE_FLAG:
	    CLEAR_TILE(minefield[y][x]);
	    SET_TILE(minefield[y][x], TILE_REGULAR);
	    game_info.flags_set--;

	    break;

	default:
	    break;
	}
    } else {
	if (GET_TILE(minefield[y][x]) == TILE_EMPTY) {
	    int bcount = 0;

	    if (x > 0) {
		bcount += is_bomb(x - 1, y);
		if (y > 0)
		    bcount += is_bomb(x - 1, y - 1);
		if (y < g_height - 1)
		    bcount += is_bomb(x - 1, y + 1);
	    }

	    if (x < g_width - 1) {
		bcount += is_bomb(x + 1, y);
		if (y > 0)
		    bcount += is_bomb(x + 1, y - 1);
		if (y < g_height - 1)
		    bcount += is_bomb(x + 1, y + 1);
	    }

	    if (y > 0)
		bcount += is_bomb(x, y - 1);
	    if (y < g_height - 1)
		bcount += is_bomb(x, y + 1);

	    if (!bcount)
		clear_neighbors(x, y);
	} else {

	    if (GET_TILE(minefield[y][x]) == TILE_FLAG)
		goto dofield;

	    if (minefield[y][x] & BOMB_FLAG) {
		int r, s;

		game_info.state = GAME_OVER;

		for (r = 0; r < g_height; r++)
		    for (s = 0; s < g_width; s++) {

			if (minefield[r][s] & BOMB_FLAG) {
			    if (GET_TILE(minefield[r][s]) != TILE_FLAG)
				set_bomb(s, r);
			} else {
			    if (GET_TILE(minefield[r][s]) == TILE_FLAG)
				set_wrong(s, r);
			}
		    }

		draw_button(BUTTON_WRONG);
	    } else
		clear_tile(x, y);
	}
    }

  dofield:
    /* Start the timer if the game is active */

    if (game_info.state == GAME_START) {
	game_info.state = GAME_ACTIVE;
	game_info.start_time = time(0);
    }

    draw_field();
}

void
draw_button(int state)
{

    GR_GC_ID gc = GrNewGC();

    GrCopyArea(button_wid, gc, 0, 0,
	       BUTTON_WIDTH, BUTTON_HEIGHT, tileset_pixmap,
	       (state * BUTTON_WIDTH), TILE_HEIGHT, MWROP_SRCCOPY);

    GrDestroyGC(gc);
}

void
handle_button(GR_EVENT_BUTTON * button, int state)
{

    draw_button(state);

    if (state == 1) {
	start_game();
	draw_field();
    }
}

void
start_game(void)
{

    time_t val = time(0);

    /* Reseed the randomizer */
    srand(val);

    /* Initalize the mine field */
    init_field();

    /* Set some variables */

    game_info.flags_set = 0;
    game_info.tiles_cleared = 0;

    game_info.timer = 0;
    game_info.start_time = 0;

    game_info.state = GAME_START;
}

int
main(int argc, char **argv)
{

    int w, h;

    /* The default game width and height */

    g_width = g_height = 10;
    g_bombs = 0;

    while (1) {
	signed char ch = getopt(argc, argv, "w:h:b:");
	if (ch == -1)
	    break;

	switch (ch) {
	case 'w':
	    g_width = atoi(optarg);
	    if (g_width > 14)
		g_width = 14;
	    if (g_width < 7)
		g_width = 7;
	    break;

	case 'h':
	    g_height = atoi(optarg);
	    if (g_height > 14)
		g_height = 14;
	    if (g_height < 7)
		g_height = 7;
	    break;

	case 'b':
	    g_bombs = atoi(optarg);
	    break;
	}
    }

    /* Load the high scores from the list */

    bzero(g_scores, sizeof(g_scores));
    load_scores("/tmp/scores", g_scores);

    /* Set the number of bombs */

    if (!g_bombs)
	g_bombs = ((g_width * g_height) / 5);
    else if (g_bombs < 5)
	g_bombs = 5;
    else if (g_bombs > ((g_width * g_height) - 1))
	g_bombs = (g_width * g_height) - 1;

    if (GrOpen() == -1) {
	fprintf(stderr, "Error - Unable to open the graphics engine\n");
	exit(-1);
    }

    g_font = GrCreateFont(GR_FONT_GUI_VAR, 0, 0);

    load_tileset("mine.xpm");

    /* Initalize the field */
    start_game();

    /* Creat the main container window */

    main_wid =
	GrNewWindowEx(WM_PROPS, "Minesweeper", GR_ROOT_WINDOW_ID, 0, 0, 240,
		      300, 0xFFFFFFFF);

    GrSelectEvents(main_wid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_KEY_DOWN |
		   GR_EVENT_MASK_KEY_UP | GR_EVENT_MASK_CLOSE_REQ);

    /* Make the minefield */

    w = (240 - TILE_WIDTH * g_width) / 2;
    h = (300 - TILE_HEIGHT * g_height) / 2;

    minefield_wid = GrNewWindow(main_wid, w, h,
				TILE_WIDTH * g_width,
				TILE_HEIGHT * g_height,
				0, GR_RGB(0xD5, 0xD6, 0xD5), 0);

    GrSelectEvents(minefield_wid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);

    minefield_pixmap = GrNewPixmap(TILE_WIDTH * g_width,
				   TILE_HEIGHT * g_height, 0);

    /* Finally, create the button that we use for game control */
    button_wid = GrNewWindow(main_wid, (240 - BUTTON_WIDTH) / 2,
			     h - (5 + BUTTON_HEIGHT),
			     BUTTON_WIDTH, BUTTON_HEIGHT, 0, 0, 0);

    GrSelectEvents(button_wid,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN |
		   GR_EVENT_MASK_BUTTON_UP);

    GrMapWindow(main_wid);

    GrMapWindow(button_wid);
    GrMapWindow(minefield_wid);

    while (1) {
	GR_EVENT event;

	GrGetNextEventTimeout(&event, 1000L);

	switch (event.type) {
	case GR_EVENT_TYPE_EXPOSURE:
	    if (event.exposure.wid == main_wid)
		draw_main();
	    else if (event.exposure.wid == minefield_wid)
		draw_field();
	    else if (event.exposure.wid == button_wid)
		draw_button(BUTTON_REGULAR);
	    else if (event.exposure.wid == scores_wid)
		draw_scores();
	    //      else
	    //draw_dialog();

	    break;

	case GR_EVENT_TYPE_BUTTON_DOWN:
	    if (event.button.wid == button_wid)
		handle_button(&event.button, BUTTON_WRONG);
	    else if (event.button.wid == minefield_wid)
		handle_minefield_down(&event.button);
	    else if (event.button.wid == scores_wid)
		GrUnmapWindow(scores_wid);
	    else if (event.button.wid == menus[0].wid)
		show_scores();
	    //      else
	    //        dialog_handle_keystroke();

	    break;

	case GR_EVENT_TYPE_BUTTON_UP:
	    if (event.button.wid == button_wid)
		handle_button(&event.button, BUTTON_REGULAR);

	    break;

	case GR_EVENT_TYPE_KEY_DOWN: {
	  int togglekey = MWKEY_UP;

	  /* Use the UP key to toggle the flag on the Ipaq */
#ifdef CONFIG_PLATFORM_IPAQ
	  togglekey = MWKEY_UP;
#endif

	  /* Use the accpet key to toggle the flag on the Zaurus */

#ifdef CONFIG_PLATFORM_ZAURUS
	  togglekey = ' ';
#endif

	  if (event.keystroke.wid == main_wid) {
	    if (event.keystroke.ch == togglekey)	      
	      g_keyflags |= KEY_UP;
	    
	    if (event.keystroke.ch == 'c')
	      handle_cheat();
	    }
	}

	  break;

	case GR_EVENT_TYPE_KEY_UP: {
	  int togglekey = MWKEY_UP;

	  /* Use the UP key to toggle the flag on the Ipaq */
#ifdef CONFIG_PLATFORM_IPAQ
	  togglekey = MWKEY_UP;
#endif

	  /* Use the accpet key to toggle the flag on the Zaurus */

#ifdef CONFIG_PLATFORM_ZAURUS
	  togglekey = ' ';
#endif
	  
	  if (event.keystroke.ch == togglekey)
	    g_keyflags &= ~(KEY_UP);
	}

	  break;

	case GR_EVENT_TYPE_TIMEOUT:
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    exit(0);
	}

	if (game_info.state == GAME_ACTIVE) {
	    int i;
	    int elapsed = time(0) - game_info.start_time;

	    if (((g_width * g_height) - g_bombs) == game_info.tiles_cleared) {
		game_info.state = GAME_OVER;
		draw_button(BUTTON_WON);

		/* Check to see if we have a high score */
		for (i = 0; i < 10; i++) {
		    int t;

		    if (!g_scores[i].seconds || g_scores[i].seconds > elapsed) {

			for (t = 9; t >= (i + 1); t--) {
			    g_scores[t].seconds = g_scores[t - 1].seconds;
			    g_scores[t].date = g_scores[t - 1].date;
			}
			break;
		    }
		}

		if (i < 10) {
		    g_scores[i].seconds = elapsed;
		    g_scores[i].date = time(0);
		    save_scores("/tmp/scores", g_scores);
		}
		//show_name_dialog();
	    } else
		game_info.timer = time(0) - game_info.start_time;
	}

	draw_main();
    }
}

