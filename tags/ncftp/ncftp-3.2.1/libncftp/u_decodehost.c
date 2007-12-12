/* u_decodehost.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

int
FTPDecodeHostName(const FTPCIPtr cip, const char *const hstr0)
{
	char hstr[256];
	char *cp;
	char *hcp;
	int port;
	FILE *fp;
	
	/* Set user, pass, port if specified using the format:
	 *   [user[:pass]@]host[:port]
	 *   [user[/path/to/password.txt]@]host[:port]
	 *   /path/to/ftp_login.cfg
	 */
	 
	if (IsLocalPathDelim(hstr0[0])) {
		/* Found this style of format: /path/to/ftp_login.cfg */
		return (FTPReadLoginConfigFile(cip, hstr0));
	}
	
	(void) STRNCPY(hstr, hstr0);
	cp = strchr(hstr, '@');
	if (cp == NULL) {
		hcp = hstr;
	} else {
		/* Found the user:pass@ portion. */
		*cp++ = '\0';
		hcp = cp;
		cp = strchr(hstr, ':');
		if (cp != NULL) {
			/* Also had the :pass portion. */
			*cp++ = '\0';
			(void) STRNCPY(cip->pass, cp);
		} else {
			cp = strchr(hstr, '/');
			if (cp != NULL) {
				/* Also had the /path/to/password.txt portion. */
				fp = fopen(cp, FOPEN_READ_TEXT);
				*cp = '\0';
				if (fp == NULL) {
					/* Could not open password file */
					return (-3);
				}
				if (FGets(cip->pass, sizeof(cip->pass), fp) == NULL) {
					return (-4);
				}
				fclose(fp);
				/* Need to be careful with this -- don't put yourself
				 * in a situation where you get tricked into reading
				 * the first line of /etc/passwd and sending it as
				 * PASS to a fake FTP server.
				 */
			}
		}
		(void) STRNCPY(cip->user, hstr);
	}

	cp = strchr(hcp, '@');
	if (cp != NULL) {
		/* Syntax error */
		return (-1);
	}
	
	cp = strchr(hcp, ':');
	if (cp != NULL) {
		/* Found the :port portion */
		*cp++ = '\0';
		port = atoi(cp);
		if ((port <= 0) || (port > 65535))
			return (-2);
		cip->port = port;
	}
	
	(void) STRNCPY(cip->host, hcp);
	return (0);
}	/* FTPDecodeHostName */




int
FTPReadLoginConfigFile(FTPCIPtr cip, const char *const fn)
{
	FILE *fp;
	char line[256];
	char *cp;
	int goodfile = 0;

	fp = fopen(fn, FOPEN_READ_TEXT);
	if (fp == NULL) {
		return (-1);
	}

	memset(line, 0, sizeof(line));
	while (fgets(line, sizeof(line) - 1, fp) != NULL) {
		if ((line[0] == '#') || (isspace((int) line[0])))
			continue;
		cp = line + strlen(line) - 1;
		if (*cp == '\n')
			*cp = '\0';
		if (ISTRNCMP(line, "user", 4) == 0) {
			(void) STRNCPY(cip->user, line + 5);
			goodfile = 1;
		} else if (ISTRNCMP(line, "password", 8) == 0) {
			(void) STRNCPY(cip->pass, line + 9);
			goodfile = 1;
		} else if ((ISTRNCMP(line, "pass", 4) == 0) && (isspace((int) line[4]))) {
			(void) STRNCPY(cip->pass, line + 5);
			goodfile = 1;
		} else if (ISTRNCMP(line, "host", 4) == 0) {
			(void) STRNCPY(cip->host, line + 5);
			goodfile = 1;
		} else if (ISTRNCMP(line, "machine", 7) == 0) {
			(void) STRNCPY(cip->host, line + 8);
			goodfile = 1;
		} else if ((ISTRNCMP(line, "acct", 4) == 0) && (isspace((int) line[4]))) {
			(void) STRNCPY(cip->acct, line + 5);
		} else if (ISTRNCMP(line, "account", 7) == 0) {
			(void) STRNCPY(cip->acct, line + 8);
		}
	}
	(void) fclose(fp);

	return (goodfile ? 0 : -2);
}	/* FTPReadLoginConfigFile */
