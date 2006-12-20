/*
 * Copyright (C) 200-2002 Century Embedded Techonlogies
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include "objects.h"
#include "NXMCursor.h"

void
NXMCursor::setCursor(GR_WINDOW_ID window)
{
    GrSetCursor(window, width, height, hx, hy, BLACK, WHITE, cursor, cursor);
}

static int
load_file(char *filename, GR_BITMAP ** bitmap, int *x, int *y)
{
    char *c;

    int width, height, bm;
    FILE *infile;
    static char *dir =
	strdup(access(xbill_path "pixmaps/logo.xpm", R_OK) ? "" : xbill_path);
    char buf[BUFSIZ];
    char tempa[40];
    char tempb[40];
    char file[255];
    GR_BITMAP *cursorptr, *cursor;

    width = 0;
    height = 0;

    sprintf(file, "%sbitmaps/%s.xbm", dir, filename);

    /* Ok, lets do this.  We have two defines that indicate the width and the height.  Grab em! */
    infile = fopen(file, "r");

    fgets(buf, BUFSIZ, infile);

    buf[strlen(buf)] = 0;

    if (buf[0] == '#') {
	char *c;
	/* Read to the second space */
	c = buf;
	for (c = buf; *c != ' '; c++);
	for (++c; *c != ' '; c++);

	c++;

	width = atoi(c);
    }

    fgets(buf, BUFSIZ, infile);

    if (buf[0] == '#') {
	char *c;
	/* Read to the second space */
	c = buf;
	for (c = buf; *c != ' '; c++);
	for (++c; *c != ' '; c++);

	height = atoi(c);
    }

    fgets(buf, BUFSIZ, infile);

    /* Now that we know the width and height, we can construct */
    /* our bitmask.  First, allocate the space required for the file */

    cursor = (GR_BITMAP *) malloc(sizeof(GR_BITMAP) * height);

    /* Now fill it up */

    fgets(buf, BUFSIZ, infile);
    c = buf;

    for (bm = 0; bm < height; bm++) {
	char *d;
	unsigned long tval, val;
	int i;
	/* Read the right number of bits.  If we are at the end, then get a new line */

	val = 0;

	for (i = ((width / 8) - 1); i >= 0; i--) {
	    if (*c == '\n') {
		fgets(buf, BUFSIZ, infile);
		c = buf;
	    }

	    if (*c == ',')
		c++;

	    tval = strtol(c, &d, 16);

	    val |= tval << (i * 8);

	    c = d;
	}

	cursor[bm] = val;
    }

    fclose(infile);

    *x = width;
    *y = height;
    *bitmap = cursor;
}

void
NXMCursor::load(char *file, int mask)
{
    load_file(file, &cursor, &width, &height);
    hx = 0;
    hy = 0;
}
