/* u_slash.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Read a line, and axe the end-of-line. */
char *
FGets(char *str, size_t size, FILE *fp)
{
	char *cp, *nlptr;
	
	cp = fgets(str, ((int) size) - 1, fp);
	if (cp != NULL) {
		cp[((int) size) - 1] = '\0';	/* ensure terminator */
		nlptr = cp + strlen(cp) - 1;
		if (*nlptr == '\n')
			*nlptr = '\0';
	} else {
		memset(str, 0, size);
	}
	return cp;
}	/* FGets */

void
StrRemoveTrailingSlashes(char *dst)
{
	char *cp;

	/* Note: Do not destroy a path of "/" */
	cp = dst + strlen(dst);
	--cp;
	while ((cp > dst) && (*cp == '/'))
		*cp-- = '\0';
}	/* StrRemoveTrailingSlashes */
