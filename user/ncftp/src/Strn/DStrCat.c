#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

const char *
DStrCat(DStr *const dst, const char *const src)
{
	size_t srcLen, allocSize, newLen, curLen;
	char *cp;

	if (IS_DSTR_CORRUPT(dst))
		return NULL;

	srcLen = strlen(src) + 1 /* copy NUL byte also */;
	curLen = dst->len;
	newLen = srcLen + curLen;
	if (newLen > 0x00FFFFFF)
		return NULL;
	if (dst->allocSize < newLen) {
		/* Need to resize buffer before copying. */
		allocSize = (newLen + 16) & 0xFFFFFFF0;
		if (dst->s == NULL) {
			cp = calloc(allocSize, (size_t) 1);
			if (cp == NULL)
				return NULL;
		} else {
			cp = realloc(dst->s, allocSize);
			if (cp == NULL)
				return NULL;
			memset(cp + curLen, 0, allocSize - curLen);
		}
		dst->s = cp;
		dst->allocSize = allocSize;
	} else {
		cp = dst->s;
	}

	memcpy(cp + curLen, src, --srcLen);
	dst->len = newLen - 1;
	cp[newLen - 1] = '\0';
	return (cp);
}	/* DStrCat */
