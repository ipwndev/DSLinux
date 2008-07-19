#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/*VARARGS*/
const char *
DStrCatList(DStr *const dst, ...)
{
	size_t catLen, allocSize, newLen, curLen, srcLen;
	char *dcp, *cp, *osrc;
	const char *src;
	char *recursive;
	va_list ap;

	if (IS_DSTR_CORRUPT(dst))
		return NULL;

	osrc = dst->s;
	recursive = NULL;
	catLen = 0;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		if (src == osrc) {
			if (recursive == NULL)
				recursive = strdup(src);
		}
		catLen += strlen(src);
		src = va_arg(ap, char *);
	}
	va_end(ap);

	catLen++; 		/* copy NUL byte also */;
	curLen = dst->len;
	newLen = catLen + curLen;
	if (newLen > 0x00FFFFFF) {
		if (recursive != NULL)
			free(recursive);
		return NULL;
	}
	if (dst->allocSize < newLen) {
		/* Need to resize buffer before copying. */
		allocSize = (newLen + 16) & 0xFFFFFFF0;
		if (dst->s == NULL) {
			cp = calloc(allocSize, (size_t) 1);
			if (cp == NULL) {
				if (recursive != NULL)
					free(recursive);
				return NULL;
			}
		} else {
			cp = realloc(dst->s, allocSize);
			if (cp == NULL) {
				if (recursive != NULL)
					free(recursive);
				return NULL;
			}
			memset(cp + curLen, 0, allocSize - curLen);
		}
		dst->s = cp;
		dst->allocSize = allocSize;
	} else {
		cp = dst->s;
	}

	dcp = cp + curLen;
	va_start(ap, dst);
	src = va_arg(ap, char *);
	while (src != NULL) {
		if (src == osrc)
			src = recursive;
		srcLen = strlen(src);
		memcpy(dcp, src, srcLen);
		dcp += srcLen;
		src = va_arg(ap, char *);
	}
	va_end(ap);
	*dcp = '\0';

	dst->len = newLen - 1;
	if (recursive != NULL)
		free(recursive);
	return (cp);
}	/* DStrCatList */
