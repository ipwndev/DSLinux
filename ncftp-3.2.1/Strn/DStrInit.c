#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
DStrInit(DStr *const dst)
{
	memset(dst, 0, sizeof(DStr));
}	/* DStrInit */
