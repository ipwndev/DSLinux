/*
 *  Copyright (C) 2002 Nigel Horne <njh@bandsman.co.uk>
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
 * Revision 1.1.2.1  2006/02/18 16:27:12  stsp2
 * adding uClinux-dist CVS snapshot from 2005-10-14
 *
 * Revision 1.7  2004/12/04 16:03:55  nigelhorne
 * Text/plain now handled as no encoding
 *
 * Revision 1.6  2004/08/22 10:34:24  nigelhorne
 * Use fileblob
 *
 * Revision 1.5  2004/08/21 11:57:57  nigelhorne
 * Use line.[ch]
 *
 * Revision 1.4  2004/07/20 14:35:29  nigelhorne
 * Some MYDOOM.I were getting through
 *
 * Revision 1.3  2004/06/22 04:08:02  nigelhorne
 * Optimise empty lines
 *
 */

typedef struct text {
	line_t	*t_line;	/* NULL if the line is empty */
	struct	text	*t_next;
} text;

#include "message.h"

void	textDestroy(text *t_head);
text	*textClean(text *t_head);
text	*textAdd(text *t_head, const text *t);
text	*textAddMessage(text *aText, message *aMessage);
blob	*textToBlob(const text *t, blob *b);
fileblob	*textToFileblob(const text *t, fileblob *fb);
