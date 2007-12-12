/* u_pathcat.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif


/* This simplifies a pathname, by converting it to the
 * equivalent of "cd $dir ; dir=`pwd`".  In other words,
 * if $PWD==/usr/spool/uucp, and you had a path like
 * "$PWD/../tmp////./../xx/", it would be converted to
 * "/usr/spool/xx".
 */

int
IsValidUNCPath(const char *const src)
{
	const char *a, *b;
	int c;
	int n;

	/* We may have a DOS path.  */
	if ((src[0] == '\\') && (src[1] == '\\') && (isalpha((int) src[2]))) {
		for (a = src + 3; ; ) {
			c = *a++;
			if (c == '\\')
				break;
			if ((! isalnum(c)) && (c != '_'))
				return (0);
		}
		b = a;
		c = *b++;
		if (! (isalpha(c)))
			return (0);	/* share does not start with letter */
		for ( ; ; ) {
			c = *b++;
			if ((c == '\\') || (c == '\0'))
				break;
			if ((! isalnum(c)) && (c != '_'))
				return (0);
		}
		n = (int) (b - src);
		return (n);
	}
	return (0);
}	/* IsValidUNCPath */



void
CompressPath(char *const dst, const char *const src, const size_t dsize, int dosCompat)
{
	int c;
	const char *s, *start, *dotp;
	char *d, *lim, *dstart;
	char *a, *b;
	char slash = (char) '/';
	size_t n;

#define isslash(c) ((c == '/') || (c == '\\'))
	s = start = src;
	d = dstart = dst;
	lim = d + dsize - 1;	/* leave room for nul byte. */

	if (dsize == 0)
		return;
	*dst = '\0';

	if ((s[0] == '\0') || (dsize < 4)) {
		return;
	} else if (dosCompat != 0) {
		if (src[0] == '\\') {
			/* We have a DOS path.  */
			slash = (char) '\\';
			n = (size_t) IsValidUNCPath(src);
			if (n != 0) {
				if (dsize < n)
					return;
				--n;
				memcpy(d, src, n);
				d += n;
				*d = '\0';
				dstart = d;
				s += n;
				start = s;
				/* unc = 1; */
			}
		} else if ((isalpha((int) src[0])) && (src[1] == ':')) {
			/* We may have a DOS driveletter+path.  */
			*d++ = src[0];
			*d++ = ':';
			start += 2;
			dstart += 2;
			s += 2;
			if (! isslash(src[2])) {
				slash = (char) '\\';
				*d++ = '\\';
			} else {
				slash = src[2];
				/* add it below *d++ = src[2]; */
			}
		}
	}

	for (;;) {
		c = *s;
		if (c == '.') {
			if (((s == start) || isslash(s[-1])) && (isslash(s[1]) || (s[1] == '\0'))) {
				/* Don't copy "./" */
				if (isslash(s[1]))
					++s;
				++s;
			} else if ((dosCompat != 0) && (s[1] == '.')) {
				if (d < lim)
					*d++ = *s++;
				if (d < lim)
					*d++ = *s++;
				if (*s == '.') {
					dotp = s;
					while (*dotp == '.')
						dotp++;
					if ((*dotp == '\0') || (isslash(*dotp))) {
						/* On DOS, "..." == "..",
						 * "...." == "..", etc,
						 * so skip the extra dots.
						 */
						s = dotp;
					}
				}
			} else if (d < lim) {
				*d++ = *s++;
			} else {
				++s;
			}
		} else if (isslash(c)) {
			/* Don't copy multiple slashes. */
			if (d < lim)
				*d++ = slash;
			++s;
			for (;;) {
				c = *s;
				if (isslash(c)) {
					/* Don't copy multiple slashes. */
					++s;
				} else if (c == '.') {
					c = s[1];
					if (isslash(c)) {
						/* Skip "./" */
						s += 2;
					} else if (c == '\0') {
						/* Skip "./" */
						s += 1;
					} else {
						break;
					}
				} else {
					break;
				}
			}
		} else if (c == '\0') {
			/* Remove trailing slash. */
			if (isslash(d[-1]) && (d > (dstart + 1)))
				d[-1] = '\0';
			*d = '\0';
			break;
		} else if (d < lim) {
			*d++ = *s++;
		} else {
			++s;
		}
	}
	a = dstart;

	/* fprintf(stderr, "<%s>\n", dst); */
	/* Go through and remove .. in the path when we know what the
	 * parent directory is.  After we get done with this, the only
	 * .. nodes in the path will be at the front.
	 */
	while (*a != '\0') {
		b = a;
		for (;;) {
			/* Get the next node in the path. */
			if (*a == '\0')
				return;
			if (isslash(*a)) {
				++a;
				break;
			}
			++a;
		}
		if ((b[0] == '.') && (b[1] == '.')) {
			if (isslash(b[2])) {
				/* We don't know what the parent of this
				 * node would be.
				 */
				continue;
			}
		}
		if ((a[0] == '.') && (a[1] == '.')) {
			if (isslash(a[2])) {
				/* Remove the .. node and the one before it. */
				if ((b == dstart) && (isslash(*dstart)))
					(void) memmove(b + 1, a + 3, strlen(a + 3) + 1);
				else
					(void) memmove(b, a + 3, strlen(a + 3) + 1);
				a = dstart;	/* Start over. */
			} else if (a[2] == '\0') {
				/* Remove a trailing .. like:  /aaa/bbb/.. */
				if (b == dstart) {
					dstart[0] = (char) ((isslash(start[0])) ? slash : '.');
					dstart[1] = '\0';
				} else if ((b <= dstart + 1) && isslash(*dstart)) {
					dstart[1] = '\0';
				} else {
					b[-1] = '\0';
				}
				a = dstart;	/* Start over. */
			} else {
				/* continue processing this node.
				 * It is probably some bogus path,
				 * like ".../", "..foo/", etc.
				 */
			}
		}
	}
#undef isslash
}	/* CompressPath */



void
PathCat(char *const dst, const size_t dsize, const char *const cwd, const char *const src, int dosCompat)
{
	char *cp;
	char tmp[512];

	if (dosCompat != 0) {
		if ((isalpha((int) cwd[0])) && (cwd[1] == ':')) {
			if ((isalpha((int) src[0])) && (src[1] == ':')) {
				/* A new fully-qualified DOS drive+path was requested. */
				CompressPath(dst, src, dsize, dosCompat);
				return;
			} else if (IsValidUNCPath(src)) {
				CompressPath(dst, src, dsize, dosCompat);
				return;
			} else if (src[0] == '\\') {
				/* A new fully-qualified DOS path on the same
				 * drive letter was requested.
				 */
				dst[0] = cwd[0];
				dst[1] = ':';
				CompressPath(dst + 2, src, dsize - 2, dosCompat);
				return;
			}
		} else if (IsValidUNCPath(src)) {
			/* A new fully-qualified DOS UNC path was requested.
			 * (but no drive letter was present on CWD?)
			 */
			CompressPath(dst, src, dsize, dosCompat);
			return;
		} else if ((src[0] == '\\') || ((isalpha((int) src[0])) && (src[1] == ':'))) {
			/* A new fully-qualified DOS path was requested.
			 * (but no drive letter was present on CWD?)
			 */
			CompressPath(dst, src, dsize, dosCompat);
			return;
		}
	}
	if ((src[0] == '/') || (src[0] == '~')) {
		/* A new fully-qualified UNIX path was requested. */
		CompressPath(dst, src, dsize, dosCompat);
		return;
	}

	cp = Strnpcpy(tmp, cwd, sizeof(tmp) - 1);
	if (dosCompat) {
		if (dst[0] == '\\')
			*cp++ = '\\';
		else if ((dst[1] != ':') || (dst[2] == '/'))
			*cp++ = '/';
		else
			*cp++ = '\\';
	} else {
		*cp++ = '/';
	}
	*cp = '\0';
	(void) Strnpcat(cp, src, sizeof(tmp) - (cp - tmp));
	CompressPath(dst, tmp, dsize, dosCompat);
}	/* PathCat */



int
DPathCat(char **const dst0, const char *const cwd, const char *const src, int dosCompat)
{
	char *cp, *dst, *tmp;
	size_t dsize;

	dsize = strlen(cwd) +
		/* pathdelim */ 1 +
		strlen(src) +
		/* NUL byte */ 1 +
		/* spare */ 10;

	dst = calloc(dsize, 1);
	*dst0 = dst;
	if (dst == NULL)
		return (-1);

	if (dosCompat != 0) {
		if ((isalpha((int) cwd[0])) && (cwd[1] == ':')) {
			if ((isalpha((int) src[0])) && (src[1] == ':')) {
				/* A new fully-qualified DOS drive+path was requested. */
				CompressPath(dst, src, dsize, dosCompat);
				return (0);
			} else if (IsValidUNCPath(src)) {
				CompressPath(dst, src, dsize, dosCompat);
				return (0);
			} else if (src[0] == '\\') {
				/* A new fully-qualified DOS path on the same
				 * drive letter was requested.
				 */
				dst[0] = cwd[0];
				dst[1] = ':';
				CompressPath(dst + 2, src, dsize - 2, dosCompat);
				return (0);
			}
		} else if (IsValidUNCPath(src)) {
			/* A new fully-qualified DOS UNC path was requested.
			 * (but no drive letter was present on CWD?)
			 */
			CompressPath(dst, src, dsize, dosCompat);
			return (0);
		} else if ((src[0] == '\\') || ((isalpha((int) src[0])) && (src[1] == ':'))) {
			/* A new fully-qualified DOS path was requested.
			 * (but no drive letter was present on CWD?)
			 */
			CompressPath(dst, src, dsize, dosCompat);
			return (0);
		}
	}
	if (src[0] == '/') {
		/* A new fully-qualified UNIX path was requested. */
		CompressPath(dst, src, dsize, dosCompat);
		return (0);
	}

	tmp = calloc(dsize, 1);
	if (tmp == NULL) {
		free(dst);
		*dst0 = NULL;
		return (-1);
	}

	cp = Strnpcpy(tmp, cwd, dsize - 1);
	if (dosCompat) {
		if (dst[0] == '\\')
			*cp++ = '\\';
		else if ((dst[1] != ':') || (dst[2] == '/'))
			*cp++ = '/';
		else
			*cp++ = '\\';
	} else {
		*cp++ = '/';
	}
	*cp = '\0';
	(void) Strnpcat(cp, src, dsize - (cp - tmp));
	CompressPath(dst, tmp, dsize, dosCompat);
	free(tmp);
	return (0);
}	/* DPathCat */
