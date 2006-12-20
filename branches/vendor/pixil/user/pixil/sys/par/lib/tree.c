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

#include <par/pardb.h>
#include "database.h"

void
tree_addData(tree_t * node, void *data, ushort size, ushort type)
{

    unsigned short lsize = size;
    if (size == 0)
	return;

    /* Add a byte for the NULL */
    if (type == PAR_TEXT) {
	if (((unsigned char *) data)[size - 1] != 0)
	    lsize = size + 1;
    }

    if (node->size && node->data) {
	if (node->size != lsize) {
	    void *ptr = realloc(node->data, lsize);
	    if (!ptr)
		return;
	    node->data = ptr;
	}
    } else {
	node->data = (void *) calloc(lsize, 1);
    }

    node->size = lsize;
    node->type = type;

    if (node->data) {
	if (type == PAR_TEXT)
	    strncpy((char *) node->data, data, size);
	else
	    memcpy(node->data, data, size);
    }
}

tree_t *
tree_addNode(tree_t * parent, char *keyword)
{

    tree_t *node;

    if (!parent) {
	SET_ERRNO(PARDB_BADNODE);
	return (0);
    }

    if (!parent->child)
	node = parent->child = (tree_t *) calloc(sizeof(tree_t), 1);
    else {
	tree_t *ptr = parent->child;
	while (ptr->peer)
	    ptr = ptr->peer;

	node = ptr->peer = (tree_t *) calloc(sizeof(tree_t), 1);
    }

    if (!node) {
	SET_ERRNO(PARDB_MEMERR);
	return (0);
    }

    node->peer = 0;
    node->child = 0;

    strncpy(node->keyword, keyword, sizeof(node->keyword) - 1);

    SET_ERRNO(PARDB_SUCCESS);
    return (node);
}

tree_t *
tree_findChildNode(tree_t * parent, char *keyword)
{

    tree_t *node;
    if (!parent) {
	SET_ERRNO(PARDB_BADNODE);
	return (0);
    }

    node = parent->child;
    while (node) {
	if (!strcmp(node->keyword, keyword))
	    return (node);
	node = node->peer;
    }

    return (0);
}

int
tree_recursiveBuild(db_handle * db, index_t * top, tree_t * tree)
{

    int ret;
    index_t *node = top;

    /* At this point we are succesful.  */
    SET_ERRNO(PARDB_SUCCESS);

    while (node) {

	ushort type;
	ushort size;

	tree_t *local;
	index_t *child;
	unsigned char *data;

	ret = db_getDataInfo(db, node, &size, 0);
	if (ret == -1 && pardb_errno != PARDB_NODATA)
	    return -1;

	local = tree_addNode(tree, node->keyword);

	if (ret > 0) {
	    if (!(data = alloca(size + 1))) {
		SET_ERRNO(PARDB_MEMERR);
		return (-1);
	    }

	    bzero(data, size + 1);

	    ret = db_getData(db, node, (void *) data, size, &type);
	    if (ret == -1)
		return -1;

	    tree_addData(local, data, ret, type);
	}

	child = db_getChild(db, node);
	if (child)
	    ret = tree_recursiveBuild(db, db_getChild(db, node), local);

	if (ret == -1)
	    return (-1);

	node = db_getPeer(db, node);
    }

    return (0);
}


int
tree_recursiveSave(db_handle * db, tree_t * head, char *name)
{

    tree_t *node = head;

    SET_ERRNO(PARDB_SUCCESS);

    while (node) {

	char *lname;

	if (!name) {
	    lname = alloca(strlen(node->keyword) + 1);
	    strcpy(lname, node->keyword);
	} else {
	    lname = alloca(strlen(name) + strlen(node->keyword) + 2);
	    sprintf(lname, "%s.%s", name, node->keyword);
	}

	if (db_addNode(db, lname, node->data, node->size, node->type) == -1)
	    return (-1);

	if (node->child)
	    if (tree_recursiveSave(db, node->child, lname) == -1)
		return (-1);

	node = node->peer;
    }

    return (0);
}

tree_t *
tree_newTree(void)
{
    tree_t *local = (tree_t *) calloc(sizeof(tree_t), 1);

    if (!local)
	SET_ERRNO(PARDB_MEMERR);
    else
	SET_ERRNO(PARDB_SUCCESS);

    return (local);
}

void
tree_recursiveFree(tree_t * tree)
{

    tree_t *node = tree->child;

    /* Free my children */

    while (node) {
	tree_t *peer = node->peer;
	tree_recursiveFree(node);
	node = peer;
    }

    /* Free my data */
    if (tree->data)
	free(tree->data);

    /* Finally, free this node */
    free(tree);
}
