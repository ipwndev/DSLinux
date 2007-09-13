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
#include <math.h>
#include "vertex.h"
#define MWINCLULDECOLORS
#include <nano-X.h>
#include <wm/nxlib.h>

static struct matrix m = { 1, 0, 0, 1, 0, 0 };
static struct matrix stack[10];
static int sptr = 0;
static GR_POINT *p = (GR_POINT *) 0;
static int gap;
static int p_size;
static int n;
static int what;

enum
{ LINE, LOOP, POLYGON, POINT_ };

void
push_matrix()
{
    stack[sptr++] = m;
}

void
pop_matrix()
{
    m = stack[--sptr];
}

void
mult_matrix(float a, float b, float c, float d, float x, float y)
{
    struct matrix o;
    o.a = a * m.a + b * m.c;
    o.b = a * m.b + b * m.d;
    o.c = c * m.a + d * m.c;
    o.d = c * m.b + d * m.d;
    o.x = x * m.a + y * m.c + m.x;
    o.y = x * m.b + y * m.d + m.y;
    m = o;
}

void
scale_xy(float x, float y)
{
    mult_matrix(x, 0, 0, y, 0, 0);
}

void
scale_x(float x)
{
    mult_matrix(x, 0, 0, x, 0, 0);
}

void
translate(float x, float y)
{
    mult_matrix(1, 0, 0, 1, x, y);
}

void
rotate(float d)
{
    if (d) {
	float s, c;
	if (0 == d) {
	    s = 0;
	    c = 1;
	} else if (90 == d) {
	    s = 1;
	    c = 0;
	} else if (180 == d) {
	    s = 0;
	    c = -1;
	} else if (270 == d || -90 == d) {
	    s = -1;
	    c = 0;
	} else {
	    s = sin(d * M_PI / 180);
	    c = cos(d * M_PI / 180);
	}
	mult_matrix(c, -s, s, c, 0, 0);
    }
}

void
begin_points()
{
    n = 0;
    what = POINT_;
}

void
begin_line()
{
    n = 0;
    what = LINE;
}

void
begin_loop()
{
    n = 0;
    what = LOOP;
}

void
begin_polygon()
{
    n = 0;
    what = POLYGON;
}

float
transform_x(float x, float y)
{
    return x * m.a + y * m.c + m.x;
}

float
transform_y(float x, float y)
{
    return x * m.b + y * m.d + m.y;
}

float
transform_dx(float x, float y)
{
    return x * m.a + y * m.c;
}

float
transform_dy(float x, float y)
{
    return x * m.b + y * m.d;
}

static void
transformed_coord_vertex(COORD_T x, COORD_T y)
{
    if (!n || x != p[n - 1].x || y != p[n - 1].y) {
	if (n >= p_size) {
	    p_size = p ? 2 * p_size : 16;
	    p = (GR_POINT *) realloc((void *) p, p_size * sizeof(*p));
	}
	p[n].x = x;
	p[n].y = y;
	n++;
    }
}

void
transformed_double_vertex(float xf, float yf)
{
    transformed_coord_vertex((COORD_T) xf + .5, (COORD_T) yf + .5);
}

void
vertex(float x, float y)
{
    transformed_double_vertex(x * m.a + y * m.c + m.x,
			      x * m.b + y * m.d + m.y);
}

void
end_points(GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    if (n > 1)
	GrPoly(pmap, gc, n, p);
}

void
end_line(GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    if (n > 1)
	GrPoly(pmap, gc, n, p);
}

static void
fixloop()
{
    while (n > 2 && p[n - 1].x == p[0].x && p[n - 1].y == p[0].y)
	n--;
}

void
end_loop(GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    fixloop();
    if (n > 2)
	transformed_coord_vertex((COORD_T) p[0].x, (COORD_T) p[0].y);
    end_line(pmap, gc);
}

void
end_polygon(GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    fixloop();
    if (n > 2)
	GrFillPoly(pmap, gc, n, p);
}


void
begin_complex_polygon()
{
    begin_polygon();
    gap = 0;
}

void
fgap()
{
    while (n > gap + 2 && p[n - 1].x == p[gap].x && p[n - 1].y == p[gap].y)
	n--;
    if (n > gap + 2) {
	transformed_coord_vertex((COORD_T) p[gap].x, (COORD_T) p[gap].y);
	gap = n;
    } else {
	n = gap;
    }
}

void
end_complex_polygon(GR_WINDOW_ID pmap, GR_GC_ID gc)
{
    fgap();
    if (n > 2)
	GrFillPoly(pmap, gc, n, p);
}

void
circle(float x, float y, float r, GR_WINDOW_ID pmap, GR_GC_ID gc,
       GR_SIZE w, GR_SIZE h)
{
    GrEllipse(pmap, gc, w / 2 - 1, h / 2 - 1, w / 2 - 1, h / 2 - 1);
}
