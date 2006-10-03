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


#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <stddef.h>

#define PARDB_MAGIC 0x2323	/* FNORD! */

#undef DEBUG

#ifdef DEBUG
#include <stdio.h>

#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(str, args...)
#endif

#define DB_DATA_BLOCK  0x01
#define DB_INDEX_BLOCK 0x02

/* Contains all the important information about the database */

typedef struct
{
    unsigned short ident;
    short free;
    unsigned long next;
    unsigned long padding[6];
}
index_info_t;

typedef struct
{
    unsigned short ident;
    unsigned short free;
    unsigned long next;
}
data_info_t;


typedef struct
{
    char keyword[PAR_DB_KEYWORD_SIZE + 1];
    unsigned long peer;
    unsigned long child;
    unsigned long data;
}
index_t;

typedef struct
{
    unsigned short type;
    unsigned short size;
    unsigned long index;
    unsigned char value;
}
data_t;

/* This is the length of the "header" on data structure (not including the length) */

/* #define DATA_ITEM_LEN ( sizeof(unsigned short) + sizeof(unsigned short) + sizeof(unsigned long) */
#define DATA_ITEM_LEN ( offsetof(data_t, value) )

typedef struct
{
    unsigned short magic;
    unsigned short blksize;
    unsigned long index;
    unsigned long data;

    unsigned long padding[5];
}
db_info_t;

#ifdef HAVE_MMAPBUG
void *db_getBlockStart(db_handle * db, void *ptr);
unsigned long db_getBlockOffset(db_handle * db, void *ptr);
#endif

#ifdef HAVE_MMAPBUG
#define BLOCK_POINTER_START(db, value) (db_getBlockStart(db, value))
#else
#define BLOCK_POINTER_START(db, value) ((void *) ((ulong) value - ((ulong) value % db->blkSize)))
#endif

#ifdef HAVE_MMAPBUG
#define BLOCK_POINTER_OFFSET(db, value) (db_getBlockOffset(db, value))
#else
#define BLOCK_POINTER_OFFSET(db, value) ((unsigned long) value % db->blkSize)
#endif

#define GET_BLOCK(db, offset) (offset - (offset % db->blkSize))
#define GET_BLOCK_OFFSET(db, offset) (offset % db->blkSize)

/* Utilities */

char *strip_keyword(char **name);

#define db_setMode(db, m)   db->mode = m
#define db_setAccess(db, acc)  db->access = acc

#define db_getAccess(db)       (db->access)

/* database.c */

int db_recursiveAdd(db_handle *, index_t *, char *name, void *data,
		    ushort size, ushort type);
index_t *db_recursiveFind(db_handle *, index_t *, char *name);
int db_recursiveDel(db_handle *, index_t *, char *name);
void db_freeDB(db_handle * db);

db_handle *db_allocDB(int fd, int bsize, int length);


/* index.c */

index_t *db_getFirstIndex(db_handle * db);
index_t *db_getPeer(db_handle * db, index_t * peer);
index_t *db_getChild(db_handle * db, index_t * parent);
index_t *db_addChild(db_handle * db, index_t * parent, char *keyword);
index_t *db_addPeer(db_handle * db, index_t * prev, char *keyword);
index_t *db_addFirstIndex(db_handle * db, char *keyword);

void db_setPeer(db_handle * db, index_t * peer, index_t * index);
void db_setChild(db_handle * db, index_t * node, index_t * index);

void db_formatIndex(db_handle * db, void *block, int offset, int size);
int db_setIndexDataOffset(db_handle * db, int offset, int data);
int db_removeIndex(db_handle * db, index_t * index);

/* data.c */

void db_formatData(db_handle * db, void *block, int offset, int size);
int db_setData(db_handle * db, index_t * index, void *value, ushort size,
	       ushort type);
int db_getData(db_handle * db, index_t * index, void *value, ushort size,
	       ushort * type);
int db_removeData(db_handle * db, int offset);

int db_getDataInfo(db_handle * db, index_t * index, ushort * size,
		   ushort * type);

/* io.c */

int db_addBlock(db_handle * db);
void *db_mapBlock(db_handle * db, int offset);
void db_unmapBlock(db_handle * db, int block);
unsigned long db_getFileOffset(db_handle * db, void *ptr);

void db_flushWrite(db_handle * db);
void db_lockDB(db_handle * db);
void db_unlockDB(db_handle * db);

/* tree.c */

tree_t *tree_newTree(void);
void tree_freeTree(tree_t * tree);

int tree_recursiveBuild(db_handle * db, index_t * top, tree_t * tree);
int tree_recursiveSave(db_handle * db, tree_t * head, char *name);
void tree_recursiveFree(tree_t * tree);

tree_t *tree_addNode(tree_t * parent, char *keyword);
void tree_addData(tree_t * node, void *data, ushort size, ushort type);

tree_t *tree_findChildNode(tree_t * parent, char *keyword);

#endif
