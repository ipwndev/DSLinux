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


#include <stdio.h>

#define MWINCLUDECOLORS
#include "nano-X.h"
#include "nxsnake.h"

unsigned char playground[YUNITS][XUNITS];

GR_WINDOW_ID mainwindow, swindow;
GR_WINDOW_ID gridb, saveb;

int dogrid = 1;
int button_down = 0;
int mouse_moved = 0;

void
set_position(GR_EVENT_MOUSE * event)
{
    int tw, th, tb;
    char buffer[100];

    int xpos = event->x / XUNITSIZE;
    int ypos = event->y / YUNITSIZE;

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    GrSetGCFont(gc, font);
    GrSetGCBackground(gc, BLACK);
    GrSetGCForeground(gc, WHITE);

    sprintf(buffer, "X: %2.2d Y %2.2d", xpos, ypos);

    GrGetGCTextSize(gc, buffer, -1, GR_TFTOP, &tw, &th, &tb);

    GrText(mainwindow, gc, XUNITS * XUNITSIZE - 10 - tw, YUNITS * YUNITSIZE,
	   buffer, -1, GR_TFTOP);

    GrDestroyFont(font);
    GrDestroyGC(gc);
}

void
draw_grid_button(void)
{
    int tw, th, tb;

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    GrSetGCForeground(gc, LTGRAY);
    GrFillRect(gridb, gc, 0, 0, 30, 20);

    GrSetGCFont(gc, font);
    GrSetGCBackground(gc, LTGRAY);
    GrSetGCForeground(gc, BLACK);

    GrGetGCTextSize(gc, "Grid", -1, GR_TFTOP, &tw, &th, &tb);

    GrText(gridb, gc, 15 - (tw / 2), 0, "Grid", -1, GR_TFTOP);

    GrDestroyFont(font);
    GrDestroyGC(gc);
}

void
draw_save_button(void)
{
    int tw, th, tb;

    GR_GC_ID gc = GrNewGC();
    GR_FONT_ID font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    GrSetGCForeground(gc, LTGRAY);
    GrFillRect(saveb, gc, 0, 0, 30, 20);

    GrSetGCFont(gc, font);
    GrSetGCBackground(gc, LTGRAY);
    GrSetGCForeground(gc, BLACK);

    GrGetGCTextSize(gc, "Save", -1, GR_TFTOP, &tw, &th, &tb);

    GrText(saveb, gc, 15 - (tw / 2), 0, "Save", -1, GR_TFTOP);

    GrDestroyFont(font);
    GrDestroyGC(gc);
}

void
redraw_screen(void)
{
    int y, x;
    GR_GC_ID gc = GrNewGC();

    for (y = 0; y < YUNITS; y++)
	for (x = 0; x < XUNITS; x++) {
	    if (playground[y][x]) {
		GrSetGCForeground(gc, BLUE);
		GrFillRect(swindow, gc, (x * XUNITSIZE), (y * YUNITSIZE),
			   XUNITSIZE, YUNITSIZE);
	    } else {
		GrSetGCForeground(gc, BLACK);
		GrFillRect(swindow, gc, (x * XUNITSIZE), (y * YUNITSIZE),
			   XUNITSIZE, YUNITSIZE);
	    }

	    if (dogrid) {
		GrSetGCForeground(gc, LTGRAY);

		GrRect(swindow, gc, (x * XUNITSIZE), (y * YUNITSIZE),
		       XUNITSIZE, YUNITSIZE);
	    }

	}

    GrDestroyGC(gc);
}

void
draw_point(int xpos, int ypos, int set)
{
    GR_GC_ID gc = GrNewGC();

    if (set) {
	GrSetGCForeground(gc, BLUE);
	playground[ypos][xpos] = 1;
    } else {
	GrSetGCForeground(gc, BLACK);
	playground[ypos][xpos] = 0;
    }

    GrFillRect(swindow, gc, (xpos * XUNITSIZE), (ypos * YUNITSIZE),
	       XUNITSIZE, YUNITSIZE);

    if (dogrid) {
	GrSetGCForeground(gc, LTGRAY);
	GrRect(swindow, gc, (xpos * XUNITSIZE), (ypos * YUNITSIZE),
	       XUNITSIZE, YUNITSIZE);
    }

    GrDestroyGC(gc);
}

void
handle_buttonup(GR_EVENT_BUTTON * event)
{
    int xpos = event->x / XUNITSIZE;
    int ypos = event->y / YUNITSIZE;

    if (event->changebuttons & GR_BUTTON_L)
	draw_point(xpos, ypos, 1);
    if (event->changebuttons & GR_BUTTON_R)
	draw_point(xpos, ypos, 0);
}

void
handle_motion(GR_EVENT_MOUSE * event)
{
    int xpos = event->x / XUNITSIZE;
    int ypos = event->y / YUNITSIZE;

    if (event->buttons & GR_BUTTON_L)
	draw_point(xpos, ypos, 1);
    if (event->buttons & GR_BUTTON_R)
	draw_point(xpos, ypos, 0);
}

void
toggle_grid(void)
{
    if (dogrid)
	dogrid = 0;
    else
	dogrid = 1;

    redraw_screen();

}

void
do_save(char *filename)
{
    int x, y;

    FILE *output;

    /* Save the current border to a file */
    output = fopen(filename, "w");
    if (!output)
	return;

    fprintf(output, "/* NXSnake level editor version .0001 */\n");
    fprintf(output, "/* Copyright Century Software.  Bob is your uncle */\n");
    fprintf(output, "\n");

    /* Now output the image */
    fprintf(output, "unsigned char level[%d] = {\n", XUNITS * YUNITS);

    for (y = 0; y < YUNITS; y++) {
	for (x = 0; x < XUNITS; x++) {
	    if (playground[y][x])
		fprintf(output, "%d", PLAYGROUND_BORDER);
	    else
		fprintf(output, "%d", PLAYGROUND_EMPTY);

	    if (((x + 1) * (y + 1)) != XUNITS * YUNITS)
		fprintf(output, ",");
	}
	fprintf(output, "\n");
    }

    fprintf(output, "};\n");

    fclose(output);
}

int
main(int argc, char **argv)
{
    bzero(&playground, XUNITS * YUNITS);

    if (GrOpen() < 0)
	exit(-1);

    /* Make the window */

    mainwindow = GrNewWindowEx(WM_PROPS, "nxsnake", GR_ROOT_WINDOW_ID,
			       10, 10, XUNITS * XUNITSIZE,
			       YUNITS * YUNITSIZE + 20, BLACK);

    swindow = GrNewWindow(mainwindow,
			  0, 0, XUNITS * XUNITSIZE,
			  YUNITS * YUNITSIZE, 0, BLACK, BLACK);

    gridb = GrNewWindow(mainwindow,
			0, YUNITS * YUNITSIZE, 30, 20, 0, BLACK, BLACK);

    saveb = GrNewWindow(mainwindow,
			50, YUNITS * YUNITSIZE, 30, 20, 0, BLACK, BLACK);


    GrSelectEvents(swindow, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_CLOSE_REQ |
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP |
		   GR_EVENT_MASK_MOUSE_POSITION);

    GrSelectEvents(gridb, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_CLOSE_REQ |
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);

    GrSelectEvents(saveb, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_CLOSE_REQ |
		   GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP);

    GrMapWindow(mainwindow);
    GrMapWindow(swindow);
    GrMapWindow(gridb);
    GrMapWindow(saveb);

    while (1) {
	GR_EVENT event;

	/* Every 250 miliseconds, redraw the screen */

	GrGetNextEvent(&event);

	switch (event.type) {
	case GR_EVENT_TYPE_EXPOSURE:

	    if (event.exposure.wid == swindow)
		redraw_screen();
	    else if (event.exposure.wid == gridb)
		draw_grid_button();
	    else if (event.exposure.wid == saveb)
		draw_save_button();

	    break;

	case GR_EVENT_TYPE_BUTTON_DOWN:
	    button_down = 1;
	    break;

	case GR_EVENT_TYPE_BUTTON_UP:
	    if (event.button.wid == swindow)
		handle_buttonup(&event.button);

	    else if (event.button.wid == gridb)
		toggle_grid();

	    else if (event.button.wid == saveb)
		do_save("test.h");

	    button_down = 0;
	    break;

	case GR_EVENT_TYPE_MOUSE_POSITION:

	    set_position(&event.mouse);

	    if (button_down) {
		mouse_moved = 1;
		handle_motion(&event.mouse);
	    }

	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);
	}
    }
}
