/* unls.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

static const char *rwx[9] = { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx", NULL };

#if 0
/* May need this later. */
static void
CheckForLS_d(FTPCIPtr cip)
{
	FTPLineList lines;
	char *cp;

	if (cip->hasNLST_d == kCommandAvailabilityUnknown) {
		if (FTPListToMemory2(cip, ".", &lines, "-d ", 0, (int *) 0) == kNoErr) {
			if ((lines.first != NULL) && (lines.first == lines.last)) {
				/* If we have only one item in the list, see if it really was
				 * an error message we would recognize.
				 */
				cp = strchr(lines.first->line, ':');
				if ((cp != NULL) && STREQ(cp, ": No such file or directory")) {
					cip->hasNLST_d = kCommandNotAvailable;
				} else {
					cip->hasNLST_d = kCommandAvailable;
				}
			} else {
				cip->hasNLST_d = kCommandNotAvailable;
			}
		} else {
			cip->hasNLST_d = kCommandNotAvailable;
		}
		DisposeLineListContents(&lines);
	}
}	/* CheckForLS_d */
#endif




static int
LsMonthNameToNum(char *cp)
{
	int mon;	/* 0..11 */

	switch (*cp++) {
		case 'A':
			mon = (*cp == 'u') ? 7 : 3;
			break;
		case 'D':
			mon = 11;
			break;
		case 'F':
			mon = 1;
			break;
		default:
		case 'J':
			if (*cp++ == 'u')
				mon = (*cp == 'l') ? 6 : 5;
			else
				mon = 0;
			break;
		case 'M':
			mon = (*++cp == 'r') ? 2 : 4;
			break;
		case 'N':
			mon = 10;
			break;
		case 'O':
			mon = 9;
			break;
		case 'S':
			mon = 8;
	}
	return (mon);
}	/* LsMonthNameToNum */




static int
UnDosLine(	char *const line,
		const char *const curdir,
		size_t curdirlen,
		char *fname,
		size_t fnamesize,
		int *ftype,
		longest_int *fsize,
		time_t *ftime)
{
	char *cp;
	int hour, year;
	char *filestart;
	char *sizestart;
	struct tm ftm;

	/*
	 *
0123456789012345678901234567890123456789012345678901234567890123456789
04-27-99  10:32PM               270158 Game booklet.pdf
03-11-99  10:03PM       <DIR>          Get A3d Banner

We also try to parse the format from CMD.EXE, which is similar:

03/22/2001  06:23p              62,325 cls.pdf

	 *
	 */
	cp = line;
	if (
		isdigit((int) cp[0])
		&& isdigit((int) cp[1])
		&& ispunct((int) cp[2])
		&& isdigit((int) cp[3])
		&& isdigit((int) cp[4])
		&& ispunct((int) cp[5])
		&& isdigit((int) cp[6])
		&& isdigit((int) cp[7])
	) {
		(void) memset(&ftm, 0, sizeof(struct tm));
		ftm.tm_isdst = -1;
		cp[2] = '\0';
		ftm.tm_mon = atoi(cp + 0);
		if (ftm.tm_mon > 0)
			ftm.tm_mon -= 1;
		cp[5] = '\0';
		ftm.tm_mday = atoi(cp + 3);
		if ((isdigit((int) cp[8])) && (isdigit((int) cp[9]))) {
			/* Four-digit year */
			cp[10] = '\0';
			year = atoi(cp + 6);
			if (year > 1900)
				year -= 1900;
			ftm.tm_year = year;	/* years since 1900 */
			cp += 11;
		} else {
			/* Two-digit year */
			cp[8] = '\0';
			year = atoi(cp + 6);
			if (year < 98)
				year += 100;
			ftm.tm_year = year;	/* years since 1900 */
			cp += 9;
		}

		for (;;) {
			if (*cp == '\0')
				return (-1);
			if (isdigit((int) *cp))
				break;
			cp++;
		}

		cp[2] = '\0';
		hour = atoi(cp);
		if (((cp[5] == 'P') || (cp[5] == 'p')) && (hour < 12))
			hour += 12;
		else if (((cp[5] == 'A') || (cp[5] == 'a')) && (hour == 12))
			hour -= 12;
		ftm.tm_hour = hour;
		cp[5] = '\0';
		ftm.tm_min = atoi(cp + 3);
		*ftime = mktime(&ftm);
		if (*ftype == (time_t) -1)
			return (-1);

		cp += 6;
		*ftype = '-';
		for (;;) {
			if (*cp == '\0')
				return (-1);
			if ((*cp == '<') && (cp[1] == 'D')) {
				/* found <DIR> */
				*ftype = 'd';
				cp += 5;
				break;	/* size field will end up being empty string */
			} else if ((*cp == '<') && (cp[1] == 'J')) {
				/* found <JUNCTION>
				 *
				 * Will we ever really see this?
				 * IIS from Win2000sp1 sends <DIR>
				 * for FTP, but CMD.EXE prints
				 * <JUNCTION>.
				 */
				*ftype = 'd';
				cp += 10;
				break;
			} else if (isdigit((int) *cp)) {
				break;
			} else {
				cp++;
			}
		}

		sizestart = cp;
		for (;;) {
			if (*cp == '\0')
				return (-1);
#ifdef HAVE_MEMMOVE
			if (*cp == ',') {
				/* Yuck -- US Locale dependency */
				memmove(cp, cp + 1, strlen(cp + 1) + 1);
			}
#endif
			if (!isdigit((int) *cp)) {
				*cp++ = '\0';
				break;
			}
			cp++;
		}

		if (fsize != NULL) {
#if defined(HAVE_LONG_LONG) && defined(SCANF_LONG_LONG)
			if (*ftype == 'd')
				*fsize = 0;
			else
				(void) sscanf(sizestart, SCANF_LONG_LONG, fsize);
#elif defined(HAVE_LONG_LONG) && defined(HAVE_STRTOQ)
			if (*ftype == 'd')
				*fsize = 0;
			else
				*fsize = (longest_int) strtoq(sizestart, NULL, 0);
#else
			*fsize = (longest_int) 0;
			if (*ftype != 'd') {
				long fsize2 = 0L;

				(void) sscanf(sizestart, "%ld", &fsize2);
				*fsize = (longest_int) fsize2;
			}
#endif
		}

		for (;;) {
			if (*cp == '\0')
				return (-1);
			if (!isspace((int) *cp)) {
				break;
			}
			cp++;
		}

		filestart = cp;
		if (curdirlen == 0) {
			(void) Strncpy(fname, filestart, fnamesize);
		} else {
			(void) Strncpy(fname, curdir, fnamesize);
			(void) Strncat(fname, filestart, fnamesize);
		}

		return (0);
	}
	return (-1);
}	/* UnDosLine */




static int
UnLslRLine(	char *const line,
		const char *const curdir,
		size_t curdirlen,
		char *fname,
		size_t fnamesize,
		char *linkto,
		size_t linktosize,
		int *ftype,
		longest_int *fsize,
		time_t *ftime,
		time_t now,
		int thisyear,
		int *plugend)
{
	char *cp;
	int mon = 0, dd = 0, hr = 0, min = 0, year = 0;
	char *monstart, *ddstart, *hrstart, *minstart, *yearstart;
	char *linktostart, *filestart = NULL;
	char *sizestart;
	char *pe;
	struct tm ftm;

	*plugend = 0;
	*ftype = 0;
	if (ftime != NULL)
		*ftime = (time_t) -1;
	if (fsize != NULL)
		*fsize = (longest_int) -1;

	/*
	 * Look for the digit just before the space
	 * before the month name.
	 *
-rw-rw----   1 gleason  sysdev      33404 Mar 24 01:29 RCmd.o
-rw-rw-r--   1 gleason  sysdevzz     1829 Jul  7  1996 README
-rw-rw-r--   1 gleason  sysdevzz     1829 Jul 7   1996 README
-rw-rw-r--   1 gleason  sysdevzz     1829 Jul  7 1996  README
-rw-rw-r--   1 gleason  sysdevzz     1829 Jul 7  1996  README
         *
	 *------------------------------^
	 *                              0123456789012345
	 *------plugend--------^
	 *                     9876543210
	 *
	 */
	for (cp = line; *cp != '\0'; cp++) {
		if (	(isdigit((int) *cp))
			&& (isspace((int) cp[1]))
			&& (isupper((int) cp[2]))
			&& (islower((int) cp[3]))
		/*	&& (islower((int) cp[4])) */
			&& (isspace((int) cp[5]))
			&& (
((isdigit((int) cp[6])) && (isdigit((int) cp[7])))
|| ((isdigit((int) cp[6])) && (isspace((int) cp[7])))
|| ((isspace((int) cp[6])) && (isdigit((int) cp[7])))
			)
			&& (isspace((int) cp[8]))
		) {
			monstart = cp + 2;
			ddstart = cp + 6;
			if (	((isspace((int) cp[9])) || (isdigit((int) cp[9])))
				&& (isdigit((int) cp[10]))
				&& (isdigit((int) cp[11]))
				&& (isdigit((int) cp[12]))
				&& ((isdigit((int) cp[13])) || (isspace((int) cp[13])))
			) {
				/* "Mon DD  YYYY" form */
				yearstart = cp + 9;
				if (isspace((int) *yearstart))
					yearstart++;
				hrstart = NULL;
				minstart = NULL;
				filestart = cp + 15;
				cp[1] = '\0';	/* end size */
				cp[5] = '\0';	/* end mon */
				cp[8] = '\0';	/* end dd */
				cp[14] = '\0';	/* end year */
				mon = LsMonthNameToNum(monstart);
				dd = atoi(ddstart);
				hr = 23;
				min = 59;
				year = atoi(yearstart);

				pe = cp;
				while (isdigit((int) *pe))
					pe--;
				while (isspace((int) *pe))
					pe--;
				*plugend = (int) (pe - line) + 1;
				break;
			} else if (	/*
					 * Windows NT does not 0 pad.
					(isdigit((int) cp[9])) &&
					 */
				(isdigit((int) cp[10]))
				&& (cp[11] == ':')
				&& (isdigit((int) cp[12]))
				&& (isdigit((int) cp[13]))
			) {
				/* "Mon DD HH:MM" form */
				yearstart = NULL;
				hrstart = cp + 9;
				minstart = cp + 12;
				filestart = cp + 15;
				cp[1] = '\0';	/* end size */
				cp[5] = '\0';	/* end mon */
				cp[8] = '\0';	/* end dd */
				cp[11] = '\0';	/* end hr */
				cp[14] = '\0';	/* end min */
				mon = LsMonthNameToNum(monstart);
				dd = atoi(ddstart);
				hr = atoi(hrstart);
				min = atoi(minstart);
				year = 0;

				pe = cp;
				while (isdigit((int) *pe))
					pe--;
				while (isspace((int) *pe))
					pe--;
				*plugend = (int) (pe - line) + 1;
				break;
			}
		}
	}

	if (*cp == '\0')
		return (-1);

	linktostart = strstr(filestart, " -> ");
	if (linktostart != NULL) {
		*linktostart = '\0';
		linktostart += 4;
		(void) Strncpy(linkto, linktostart, linktosize);
	} else {
		*linkto = '\0';
	}

	if (curdirlen == 0) {
		(void) Strncpy(fname, filestart, fnamesize);
	} else {
		(void) Strncpy(fname, curdir, fnamesize);
		(void) Strncat(fname, filestart, fnamesize);
	}

	if (ftime != NULL) {
		(void) memset(&ftm, 0, sizeof(struct tm));
		ftm.tm_mon = mon;
		ftm.tm_mday = dd;
		ftm.tm_hour = hr;
		ftm.tm_min = min;
		ftm.tm_isdst = -1;
		if (year == 0) {
			/* We guess the year, based on what the
			 * current year is.  We know the file
			 * on the remote server is either less
			 * than six months old or less than
			 * one hour into the future.
			 */
			ftm.tm_year = thisyear - 1900;
			*ftime = mktime(&ftm);
			if (*ftime == (time_t) -1) {
				/* panic */
			} else if (*ftime > (now + (15552000L + 86400L))) {
				--ftm.tm_year;
				*ftime = mktime(&ftm);
			} else if (*ftime < (now - (15552000L + 86400L))) {
				++ftm.tm_year;
				*ftime = mktime(&ftm);
			}
		} else {
			ftm.tm_year = year - 1900;
			*ftime = mktime(&ftm);
		}
	}

	if (fsize != NULL) {
		while ((cp > line) && (isdigit((int) *cp)))
			--cp;
		sizestart = cp + 1;
#if defined(HAVE_LONG_LONG) && defined(SCANF_LONG_LONG)
		(void) sscanf(sizestart, SCANF_LONG_LONG, fsize);
#elif defined(HAVE_LONG_LONG) && defined(HAVE_STRTOQ)
		*fsize = (longest_int) strtoq(sizestart, NULL, 0);
#else
		{
			long fsize2 = 0L;
			
			(void) sscanf(sizestart, "%ld", &fsize2);
			*fsize = (longest_int) fsize2;
		}
#endif
	}

	switch (line[0]) {
		case 'd':
		case 'l':
			*ftype = (int) line[0];
			break;
		case 'b':
		case 'c':
		case 's':
			*ftype = (int) line[0];
			return (-1);
		default:
			*ftype = '-';
	}

	return (0);
}	/* UnLslRLine */



int
UnLslR(const FTPCIPtr cip, FTPFileInfoListPtr filp, FTPLineListPtr llp, int serverType)
{
	char curdir[512];
	char line[512];
	int len;
	size_t curdirlen = 0;
	char fname[256];
	char linkto[256];
	char *cp;
	longest_int fsize;
	int ftype;
	time_t ftime, now;
	int thisyear;
	struct tm nowtm;
	int rc;
	FTPLinePtr lp;
	FTPFileInfo fi;
	int linesread = 0;
	int linesconverted = 0;
	size_t maxFileLen = 0;
	size_t maxPlugLen = 0;
	size_t fileLen;
	int plugend;
	int skipdir = 0;
	size_t cwdlen;

	if (Localtime(time(&now), &nowtm) == NULL)
		thisyear = 1970;	/* should never happen */
	else
		thisyear = nowtm.tm_year + 1900;

	curdir[0] = '\0';
	cip->buf[0] = '\0';
	cwdlen = 0;

	InitFileInfoList(filp);
	for (lp = llp->first; lp != NULL; lp = lp->next) {
		len = (int) strlen(STRNCPY(line, lp->line));
		for (cp = line; ; cp++) {
			if ((*cp == '\0') || (!isspace((int) *cp)))
				break;
		}
		if (*cp == '\0') {
			/* Entire line was blank. */
			/* separator line between dirs */
			continue;
		}

		if (serverType == kServerTypeMicrosoftFTP) {
			/* IIS runs on WinNT only, and NT
			 * can use / as a separator rather
			 * that \.  We will convert them
			 * here.
			 */
			for (cp = line; *cp != '\0'; cp++) {
				if (*cp == '\\')
					*cp = '/';
			}
		}

		linesread++;
		rc = UnLslRLine(line, curdir, curdirlen, fname, sizeof(fname), linkto, sizeof(linkto), &ftype, &fsize, &ftime, now, thisyear, &plugend);
		if ((rc < 0) && (serverType == kServerTypeMicrosoftFTP)) {
			rc = UnDosLine(line, curdir, curdirlen, fname, sizeof(fname), &ftype, &fsize, &ftime);
			if (rc == 0) {
				*linkto = '\0';
				plugend = 0;
			}
		}

		if (rc == 0) {
			if (skipdir != 0)
				continue;
			cp = fname + curdirlen;
			if ((cp[0] == '.') && ((cp[1] == '\0') || ((cp[1] == '.') && (cp[2] == '\0'))))
				continue;	/* ignore . and .. */
			linesconverted++;
			cp = fname;
			if ((cp[0] == '.') && ((cp[1] == '/') || (cp[1] == '\\')))
				cp += 2;
			if ((*cp == '/') || (*cp == '\\')) {
				/* Absolute pathnames not allowed unless
				 * they are in the cwd.
				 */
				if (cip->buf[0] == '\0') {
					(void) FTPGetCWD(cip, cip->buf, cip->bufSize);
					cwdlen = strlen(cip->buf);
				}
				/* In root directory (cwdlen == 1), paths
				 * from root are OK.
				 */
				if (cwdlen > 1) {
					if (memcmp(cp, cip->buf, cwdlen) == 0) {
						/* Abs path prefixed with cwd */
						cp += cwdlen;
						if ((*cp == '/') || (*cp == '\\'))
							cp++;
					} else {
						/* Abs path not prefixed with cwd */
						continue;
					}
				}
			}
			if (PathContainsIntermediateDotDotSubDir(cp))
				continue;
			fileLen = strlen(cp);
			if (fileLen > maxFileLen)
				maxFileLen = fileLen;
			fi.relnameLen = fileLen;
			fi.relname = StrDup(cp);
			fi.rname = NULL;
			fi.lname = NULL;
			fi.rlinkto = (linkto[0] == '\0') ? NULL : StrDup(linkto);
			fi.mdtm = ftime;
			fi.size = (longest_int) fsize;
			fi.type = ftype;
			fi.mode = -1;
			if (plugend > 0) {
				fi.plug = (char *) malloc((size_t) plugend + 1);
				if (fi.plug != NULL) {
					(void) memcpy(fi.plug, line, (size_t) plugend);
					fi.plug[plugend] = '\0';
					if ((size_t) plugend > maxPlugLen)
						maxPlugLen = (size_t) plugend;
				}
			} else {
				fi.plug = (char *) malloc(32 + 1);
				if (fi.plug != NULL) {
					(void) strcpy(fi.plug, "----------   1 ftpuser  ftpusers");
					fi.plug[0] = (char) ftype;
					if (32 > maxPlugLen)
						maxPlugLen = (size_t) 32;
				}
			}
			(void) AddFileInfo(filp, &fi);
		} else if ((rc < 0) && (line[len - 1] == ':')) {
			if ((line[0] == '.') && (line[1] == '/')) {
				line[len - 1] = '/';
				(void) memcpy(curdir, line + 2, (size_t) len + 1 - 2);
				curdirlen = (size_t) (len - 2);
			} else if ((line[0] == '.') && (line[1] == '\\')) {
				line[len - 1] = '\\';
				(void) memcpy(curdir, line + 2, (size_t) len + 1 - 2);
				curdirlen = (size_t) (len - 2);
			} else {
				line[len - 1] = '/';
				(void) memcpy(curdir, line, (size_t) len + 1);
				curdirlen = (size_t) len;
			}
			skipdir = 0;
			cp = curdir;
			if ((*cp == '/') || (*cp == '\\')) {
				skipdir = 1;	/* absolute pathnames not allowed */
				if (cip->buf[0] == '\0') {
					(void) FTPGetCWD(cip, cip->buf, cip->bufSize);
					cwdlen = strlen(cip->buf);
				}
				if (cwdlen == 1) {
					/* In root directory, so paths
					 * from root are OK.
					 */
					skipdir = 0;
				} else {
					if (memcmp(cp, cip->buf, cwdlen) == 0) {
						cp += cwdlen;
						if ((*cp == '/') || (*cp == '\\'))
							cp++;
						memmove(curdir, cp, strlen(cp) + 1);
						skipdir = 0;
						cp = curdir;
					}
				}
			}
			if (PathContainsIntermediateDotDotSubDir(cp)) {
				skipdir = 1;
			}
		}
	}

	filp->maxFileLen = maxFileLen;
	filp->maxPlugLen = maxPlugLen;
	if (linesread == 0)
		return (0);
	return ((linesconverted > 0) ? linesconverted : (-1));
}	/* UnLslR */




int
UnMlsT(const FTPCIPtr UNUSED(cip), const char *const line0, const MLstItemPtr mlip)
{
	char *cp, *val, *fact;
	int ec;
	size_t len;
	char line[1024];

	LIBNCFTP_USE_VAR(cip);
	memset(mlip, 0, sizeof(MLstItem));
	mlip->mode = -1;
	mlip->fsize = kSizeUnknown;
	mlip->ftype = '-';
	mlip->ftime = kModTimeUnknown;

	len = strlen(line0);
	if (len > (sizeof(line) - 1))
		return (-1);	/* Line too long, sorry. */
	/* This should be re-coded so does not need to make a
	 * copy of the buffer; it could be done in place.
	 */
	memcpy(line, line0, len + 1);

	/* Skip leading whitespace. */
	for (cp = line; *cp != '\0'; cp++) {
		if (! isspace((int) *cp))
			break;
	}

	while (*cp != '\0') {
		for (fact = cp; ; cp++) {
			if ((*cp == '\0') || (*cp == ' ')) {
				/* protocol violation */
				return (-1);
			}
			if (*cp == '=') {
				/* End of fact name. */
				*cp++ = '\0';
				break;
			}
		}
		for (val = cp; ; cp++) {
			if (*cp == '\0') {
				/* protocol violation */
				return (-1);
			}
			if (*cp == ' ') {
				ec = ' ';
				*cp++ = '\0';
				break;
			} else if (*cp == ';') {
				if (cp[1] == ' ') {
					ec = ' ';
					*cp++ = '\0';
					*cp++ = '\0';
				} else {
					ec = ';';
					*cp++ = '\0';
				}
				break;
			}
		}
		if (ISTRNEQ(fact, "OS.", 3))
			fact += 3;
		if (ISTREQ(fact, "type")) {
			if (ISTREQ(val, "file")) {
				mlip->ftype = '-';
			} else if (ISTREQ(val, "dir")) {
				mlip->ftype = 'd';
			} else if (ISTREQ(val, "cdir")) {
				/* not supported: current directory */
				return (-2);
			} else if (ISTREQ(val, "pdir")) {
				/* not supported: parent directory */
				return (-2);
			} else {
				/* ? */
				return (-1);
			}
		} else if (ISTREQ(fact, "UNIX.mode")) {
			if (val[0] == '0')
				sscanf(val, "%o", &mlip->mode);
			else	
				sscanf(val, "%i", &mlip->mode);
			if (mlip->mode != (-1))
				mlip->mode &= 00777;
		} else if (ISTREQ(fact, "perm")) {
			STRNCPY(mlip->perm, val);
		} else if (ISTREQ(fact, "size")) {
#if defined(HAVE_LONG_LONG) && defined(SCANF_LONG_LONG)
			(void) sscanf(val, SCANF_LONG_LONG, &mlip->fsize);
#elif defined(HAVE_LONG_LONG) && defined(HAVE_STRTOQ)
			mlip->fsize = (longest_int) strtoq(val, NULL, 0);
#else
			{
				long fsize2 = 0L;

				(void) sscanf(val, "%ld", &fsize2);
				mlip->fsize = (longest_int) fsize2;
			}
#endif
		} else if (ISTREQ(fact, "modify")) {
			mlip->ftime = UnMDTMDate(val);
		} else if (ISTREQ(fact, "UNIX.owner")) {
			STRNCPY(mlip->owner, val);
		} else if (ISTREQ(fact, "UNIX.group")) {
			STRNCPY(mlip->group, val);
		} else if (ISTREQ(fact, "UNIX.uid")) {
			mlip->uid = atoi(val);
		} else if (ISTREQ(fact, "UNIX.gid")) {
			mlip->gid = atoi(val);
		} else if (ISTREQ(fact, "perm")) {
			STRNCPY(mlip->perm, val);
		}

		/* End of facts? */
		if (ec == ' ')
			break;
	}

	len = strlen(cp);
	if (len > (sizeof(mlip->fname) - 1)) {
		/* Filename too long */
		return (-1);
	}
	memcpy(mlip->fname, cp, len);

	/* also set linkto here if used */

	return (0);
}	/* UnMlsT */




int
UnMlsD(const FTPCIPtr cip, FTPFileInfoListPtr filp, FTPLineListPtr llp)
{
	MLstItem mli;
	char plug[64];
	char og[32];
	int rc;
	FTPLinePtr lp;
	FTPFileInfo fi;
	int linesread = 0;
	int linesconverted = 0;
	int linesignored = 0;
	size_t maxFileLen = 0;
	size_t maxPlugLen = 0;
	size_t fileLen, plugLen;
	int m1, m2, m3;
	const char *cm1, *cm2, *cm3;

	InitFileInfoList(filp);
	for (lp = llp->first; lp != NULL; lp = lp->next) {
		linesread++;
		rc = UnMlsT(cip, lp->line, &mli);
		if (rc == 0) {
			if (PathContainsIntermediateDotDotSubDir(mli.fname)) {
				linesignored++;
				continue;
			}
			fileLen = strlen(mli.fname);
			linesconverted++;
			if (fileLen > maxFileLen)
				maxFileLen = fileLen;
			fi.relnameLen = fileLen;
			fi.relname = StrDup(mli.fname);
			fi.rname = NULL;
			fi.lname = NULL;
			fi.rlinkto = (mli.linkto[0] == '\0') ? NULL : StrDup(mli.linkto);
			fi.mdtm = mli.ftime;
			fi.size = (longest_int) mli.fsize;
			fi.type = mli.ftype;
			fi.mode = -1;
			plug[0] = (char) mli.ftype;
			plug[1] = '\0';
			m1 = 0;
			m2 = 0;
			m3 = -1;
			if (mli.mode != (-1)) {
				fi.mode = mli.mode;
				m1 = (mli.mode & 00700) >> 6;
				m2 = (mli.mode & 00070) >> 3;
				m3 = (mli.mode & 00007);
			}
			if (mli.perm[0] != '\0') {
				m3 = 0;
				if (fi.type == 'd') {
					if (strchr(mli.perm, 'e') != NULL) {
						/* execute -> execute */
						m3 |= 00001;
					}
					if (strchr(mli.perm, 'c') != NULL) {
						/* create -> write */
						m3 |= 00002;
					}
					if (strchr(mli.perm, 'l') != NULL) {
						/* list -> read */
						m3 |= 00004;
					}
				} else {
					if (strchr(mli.perm, 'w') != NULL) {
						/* write -> write */
						m3 |= 00002;
					}
					if (strchr(mli.perm, 'r') != NULL) {
						/* read -> read */
						m3 |= 00004;
					}
				}
			}
			if (m3 != (-1)) {
				cm1 = rwx[m1];
				cm2 = rwx[m2];
				cm3 = rwx[m3];
				sprintf(plug + 1, "%s%s%s", cm1, cm2, cm3);
			}
			if (mli.owner[0] != '\0') {
				if (mli.group[0] != '\0') {
#ifdef HAVE_SNPRINTF
					snprintf(og, sizeof(og) - 1,
#else
					sprintf(og,
#endif	/* HAVE_SNPRINTF */
						"   %-8.8s %s",
						mli.owner, mli.group
					);
					STRNCAT(plug, og);
				} else {
					STRNCAT(plug, "   ");
					STRNCAT(plug, mli.owner);
				}
			}
			fi.plug = StrDup(plug);
			if (fi.plug != NULL) {
				plugLen = strlen(plug);
				if (plugLen > maxPlugLen)
					maxPlugLen = plugLen;
			}
			(void) AddFileInfo(filp, &fi);
		} else if (rc == (-2)) {
			linesignored++;
		}
	}

	filp->maxFileLen = maxFileLen;
	filp->maxPlugLen = maxPlugLen;
	if ((linesread == 0) || ((linesconverted == 0) && (linesignored > 0)))
		return (0);
	return ((linesconverted > 0) ? linesconverted : (-1));
}	/* UnMlsD */


