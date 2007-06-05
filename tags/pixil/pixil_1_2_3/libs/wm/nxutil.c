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
#include <stdlib.h>
#include <string.h>

#define MWINCLUDECOLORS
#include <nano-X.h>
#include "nxdraw.h"
#include <wm/nxlib.h>

/*
nxterm -title nxterm -geom WxH+X+Y
nxclock -0+0
nxkbd -geom +0-0 -background 1 -style 44
nxscribble -geom -0-0
nxkbd -nomove -nofocus -noraise -appwindow -appframe -border -caption

nxARGS args[] = {
	nxTITLE(DEF_TITLE),
	nxGEOMETRY(DEF_GEOMETRY),
	nxBACKGROUND(GR_COLOR_WINDOW),
	nxSTYLE(GR_WM_PROPS_NOAUTOMOVE|GR_WM_PROPS_BORDER|GR_WM_PROPS_CAPTION|GR_WM_PROPS_CLOSEBOX),
	nxEND
};
	wid = nxCreateAppWindow(&ac, &av, args);
*/

/*
 * Initialize application from nxARGS array and
 * passed user argument list.
 * Create new window with calculated style, geometry,
 * color and background.
 */
GR_WINDOW_ID
nxCreateAppWindow(int *aac, char ***aav, nxARGS * alist)
{
    int ac = *aac - 1;
    char **av = *aav + 1;
    nxARGS *pargs;
    int i;
    GR_COORD x, y;
    GR_SIZE w, h;
    char *title = NULL;
    char *defgeom = "100x100+100+150";
    char *usergeom = NULL;
    GR_COLOR background = GR_COLOR_APPWINDOW;
    GR_WM_PROPS style = GR_WM_PROPS_APPWINDOW;

    /* first traverse initialization list */
    for (pargs = alist; pargs->type; ++pargs) {
	switch (pargs->type & 0xff) {
	case nxtypeTITLE:
	    title = pargs->defvalue;
	    break;
	case nxtypeGEOMETRY:
	    defgeom = pargs->defvalue;
	    break;
	case nxtypeBACKGROUND:
	    background = (GR_COLOR) pargs->defvalue;
	    break;
	case nxtypeSTYLE:
	    style = (GR_WM_PROPS) pargs->defvalue;
	    break;
	}
    }

    /* then traverse passed user argument list */
    for (i = 0; i < ac; ++i) {
	for (pargs = alist; pargs->type; ++pargs) {
	    if (strcmp(av[i], pargs->optname) == 0) {
		if (i == ac - 1) {
		    printf("nxArgs: %s missing option\n", pargs->optname);
		    break;
		}
		++i;
		switch (pargs->type & 0xff) {
		case nxtypeTITLE:
		    title = av[i];
		    break;
		case nxtypeGEOMETRY:
		    usergeom = av[i];
		    break;
		case nxtypeBACKGROUND:
		    background = atoi(av[i]);
		    break;
		case nxtypeSTYLE:
		    style = atoi(av[i]);
		    break;
		}
		break;
	    }
	}
    }

    /* return ptr to first unused arg */
    av += ac;
    *aac = ac;
    *aav = av;

    /* parse geometry */
    nxGetGeometry(usergeom, defgeom, style, &x, &y, &w, &h);

    /* convert color */
    background = GrGetSysColor(background);

    /* create window and specify properties and title */
    return GrNewWindowEx(style, title, GR_ROOT_WINDOW_ID, x, y, w, h,
			 background);
}

/* return default window decoration style (for GR_WM_PROPS_APPWINDOW)*/
GR_WM_PROPS
nxGetDefaultWindowStyle(void)
{
    static GR_SCREEN_INFO si;

    if (!si.rows)
	GrGetScreenInfo(&si);
    if (si.ws_width < 400)
	return STYLE_PDA;
    return STYLE_WEBPAD;
}

/*
 * Calculate container size and client window offsets from
 * passed client window size and style.
 */

void
nxCalcNCSize(GR_WM_PROPS style, GR_SIZE wClient, GR_SIZE hClient,
	     GR_COORD * xCliOffset, GR_COORD * yCliOffset,
	     GR_SIZE * wContainer, GR_SIZE * hContainer)
{
    GR_SIZE width, height;
    GR_SIZE xoffset, yoffset;

    /* determine container size and client child window offsets */
    if (style & GR_WM_PROPS_APPFRAME) {
	width = wClient + CXFRAME;
	height = hClient + CYFRAME;
	xoffset = CXBORDER;
	yoffset = CYBORDER;
    } else if (style & GR_WM_PROPS_BORDER) {
	width = wClient + 2;
	height = hClient + 2;
	xoffset = 1;
	yoffset = 1;
    } else {
	width = wClient;
	height = hClient;
	xoffset = 0;
	yoffset = 0;
    }
    if (style & GR_WM_PROPS_CAPTION) {
	height += CYCAPTION;
	yoffset += CYCAPTION;
	if (style & GR_WM_PROPS_APPFRAME) {
	    /* extra line under caption with appframe */
	    ++height;
	    ++yoffset;
	}
    }

    if (xCliOffset)
	*xCliOffset = xoffset;
    if (yCliOffset)
	*yCliOffset = yoffset;
    if (wContainer)
	*wContainer = width;
    if (hContainer)
	*hContainer = height;
}



/*
 * Calculate client size window offsets from
 * passed container window size and style.
 */
void
nxCalcClientSize(GR_WM_PROPS style, GR_SIZE wContainer, GR_SIZE hContainer,
		 GR_COORD * xCliOffset, GR_COORD * yCliOffset,
		 GR_SIZE * wClient, GR_SIZE * hClient)
{
    GR_SIZE width, height;
    GR_SIZE xoffset, yoffset;

    /* determine client size and window offsets */
    if (style & GR_WM_PROPS_APPFRAME) {
	width = wContainer - CXFRAME;
	height = hContainer - CYFRAME;
	xoffset = CXBORDER;
	yoffset = CYBORDER;
    } else if (style & GR_WM_PROPS_BORDER) {
	width = wContainer - 2;
	height = hContainer - 2;
	xoffset = 1;
	yoffset = 1;
    } else {
	width = wContainer;
	height = hContainer;
	xoffset = 0;
	yoffset = 0;
    }
    if (style & GR_WM_PROPS_CAPTION) {
	height -= CYCAPTION;
	yoffset += CYCAPTION;
	if (style & GR_WM_PROPS_APPFRAME) {
	    /* extra line under caption with appframe */
	    --height;
	    ++yoffset;
	}
    }

    if (xCliOffset)
	*xCliOffset = xoffset;
    if (yCliOffset)
	*yCliOffset = yoffset;
    if (wClient)
	*wClient = width;
    if (hClient)
	*hClient = height;
}
