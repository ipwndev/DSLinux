#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
DStrClear(DStr *const dst)
{
	if (! IS_DSTR_CORRUPT(dst)) {
		if (dst->s != NULL)
			memset(dst->s, 0, dst->allocSize);
		dst->len = 0;
	}
}	/* DStrClear */



void
DStrFree(DStr *const dst)
{
	if (! IS_DSTR_CORRUPT(dst)) {
		if (dst->s != NULL)
			free(dst->s);
	}
	memset(dst, 0, sizeof(DStr));
}	/* DStrFree */
