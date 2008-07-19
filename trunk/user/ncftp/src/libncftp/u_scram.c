/* u_scram.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
Scramble(unsigned char *dst, size_t dsize, unsigned char *src, char *key)
{
	int i;
	unsigned int ch;
	unsigned char *k2;
	size_t keyLen;

	keyLen = strlen(key);
	k2 = (unsigned char *) key;
	for (i=0; i < (int) dsize - 1; i++) {
		ch = src[i];
		if (ch == 0)
			break;
		dst[i] = (unsigned char) (ch ^ (int) (k2[i % (int) keyLen]));
	}
	dst[i] = '\0';
}	/* Scramble */
