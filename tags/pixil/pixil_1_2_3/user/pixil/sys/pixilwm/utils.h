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


#ifndef _UTILS_H_
#define _UTILS_H_

/* dbl linked list data structure*/

typedef struct _nxlist
{				/* LIST must be first decl in struct */
    struct _nxlist *next;	/* next item */
    struct _nxlist *prev;	/* previous item */
}
NXLIST, *PNXLIST;

/* dbl linked list head data structure*/

typedef struct _nxlisthead
{
    struct _nxlist *head;	/* first item */
    struct _nxlist *tail;	/* last item */
}
NXLISTHEAD, *PNXLISTHEAD;

void *nxItemAlloc(unsigned int size);
void nxListAdd(PNXLISTHEAD pHead, PNXLIST pItem);
void nxListInsert(PNXLISTHEAD pHead, PNXLIST pItem);
void nxListInsertAfter(PNXLISTHEAD pHead, PNXLIST pItem, PNXLIST pPrev);


void nxListRemove(PNXLISTHEAD pHead, PNXLIST pItem);

#define nxItemNew(type)	((type *)nxItemAlloc(sizeof(type)))
#define nxItemFree(ptr)	free((void *)ptr)

/* field offset*/
#define NXITEM_OFFSET(type, field)    ((long)&(((type *)0)->field))

/* return base item address from list ptr*/
#define nxItemAddr(p,type,list)	((type *)((long)p - NXITEM_OFFSET(type,list)))

/* calculate container size from client style/size*/
void nxCalcNCSize(GR_WM_PROPS style, GR_SIZE wClient,
		  GR_SIZE hClient, GR_COORD * xCliOffset,
		  GR_COORD * yCliOffset, GR_SIZE * wContainer,
		  GR_SIZE * hContainer);

/* calculate client size from container style/size*/
void nxCalcClientSize(GR_WM_PROPS style, GR_SIZE wContainer,
		      GR_SIZE hContainer, GR_COORD * xCliOffset,
		      GR_COORD * yCliOffset, GR_SIZE * wClient,
		      GR_SIZE * hClient);

/* utility routines*/
void strzcpy(char *dst, char *src, int count);

#endif
