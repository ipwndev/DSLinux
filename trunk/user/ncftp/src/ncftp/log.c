/* log.c
 *
 * Copyright (c) 1992-2005 by Mike Gleason.
 * All rights reserved.
 * 
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#include "util.h"
#include "log.h"

extern int gMaxLogSize;
char gLogFileName[256];

extern char gOurDirectoryPath[];


void
InitLog(void)
{
	OurDirectoryPath(gLogFileName, sizeof(gLogFileName), kLogFileName);
}	/* InitLog */



void
LogXfer(const char *const mode, const char *const url)
{
	FILE *fp;

	if (gMaxLogSize == 0)
		return;		/* Don't log */

	fp = fopen(gLogFileName, FOPEN_APPEND_TEXT);
	if (fp != NULL) {
		(void) fprintf(fp, "  %s %s\n", mode, url);
		(void) fclose(fp);
	}
}	/* LogOpen */



void
LogOpen(const char *const host)
{
	time_t now;
	FILE *fp;

	if (gMaxLogSize == 0)
		return;		/* Don't log */

	time(&now);
	fp = fopen(gLogFileName, FOPEN_APPEND_TEXT);
	if (fp != NULL) {
		(void) fprintf(fp, "%s at %s", host, ctime(&now));
		(void) fclose(fp);
	}
}	/* LogOpen */




void
EndLog(void)
{
	FILE *newfp, *oldfp;
	struct Stat st;
	long fat;
	char str[512];
	char siteline[128];
	char tmpLog[256];

	if (gOurDirectoryPath[0] == '\0')
		return;		/* Don't create in root directory. */

	/* If the user wants to, s/he can specify the maximum size of the log file,
	 * so it doesn't waste too much disk space.  If the log is too fat, trim the
	 * older lines (at the top) until we're under the limit.
	 */
	if ((gMaxLogSize <= 0) || (Stat(gLogFileName, &st) < 0))
		return;						   /* Never trim, or no log. */

	if ((size_t)st.st_size < (size_t)gMaxLogSize)
		return;						   /* Log size not over limit yet. */

	if ((oldfp = fopen(gLogFileName, FOPEN_READ_TEXT)) == NULL)
		return;

	/* Want to make it so we're about 30% below capacity.
	 * That way we won't trim the log each time we run the program.
	 */
	fat = (long) st.st_size - (long) gMaxLogSize + (long) (0.20 * gMaxLogSize);
	siteline[0] = '\0';
	while (fat > 0L) {
		if (fgets(str, (int) sizeof(str), oldfp) == NULL)
			return;
		fat -= (long) strlen(str);
		if (! isspace((int) *str))
			STRNCPY(siteline, str);
	}
	/* skip lines until a new site was opened */
	for (fat = 0; fat < (long) (0.10 * gMaxLogSize); ) {
		if (fgets(str, (int) sizeof(str), oldfp) == NULL) {
			(void) fclose(oldfp);
			(void) remove(gLogFileName);
			return;					   /* Nothing left, start anew next time. */
		}
		fat += (long) strlen(str);
		if (! isspace((int) *str)) {
			siteline[0] = '\0';
			break;
		}
	}

	/* Copy the remaining lines in "old" to "new" */
	OurDirectoryPath(tmpLog, sizeof(tmpLog), "log.tmp");
	if ((newfp = fopen(tmpLog, FOPEN_WRITE_TEXT)) == NULL) {
		(void) fclose(oldfp);
		return;
	}
	if (siteline[0] != '\0') {
		if (siteline[strlen(siteline) - 1] == '\n')
			siteline[strlen(siteline) - 1] = '\0';
		if (siteline[strlen(siteline) - 1] =='\r')
			siteline[strlen(siteline) - 1] = '\0';
#ifdef HAVE_STRSTR
		if (strstr(siteline, "(other entries from this session have been purged)") != NULL) {
#else
		if (strchr(siteline, '(') != NULL) {
#endif
			(void) fprintf(newfp, "%s\n", siteline);
		} else {
			(void) fprintf(newfp, "%s %s\n",
				siteline,
				"(other entries from this session have been purged)"
			);
		}
	}

	(void) fputs(str, newfp);
	while (fgets(str, (int) sizeof(str), oldfp) != NULL)
		(void) fputs(str, newfp);
	(void) fclose(oldfp);
	(void) fclose(newfp);
	if (remove(gLogFileName) < 0)
		return;
	if (rename(tmpLog, gLogFileName) < 0)
		return;
}	/* EndLog */
