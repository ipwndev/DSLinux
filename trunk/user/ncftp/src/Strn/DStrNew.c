#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
DStrNew(DStr *const dst, const size_t srcLen)
{
	size_t allocSize;
	char *cp;

	memset(dst, 0, sizeof(DStr));
	if (srcLen > 0x00FFFFFF)
		return (-1);
	allocSize = (srcLen + 16) & 0xFFFFFFF0;
	cp = calloc(allocSize, (size_t) 1);
	if (cp == NULL)
		return (-1);
	dst->allocSize = allocSize;
	dst->s = cp;
	dst->len = 0;
	return (0);
}	/* DStrNew */
