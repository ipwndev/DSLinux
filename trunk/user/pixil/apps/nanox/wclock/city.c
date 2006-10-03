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
#include <math.h>

#include "city.h"

city_t *
find_city(char *name, city_t * list)
{
    int i;
    char *buffer = alloca(strlen(name) + 1);
    city_t *city = list;

    strcpy(buffer, name);

    for (i = 0; i < strlen(buffer); i++)
	if (buffer[i] == ' ')
	    buffer[i] = 0x1F;

    for (; city; city = city->next)
	if (!strcmp(buffer, city->name))
	    return city;

    return 0;
}

int
load_cities(char *base, char *filename, city_t ** head, city_t ** tail)
{

    char path[128];

    int count = 0;

    city_t *list = 0;
    city_t *city = 0;

    char *ptr, *ch;
    FILE *file = 0;

    snprintf(path, sizeof(path), "%s/%s", base, filename);

    if (!(file = fopen(path, "r")))
	return -1;

    while (!feof(file)) {
	char buffer[80];
	if (!fgets(buffer, sizeof(buffer), file))
	    continue;

	buffer[strlen(buffer) - 1] = 0;
	if (!strlen(buffer) || buffer[0] == '#')
	    continue;

	if (!city)
	    city = list = (city_t *) calloc(1, sizeof(city_t));
	else {
	    city->next = (city_t *) calloc(1, sizeof(city_t));
	    city->next->prev = city;

	    city = city->next;
	}

	count++;

	city->visible = 1;

	ptr = buffer;
	if (!(ch = (char *) strchr(buffer, '^')))
	    continue;

	city->name = (char *) strndup(ptr, ch - ptr);

	ptr = ch + 1;
	city->lat = strtod(ptr, 0);

	if (!(ch = (char *) strchr(ptr, '^')))
	    continue;
	ptr = ch + 1;

	city->lon = strtod(ptr, 0);

	if (!(ch = (char *) strchr(ptr, '^')))
	    continue;
	ptr = ch + 1;

	city->tz = (char *) strdup(ptr);
    }

    *head = list;
    *tail = city;

    return count;
}
