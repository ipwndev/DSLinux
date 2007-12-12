/* u_getopt.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* This must be called before each Getopt. */
void
GetoptReset(GetoptInfo *const opt)
{
	memset(opt, 0, sizeof(GetoptInfo));
	opt->ind = 1;
	opt->init = 0xF123456F;
}	/* GetoptReset */



int
Getopt(GetoptInfo *const opt, int nargc, char **const nargv, const char *const ostr)
{
	const char *oli;				   /* Option letter list index */
	if ((opt == NULL) || (nargc == 0) || (nargv == (char **) 0) || (ostr == NULL))
		return (EOF);

	if (opt->init != 0xF123456F) {
		/* We can do it for them. */
		GetoptReset(opt);
	}

	if ((opt->place == NULL) || (opt->place[0] == '\0')) {
		/* update scanning pointer */
		if (opt->ind >= nargc || *(opt->place = nargv[opt->ind]) != '-')
			return (EOF);
		if (opt->place[1] && *++opt->place == '-') {	/* found "--" */
			++opt->ind;
			return (EOF);
		}
	}								   /* Option letter okay? */

	if (opt->place == NULL)
		oli = NULL;
	else if ((opt->opt = (int) *opt->place++) == (int) ':')
		oli = NULL;
	else
		oli = strchr(ostr, opt->opt);

	if (oli == NULL) {
		if ((opt->place == NULL) || (opt->place[0] == '\0'))
			++opt->ind;
		if (opt->err)
			(void) fprintf(stderr, "%s%s%c\n", *nargv, ": illegal option -- ", opt->opt);
		return((int) '?');
	}
	if (*++oli != ':') {			   /* don't need argument */
		opt->arg = NULL;
		if ((opt->place == NULL) || (opt->place[0] == '\0'))
			++opt->ind;
	} else {						   /* need an argument */
		if ((opt->place != NULL) && (opt->place[0] != '\0'))
			opt->arg = (char *) opt->place;
		else if (nargc <= ++opt->ind) {  /* no arg */
			opt->place = NULL;
			if (opt->err) 
				(void) fprintf(stderr, "%s%s%c\n", *nargv, ": option requires an argument -- ", opt->opt);
			return((int) '?');
		} else						   /* white space */
			opt->arg = (char *) nargv[opt->ind];
		opt->place = NULL;
		++opt->ind;
	}
	return (opt->opt);				   /* dump back Option letter */
}									   /* Getopt */

/* eof */
