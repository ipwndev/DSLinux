/*
 *  Copyright (C) 2004 Nigel Horne <njh@bandsman.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Log$
 * Revision 1.1.2.1  2006/02/18 16:27:11  stsp2
 * adding uClinux-dist CVS snapshot from 2005-10-14
 *
 * Revision 1.4  2004/10/14 17:45:55  nigelhorne
 * Try to reclaim some memory if it becomes low when decoding
 *
 * Revision 1.3  2004/08/21 11:57:57  nigelhorne
 * Use line.[ch]
 *
 * Revision 1.2  2004/08/20 19:06:45  kojm
 * add line.[ch]
 *
 * Revision 1.1  2004/08/20 11:58:20  nigelhorne
 * First draft
 *
 */

#ifndef __LINE_H
#define __LINE_H

#ifdef	OLD
/* easier to read, but slower */

typedef struct line {
	char	*l_str;	/* the line's contents */
	unsigned int	l_refs;	/* the number of references to the data */
} line_t;
#else
typedef	char	line_t;	/* first byte is the ref count */
#endif

line_t	*lineCreate(const char *data);
line_t	*lineLink(line_t *line);
line_t	*lineUnlink(line_t *line);
const	char	*lineGetData(const line_t *line);
unsigned	char	lineGetRefCount(const line_t *line);

#endif
