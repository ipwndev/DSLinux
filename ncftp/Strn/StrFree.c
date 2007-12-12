#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef HAVE_STRDUP
char *
strdup(const char *const src)
{
	size_t len;
	char *dst;

	if (src != NULL) {
		len = strlen(src) + 1;
		dst = malloc(len);
		if (dst != NULL) {
			(void) memcpy(dst, src, len);
			return (dst);
		}
	}
	return (NULL);
}	/* strdup */
#endif	/* HAVE_STRDUP */



void
StrFree(char **dst)
{
	if (dst != (char **) 0) {
		if (*dst != NULL) {
			free((void *) *dst);
			*dst = NULL;
		}
	}
}	/* StrFree */
