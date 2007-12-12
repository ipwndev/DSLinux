#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

const char *
DStrCpy(DStr *const dst, const char *const src)
{
	size_t srcLen, allocSize;
	char *cp;

	if (IS_DSTR_CORRUPT(dst))
		return NULL;

	if (dst->s == src)
		return (dst->s);

	srcLen = strlen(src) + 1 /* copy NUL byte also */;
	if (srcLen > 0x00FFFFFF)
		return NULL;
	if (dst->allocSize < srcLen) {
		/* Need to resize buffer before copying. */
		allocSize = (srcLen + 16) & 0xFFFFFFF0;
		if (dst->s == NULL) {
			cp = calloc(allocSize, (size_t) 1);
			if (cp == NULL)
				return NULL;
		} else {
			cp = realloc(dst->s, allocSize);
			if (cp == NULL)
				return NULL;
			memset(cp, 0, allocSize);
		}
		dst->s = cp;
		dst->allocSize = allocSize;
	} else {
		cp = dst->s;
	}

	memcpy(cp, src, srcLen);
	dst->len = srcLen - 1;
	return (cp);
}	/* DStrCpy */
