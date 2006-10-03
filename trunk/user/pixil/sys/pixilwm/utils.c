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


/* These are functions that were previously in nxlib that */
/* were pulled over here to help with size considerations */

#include <stdio.h>
#include <stdlib.h>
#include <nano-X.h>
#include <nxdraw.h>
#include "utils.h"

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

/******** List Functions ************/

void *
nxItemAlloc(unsigned int size)
{
    return (void *) calloc(size, 1);
}

/* insert at tail of list*/
void
nxListAdd(PNXLISTHEAD pHead, PNXLIST pItem)
{
    if (pHead->tail) {
	pItem->prev = pHead->tail;
	pHead->tail->next = pItem;
    } else
	pItem->prev = NULL;
    pItem->next = NULL;
    pHead->tail = pItem;
    if (!pHead->head)
	pHead->head = pItem;
}

/* insert at head of list*/
void
nxListInsert(PNXLISTHEAD pHead, PNXLIST pItem)
{
    if (pHead->head) {
	pItem->next = pHead->head;
	pHead->head->prev = pItem;
    } else
	pItem->next = NULL;
    pItem->prev = NULL;
    pHead->head = pItem;
    if (!pHead->head)
	pHead->head = pItem;
}

void
nxListRemove(PNXLISTHEAD pHead, PNXLIST pItem)
{
    if (pItem->next)
	pItem->next->prev = pItem->prev;
    if (pItem->prev)
	pItem->prev->next = pItem->next;
    if (pHead->head == pItem)
	pHead->head = pItem->next;
    if (pHead->tail == pItem)
	pHead->tail = pItem->prev;
    pItem->next = pItem->prev = NULL;
}
