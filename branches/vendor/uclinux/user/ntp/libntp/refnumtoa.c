/*
 * refnumtoa - return asciized refclock addresses stored in local array space
 */
#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"

char *
refnumtoa(
	u_int32 num
	)
{
	register u_int32 netnum;
	register char *buf;
	register const char *rclock;
	
	netnum = ntohl(num);
	
	LIB_GETBUF(buf);

	rclock = clockname((int)((u_long)netnum >> 8) & 0xff);

	if (rclock != NULL)
	    (void)sprintf(buf, "%s(%lu)", rclock, (u_long)netnum & 0xff);
	else
	    (void)sprintf(buf, "REFCLK(%lu,%lu)",
			  ((u_long)netnum >> 8) & 0xff, (u_long)netnum & 0xff);
	return buf;
}
