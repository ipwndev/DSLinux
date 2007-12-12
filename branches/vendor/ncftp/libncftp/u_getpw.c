/* u_getpw.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#if defined(SOLARIS) && (SOLARIS >= 250)
#	define _POSIX_PTHREAD_SEMANTICS 1
#endif

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#else

#ifdef BSDOS
int getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
#endif

#if (defined(AIX) && (AIX < 433))
extern int _posix_getpwuid_r(uid_t, struct passwd *, char *, int
, struct passwd **);
extern int _posix_getpwnam_r(const char *, struct passwd *, char *, int, struct passwd **);
#endif

int
GetPwUid(struct passwd *pwp, const uid_t uid, char *const pwbuf, size_t pwbufsize)
{
#if ((defined(MACOSX)) && (defined(HAVE_GETPWUID_R)))
	/* Allow backwards compat on 10.1 if building on 10.2+ */
#	undef HAVE_GETPWUID_R
#	undef HAVE_GETPWNAM_R
#endif

#if defined(HAVE_GETPWUID_R) && ( (defined(SOLARIS) && (SOLARIS < 250)) || (defined(IRIX) && (IRIX < 6)) )
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = getpwuid_r(uid, pwp, pwbuf, pwbufsize);
	if (p != NULL)
		return (0);
#elif defined(HAVE_GETPWUID_R) && (defined(HPUX)) && (HPUX < 1100)
	memset(pwbuf, 0, pwbufsize);
	if (getpwuid_r(uid, pwp, pwbuf, pwbufsize) >= 0)
		return (0);
#elif defined(HAVE__POSIX_GETPWUID_R)
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((_posix_getpwuid_r(uid, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#elif defined(HAVE_GETPWUID_R)
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((getpwuid_r(uid, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#else
	struct passwd *p;
	p = getpwuid(uid);
	if (p != NULL) {
		memcpy(pwp, p, sizeof(struct passwd));
		return (0);
	} else {
		memset(pwbuf, 0, pwbufsize);
		memset(pwp, 0, sizeof(struct passwd));
	}
#endif
	return (-1);
}	/* GetPwUid */




int
GetPwNam(struct passwd *pwp, const char *const nam, char *const pwbuf, size_t pwbufsize)
{
#if defined(HAVE_GETPWNAM_R) && ( (defined(SOLARIS) && (SOLARIS < 250)) || (defined(IRIX) && (IRIX < 6)) )
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = getpwnam_r(nam, pwp, pwbuf, pwbufsize);
	if (p != NULL)
		return (0);
#elif defined(HAVE_GETPWNAM_R) && (defined(HPUX)) && (HPUX < 1100)
	memset(pwbuf, 0, pwbufsize);
	if (getpwnam_r(nam, pwp, pwbuf, pwbufsize) >= 0)
		return (0);
#elif defined(HAVE__POSIX_GETPWNAM_R)
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((_posix_getpwnam_r(nam, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#elif defined(HAVE_GETPWNAM_R)
	struct passwd *p;
	memset(pwbuf, 0, pwbufsize);
	p = NULL;
	if ((getpwnam_r(nam, pwp, pwbuf, pwbufsize, &p) == 0) && (p != NULL)) {
		return (0);
	}
#else
	struct passwd *p;
	p = getpwnam(nam);
	if (p != NULL) {
		memcpy(pwp, p, sizeof(struct passwd));
		return (0);
	} else {
		memset(pwbuf, 0, pwbufsize);
		memset(pwp, 0, sizeof(struct passwd));
	}
#endif
	return (-1);
}	/* GetPwNam */



/* This looks up the user's password entry, trying to look by the username.
 * We have a couple of extra hacks in place to increase the probability
 * that we can get the username.
 */
int
GetMyPwEnt(struct passwd *pwp, char *const pwbuf, size_t pwbufsize)
{
	char *cp;
	int rc;
#ifdef HAVE_GETLOGIN_R
	char logname[128];
#endif
	rc = GetPwUid(pwp, getuid(), pwbuf, pwbufsize);
	if (rc == 0)
		return (rc);

	cp = (char *) getenv("LOGNAME");
	if (cp == NULL)
		cp = (char *) getenv("USER");

	if (cp == NULL) {
		/* Avoid getlogin() if possible, which may
		 * wade through a potentially large utmp file.
		 */
#ifdef HAVE_GETLOGIN_R
		memset(logname, 0, sizeof(logname));
		(void) getlogin_r(logname, sizeof(logname) - 1);
		cp = (logname[0] == '\0') ? NULL : logname;
#else
		cp = getlogin();
#endif
	}

	rc = -1;
	if ((cp != NULL) && (cp[0] != '\0'))
		rc = GetPwNam(pwp, cp, pwbuf, pwbufsize);
	return (rc);
}	/* GetMyPwEnt */

#endif	/* UNIX */
