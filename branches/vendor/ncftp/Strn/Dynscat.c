#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/*VARARGS*/
char *
Dynscat(char **dst, ...)
{
	va_list ap;
	const char *src;
	char *newdst, *dcp;
	size_t curLen, catLen, srcLen;
	int recursive = 0;

	if (dst == (char **) 0)
		return NULL;

	catLen = 0;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		if (src == *dst)
			recursive = 1;
		catLen += strlen(src);
		src = va_arg(ap, char *);
	}
	va_end(ap);

	if (recursive != 0) {
		/* Don't allow this:
		 *
		 *   Dynscat(&p, "foo", p, "bar", 0);
		 *
		 */
		if (*dst != NULL)
			free(*dst);
		*dst = NULL;
		return NULL;
	}

	if ((*dst == NULL) || (**dst == '\0'))
		curLen = 0;
	else
		curLen = strlen(*dst);

	if (*dst == NULL)
		newdst = malloc(curLen + catLen + 2);
	else
		newdst = realloc(*dst, curLen + catLen + 2);
	if (newdst == NULL)
		return NULL;

	dcp = newdst + curLen;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		srcLen = strlen(src);
		memcpy(dcp, src, srcLen);
		dcp += srcLen;
		src = va_arg(ap, char *);
	}
	va_end(ap);
	*dcp = '\0';

	*dst = newdst;
	return (newdst);
}	/* Dynscat */
