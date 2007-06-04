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
#include "nano-X.h"
#include "nxdraw.h"
#include <wm/nxlib.h>

/*
Copyright 1989, 1998  The Open Group
Copyright 1985, 1986, 1987,1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/

/* 
 * Bitmask returned by ParseGeometry().  Each bit tells if the corresponding
 * value (x, y, width, height) was found in the parsed string.
 */
#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020

/*
 *   ParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */
static int
ReadInteger(char *string, char **NextString)
{
    int Result = 0;
    int Sign = 1;

    if (*string == '+')
	string++;
    else if (*string == '-') {
	string++;
	Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
	Result = (Result * 10) + (*string - '0');
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    return (-Result);
}

static int
ParseGeometry(char *string, int *x, int *y, unsigned int *width,
	      unsigned int *height)
{
    int mask = NoValue;
    register char *strind;
    unsigned int tempWidth = 0, tempHeight = 0;
    int tempX = 0, tempY = 0;
    char *nextCharacter;

    if ((string == NULL) || (*string == '\0'))
	return (mask);
    if (*string == '=')
	string++;		/* ignore possible '=' at beg of geometry spec */

    strind = (char *) string;
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
	tempWidth = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	    return (0);
	strind = nextCharacter;
	mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {
	strind++;
	tempHeight = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	    return (0);
	strind = nextCharacter;
	mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
	if (*strind == '-') {
	    strind++;
	    tempX = -ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return (0);
	    strind = nextCharacter;
	    mask |= XNegative;

	} else {
	    strind++;
	    tempX = ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return (0);
	    strind = nextCharacter;
	}
	mask |= XValue;
	if ((*strind == '+') || (*strind == '-')) {
	    if (*strind == '-') {
		strind++;
		tempY = -ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= YNegative;

	    } else {
		strind++;
		tempY = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
	    }
	    mask |= YValue;
	}
    }

    /* If strind isn't at the end of the string the it's an invalid
       geometry specification. */

    if (*strind != '\0')
	return (0);

    if (mask & XValue)
	*x = tempX;
    if (mask & YValue)
	*y = tempY;
    if (mask & WidthValue)
	*width = tempWidth;
    if (mask & HeightValue)
	*height = tempHeight;
    return (mask);
}

/*
 * This routine given a user supplied positional argument and a default
 * argument (fully qualified) will return the position the window should take.
 * Always sets all return values and returns a mask describing which fields
 * were set by the user or'ed with whether or not the x and y values should
 * be considered "negative".
 */
int
nxGetGeometry(GR_CHAR * user_geom,	/* user provided geometry spec */
	      GR_CHAR * def_geom,	/* default geometry spec for window */
	      GR_WM_PROPS style,	/* window style */
	      GR_COORD * x_return,	/* location of window */
	      GR_COORD * y_return,	/* location of window */
	      GR_SIZE * width_return,	/* size of window */
	      GR_SIZE * height_return)
{				/* size of window */
    int ux, uy;			/* returned values from parse */
    unsigned int uwidth, uheight;	/* returned values from parse */
    int umask;			/* parse mask of returned values */
    int dx, dy;			/* default values from parse */
    unsigned int dwidth, dheight;	/* default values from parse */
    int dmask;			/* parse mask of returned values */
    int min_width, min_height;	/* valid amounts */
    int rx, ry, rwidth, rheight;	/* return values */
    int rmask;			/* return mask */
    int cxborder = 0, cyborder = 0;
    GR_SCREEN_INFO si;
#define SCREENWIDTH	si.ws_width	/* useable workspace width */
#define SCREENHEIGHT	si.ws_height	/* useable workspace height */

    GrGetScreenInfo(&si);

    /*
     * Calc correct border sizes
     */
    // FIXME put in nxCalcBorderSizes
    if ((style & GR_WM_PROPS_APPMASK) == GR_WM_PROPS_APPWINDOW)
	style = (style & ~GR_WM_PROPS_APPMASK) | nxGetDefaultWindowStyle();
    if (style & GR_WM_PROPS_BORDER) {
	cxborder = 1 * 2;
	cyborder = 1 * 2;
    }
    /* appframe overrides border */
    if (style & GR_WM_PROPS_APPFRAME) {
	cxborder = 0 * 2;
	cyborder = 0 * 2;
    }
    if (style & GR_WM_PROPS_CAPTION) {
	cyborder += 16;
    }

    /*
     * parse the two geometry masks
     */
    rmask = umask = ParseGeometry(user_geom, &ux, &uy, &uwidth, &uheight);
    dmask = ParseGeometry(def_geom, &dx, &dy, &dwidth, &dheight);

    /*
     * get the width and height:
     *     1.  if user-specified, then take that value
     *     2.  else, if program-specified, then take that value
     *     3.  else, take 1
     */
    rwidth = ((umask & WidthValue) ? uwidth :
	      ((dmask & WidthValue) ? dwidth : 1));
    rheight = ((umask & HeightValue) ? uheight :
	       ((dmask & HeightValue) ? dheight : 1));

    /*
     * Make sure computed size is within limits.  Note that we always do the
     * lower bounds check since the base size (which defaults to 0) should
     * be used if a minimum size isn't specified.
     */
    min_width = 0;
    min_height = 0;
    if (rwidth < min_width)
	rwidth = min_width;
    if (rheight < min_height)
	rheight = min_height;

    /*
     * Compute the location.  Set the negative flags in the return mask
     * (and watch out for borders), if necessary.
     */
    if (umask & XValue) {
	rx = ((umask & XNegative) ?
	      (SCREENWIDTH + ux - rwidth - cxborder) : ux);
    } else if (dmask & XValue) {
	if (dmask & XNegative) {
	    rx = (SCREENWIDTH + dx - rwidth - cxborder);
	    rmask |= XNegative;
	} else
	    rx = dx;
    } else {
	rx = 0;			/* gotta choose something... */
    }

    if (umask & YValue) {
	ry = ((umask & YNegative) ?
	      (SCREENHEIGHT + uy - rheight - cyborder) : uy);
    } else if (dmask & YValue) {
	if (dmask & YNegative) {
	    ry = (SCREENHEIGHT + dy - rheight - cyborder);
	    rmask |= YNegative;
	} else
	    ry = dy;
    } else {
	ry = 0;			/* gotta choose something... */
    }

    /*
     * All finished, so set the return variables.
     */
    *x_return = rx;
    *y_return = ry;
    *width_return = rwidth;
    *height_return = rheight;
    return rmask;
}

#if TEST
int
main(int ac, char **av)
{
    char *geom = NULL;
    int x, y, w, h;

    if (ac > 1)
	geom = av[1];
    nxGetGeometry(geom, "320x240+10+10", 0, &x, &y, &w, &h);
    printf("%d,%d,%d,%d\n", x, y, w, h);
}
#endif
