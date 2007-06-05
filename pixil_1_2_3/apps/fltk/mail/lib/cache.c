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


/*
 * This manages a simple cache that holds the last 
 * few messages that were accessed 
 */

#define CACHE_SIZE 5

#include <stdlib.h>
#include <string.h>
#include "cache.h"

static cache_entry_t cache_entry[5];

int
get_oldest_entry()
{
    int useentry = -1;
    int oldest = -1;
    int age = 0;
    int i;

    for (i = 0; i < CACHE_SIZE; i++) {
	/* If the age is too high, then it must be corrupted */

	if (cache_entry[i].age > 1000) {
	    bzero(&cache_entry[i], sizeof(cache_entry));
	    cache_entry[i].age = -1;
	}

	if (cache_entry[i].age == -1) {
	    if (useentry == -1)
		useentry = i;
	} else {
	    cache_entry[i].age++;

	    if (cache_entry[i].age > age) {
		age = cache_entry[i].age;
		oldest = i;
	    }
	}
    }

    if (useentry != -1)
	return (useentry);
    return (oldest);
}


cache_entry_t *
search_cache_entry(int key)
{
    int i;

    for (i = 0; i < CACHE_SIZE; i++) {
	if (cache_entry[i].key == key)
	    return (&cache_entry[i]);
    }

    return (0);
}

void
free_cache_entry(cache_entry_t * entry)
{
    if (entry->raw)
	free(entry->raw);

    nxmail_free_addrlist(&entry->header.to);

    nxmail_free_addrlist(&entry->header.cc);

    nxmail_free_body_struct(entry->body);

    bzero(entry, sizeof(cache_entry_t));
    entry->age = -1;
}

/* The size is the size of the raw message */

cache_entry_t *
get_new_cache_entry(int key, int size)
{
    /* Grab the oldest entry */
    int entry = get_oldest_entry();

    /* If it was being used, then free it */
    if (cache_entry[entry].age != -1)
	free_cache_entry(&cache_entry[entry]);

    /* Now, allocate some space for the raw message */
    cache_entry[entry].raw = (char *) calloc(size + 1, 1);

    if (!cache_entry[entry].raw)
	return (0);

    cache_entry[entry].age = 0;
    cache_entry[entry].size = size;
    cache_entry[entry].key = key;
    cache_entry[entry].body = 0;

    return (&cache_entry[entry]);
}

void
cache_init(void)
{
    int i;

    for (i = 0; i < CACHE_SIZE; i++) {
	bzero(&cache_entry[i], sizeof(cache_entry_t));
	cache_entry[i].age = -1;
    }
}


void
cache_close(void)
{
    int i;

    for (i = 0; i < CACHE_SIZE; i++)
	free_cache_entry(&cache_entry[i]);

}
