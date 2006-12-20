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


#ifndef _THEMES_H_
#define _THEMES_H_


/* A macro to calculate the correct widget number */

#define COMPONENT_WIDGET(comp, index)  ((comp & 0xFF) << 8 | (index & 0xFF))
#define GET_COMPONENT(item)            ((item >> 8) & 0xFF)
#define GET_WIDGET(item)               (item & 0xFF)

/* The available components */

#define COMPONENT_TITLEBAR    0
#define COMPONENT_LEFT        1
#define COMPONENT_RIGHT       2
#define COMPONENT_BOTTOM      3
#define COMPONENT_TOP         4

#define THEME_COMPONENT_COUNT 5

/* Titlebar widgets */

#define TITLEBAR_WIDGET_CAPTION     COMPONENT_WIDGET(COMPONENT_TITLEBAR, 0)
#define TITLEBAR_WIDGET_CLOSEBUTTON COMPONENT_WIDGET(COMPONENT_TITLEBAR, 1)
#define TITLEBAR_WIDGET_TITLEBAR    COMPONENT_WIDGET(COMPONENT_TITLEBAR, 2)

#define TITLEBAR_WIDGET_COUNT 3
#define BORDER_WIDGET_COUNT   1

#define STATE_ACTIVE     0
#define STATE_INACTIVE   1

typedef struct
{
    int flags;

    char *image;
    GR_COLOR fgcolor;
    GR_COLOR bgcolor;

    GR_WINDOW_ID imageid;

    int imageWidth;
    int imageHeight;
}
widget_state_t;

typedef struct
{

    struct
    {
	int x;
	int y;
	int w;
	int h;
    }
    hotspot;

    struct
    {
	int min;
	int max;
    }
    width;

    struct
    {
	int min;
	int max;
    }
    height;

    widget_state_t states[2];
}
widget_t;

typedef struct
{
    int type;
    int widgetCount;
    widget_t **widgets;
}
component_t;

typedef struct
{

    /* A component is some part of the screen that is made up of different images */
    component_t components[THEME_COMPONENT_COUNT];

    /* This defines how the client is offset from the container */

    struct
    {
	int left;
	int right;
	int top;
	int bottom;
    }
    client;

    GR_FONT_ID font;
}
theme_t;

theme_t *createTheme(const char *directory);

void set_activeTheme(theme_t * theme);
theme_t *get_activeTheme(void);

void themeDrawContainer(GR_DRAW_ID, int, int, GR_CHAR *, GR_BOOL, GR_WM_PROPS,
			theme_t *);
void themeContainerSize(theme_t *, GR_WM_PROPS, int, int, int *, int *, int *,
			int *);
void themeClientSize(theme_t *, GR_WM_PROPS style, int, int, int *clientw,
		     int *);

int widgetCheckBounds(theme_t * theme, GR_WM_PROPS style, int index, int x,
		      int y, int w, int h);
int themeCheckBounds(theme_t * theme, GR_WM_PROPS style, int index, int x,
		     int y, int w, int h);


#endif
