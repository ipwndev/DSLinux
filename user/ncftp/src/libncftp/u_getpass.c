/* u_getpass.c
 *
 * Copyright (c) 1996-2006 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#if ((defined(HAVE_TCGETATTR)) && (defined(HAVE_TERMIOS_H)))
#	include <termios.h>
#endif
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

char *
GetPass(const char *const prompt, char *const pwbuf, const size_t pwbufsize)
{
#if defined(HAVE_GETPASS) && !defined(_REENTRANT)
	char *cp;

	cp = getpass(prompt);
	if (cp != NULL) {
		strncpy(pwbuf, cp, pwbufsize);
		pwbuf[pwbufsize - 1] = '\0';
		cp = pwbuf;
	} else {
		(void) memset(pwbuf, 0, pwbufsize);
	}
	return (cp);
#elif defined(_CONSOLE) && ( (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__) )

	char *dst, *dlim;
	int c;

	(void) memset(pwbuf, 0, pwbufsize);
	if (! _isatty(_fileno(stdout)))
		return (pwbuf);
	(void) fputs(prompt, stdout);
	(void) fflush(stdout);

	for (dst = pwbuf, dlim = dst + pwbufsize - 1;;) {
		c = _getch();
		if ((c == 0) || (c == 0xe0)) {
			/* The key is a function or arrow key; read and discard. */
			(void) _getch();
		}
		if ((c == '\r') || (c == '\n'))
			break;
		if (dst < dlim)
			*dst++ = c;
	}
	*dst = '\0';

	(void) fflush(stdout);
	(void) fflush(stdin);
	return (pwbuf);
#elif ((defined(HAVE_TCGETATTR)) && (defined(HAVE_TERMIOS_H)))
	/* Should also work for Cygwin. */
	struct termios old_ti, new_ti;

	(void) memset(pwbuf, 0, pwbufsize);
	if (! isatty(fileno(stdout)))
		return (pwbuf);
	(void) fputs(prompt, stdout);
	(void) fflush(stdout);
	if (tcgetattr(fileno(stdout), &old_ti) != 0)
		return (pwbuf);
	new_ti = old_ti;
	new_ti.c_lflag &= ~ECHO;
	if (tcsetattr(fileno(stdout), TCSAFLUSH, &new_ti) != 0)
		return (pwbuf);
	(void) FGets(pwbuf, pwbufsize, stdin);
	(void) fflush(stdout);
	(void) fflush(stdin);
	(void) tcsetattr(fileno(stdout), TCSAFLUSH, &old_ti);
	return (pwbuf);
#else
	(void) memset(pwbuf, 0, pwbufsize);
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	if (! _isatty(_fileno(stdout)))
#else
	if (! isatty(1))
#endif
		return (pwbuf);
	(void) fputs(prompt, stdout);
	(void) fflush(stdout);
	(void) FGets(pwbuf, pwbufsize, stdin);
	(void) fflush(stdout);
	(void) fflush(stdin);
	return (pwbuf);
#endif
}	/* GetPass */
