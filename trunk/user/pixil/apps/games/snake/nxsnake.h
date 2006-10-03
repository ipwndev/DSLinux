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

/* NXSNAKE.H
   
   Constants for the nxSnake game 
   Copyright 2001, Jordan Crouse (jordanc@censoft.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef NXSNAKE_H
#define NXSNAKE_H

#define WM_PROPS   (GR_WM_PROPS_MAXIMIZE |\
                    GR_WM_PROPS_BORDER |\
		    GR_WM_PROPS_CAPTION |\
                    GR_WM_PROPS_CLOSEBOX)

/* This is where we define the size of the whole window */
#define WWIDTH  240
#define WHEIGHT 280

/* This is the size of the playground */
/* Minus room for the score and such  */

#define PWIDTH  (WWIDTH)
#define PHEIGHT (WHEIGHT - 20)

/* This is the number of units the screen should be */

#define XUNITS  40
#define YUNITS  40

/* This is the size of the border, the snake, and the nibbles */
#define XUNITSIZE  (PWIDTH / XUNITS)
#define YUNITSIZE  (PHEIGHT / XUNITS)

#define LEVEL_SCORE 10		/* Advance levels every 10 nibbles */

#define TILE_BORDER 0
#define TILE_SNAKE  1
#define TILE_NIBBLE 2

#define TILE_COUNT  3

/* Game states */

#define SNAKE_START        0
#define SNAKE_INSTRUCTIONS 1
#define SNAKE_PLAYING      2
#define SNAKE_PAUSED       3
#define SNAKE_NEXTLEVEL    4
#define SNAKE_DONE         5

#define SNAKE_DIR_UP    0x01
#define SNAKE_DIR_DOWN  0x02
#define SNAKE_DIR_LEFT  0x04
#define SNAKE_DIR_RIGHT 0x08

#define PLAYGROUND_EMPTY  0x01
#define PLAYGROUND_BORDER 0x02
#define PLAYGROUND_SNAKE  0x03
#define PLAYGROUND_TAIL   0x04
#define PLAYGROUND_NIBBLE 0x05

#define GET_SNAKE_DIRECTION(val)      ((val & (0xF << 12)) >> 12)
#define GET_SNAKE_OFFSET(val)         (val & 0xFFF)

typedef struct
{
    unsigned short score;
    unsigned char lives;

    unsigned char speed;	/* The speed of the snake */

    unsigned char headx;	/* The absolute position of the head */
    unsigned char heady;

    unsigned char tailpointer;

    unsigned short length;
    unsigned short growth;

    /* Allow up to 256 girations */
    unsigned short body[256];
}
snake_t;

#endif
