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


#ifndef vertex_h
#define vertex_h
#endif

#include "nano-X.h"

typedef short COORD_T;

struct matrix
{
    double a, b, c, d, x, y;
};

void push_matrix();
void pop_matrix();
void mult_matrix(double a, double b, double c, double d, double x, double y);
void scale_xy(double x, double y);
void scale_x(double x);
void translate(double x, double y);
void rotate(double d);
void begin_points();
void begin_line();
void begin_loop();
void begin_polygon();
double transform_x(double x, double y);
double transform_y(double x, double y);
double transform_dx(double x, double y);
double transform_dy(double x, double y);
void transformed_double_vertex(double xf, double yf);
void vertex(double x, double y);
void end_points(GR_WINDOW_ID pmap, GR_GC_ID gc);
void end_line(GR_WINDOW_ID pmap, GR_GC_ID gc);
void end_loop(GR_WINDOW_ID pmap, GR_GC_ID gc);
void end_polygon(GR_WINDOW_ID pmap, GR_GC_ID gc);
void begin_complex_polygon();
void fgap();
void end_complex_polygon(GR_WINDOW_ID pmap, GR_GC_ID gc);
void circle(double x, double y, double r, GR_WINDOW_ID pmap, GR_GC_ID gc,
	    GR_SIZE w, GR_SIZE h);
