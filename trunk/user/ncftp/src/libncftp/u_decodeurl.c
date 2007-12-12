/* u_decodeurl.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

void
URLCopyToken(char *dst, size_t dsize, const char *src, size_t howmuch)
{
	char *dlim;
	const char *slim;
	unsigned int hc;
	int c;
	char h[4];

	dlim = dst + dsize - 1;		/* leave room for \0 */
	slim = src + howmuch;
	while (src < slim) {
		c = *src++;
		if (c == '\0')
			break;
		if (c == '%') {
			/* hex representation */
			if (src < slim + 2) {
				h[0] = *src++;
				h[1] = *src++;
				h[2] = '\0';
				hc = 0xeeff;
				if ((sscanf(h, "%x", &hc) >= 0) && (hc != 0xeeff)) {
					if ((hc == 0) || (hc == '\n') || (hc == '\r') || (hc == /* '\a' */ 0x07) || (hc == /* '\b' */ 0x08) || (hc == /* '\v' */ 0x0B) || (hc == /* '\f' */ 0x0C)) {
						/* Do not allow these in URLs. */
						break;
					}
					if (dst < dlim) {
						*(unsigned char *)dst = (unsigned char) hc;
						dst++;
					}
				}
			} else {
				break;
			}
		} else {
			*dst++ = (char) c;
		}
	}
	*dst = '\0';
}	/* URLCopyToken */




int
FTPDecodeURL(
	const FTPCIPtr cip,	/* area pointed to may be modified */
	char *const url,	/* always modified */
	FTPLineListPtr cdlist,	/* always modified */
	char *const fn,		/* always modified */
	const size_t fnsize,
	int *const xtype,	/* optional; may be modified */
	int *const wantnlst	/* optional; always modified */
)
{
	char *cp;
	char *hstart, *hend;
	char *h2start;
	char *at1;
	char portstr[32];
	int port;
	int sc;
	char *lastslash;
	char *parsestr;
	char *tok;
	char subdir[128];
	char *semi;
	char *ctext;

	InitLineList(cdlist);
	*fn = '\0';
	if (wantnlst != NULL)
		*wantnlst = 0;
	if (xtype != NULL)
		*xtype = kTypeBinary;

	cp = NULL;	/* shut up warnings */
#ifdef HAVE_STRCASECMP
	if (strncasecmp(url, "<URL:ftp://", 11) == 0) {
		cp = url + strlen(url) - 1;
		if (*cp != '>')
			return (kMalformedURL);	/* missing closing > */
		*cp = '\0';
		cp = url + 11;
	} else if (strncasecmp(url, "ftp://", 6) == 0) {
		cp = url + 6;
	} else {
		return (-1);		/* not a RFC 1738 URL */
	}
#else	/* HAVE_STRCASECMP */
	if (strncmp(url, "<URL:ftp://", 11) == 0) {
		cp = url + strlen(url) - 1;
		if (*cp != '>')
			return (kMalformedURL);	/* missing closing > */
		*cp = '\0';
		cp = url + 11;
	} else if (strncmp(url, "ftp://", 6) == 0) {
		cp = url + 6;
	} else {
		return (-1);		/* not a RFC 1738 URL */
	}
#endif	/* HAVE_STRCASECMP */

	/* //<user>:<password>@<host>:<port>/<url-path> */

	at1 = NULL;
	for (hstart = cp; ; cp++) {
		if (*cp == '@') {
			if (at1 == NULL)
				at1 = cp;
			else 
				return (kMalformedURL);
		} else if ((*cp == '\0') || (*cp == '/')) {
			hend = cp;
			break;
		}
	}

	sc = *hend;
	*hend = '\0';
	if (at1 == NULL) {
		/* no user or password */
		h2start = hstart;
	} else {
		*at1 = '\0';
		cp = strchr(hstart, ':');
		if (cp == NULL) {
			/* whole thing is the user name then */
			URLCopyToken(cip->user, sizeof(cip->user), hstart, (size_t) (at1 - hstart));
		} else if (strchr(cp + 1, ':') != NULL) {
			/* Too many colons */
			return (kMalformedURL);
		} else {
			URLCopyToken(cip->user, sizeof(cip->user), hstart, (size_t) (cp - hstart));
			URLCopyToken(cip->pass, sizeof(cip->pass), cp + 1, (size_t) (at1 - (cp + 1)));
		}
		*at1 = '@';
		h2start = at1 + 1;
	}

	cp = strchr(h2start, ':');
	if (cp == NULL) {
		/* whole thing is the host then */
		URLCopyToken(cip->host, sizeof(cip->host), h2start, (size_t) (hend - h2start));
	} else if (strchr(cp + 1, ':') != NULL) {
		/* Too many colons */
		return (kMalformedURL);
	} else {
		URLCopyToken(cip->host, sizeof(cip->host), h2start, (size_t) (cp - h2start));
		URLCopyToken(portstr, sizeof(portstr), cp + 1, (size_t) (hend - (cp + 1)));
		port = atoi(portstr);
		if (port > 0)
			cip->port = port;
	}

	*hend = (char) sc;
	if ((*hend == '\0') || ((*hend == '/') && (hend[1] == '\0'))) {
		/* no path, okay */
		return (0);
	}

	lastslash = strrchr(hend, '/');
	if (lastslash == NULL) {
		/* no path, okay */
		return (0);
	}	
	*lastslash = '\0';

	if ((semi = strchr(lastslash + 1, ';')) != NULL) {
#ifdef HAVE_STRCASECMP
		if (strcasecmp(semi, ";type=i") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeBinary;
		} else if (strcasecmp(semi, ";type=a") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeAscii;
		} else if (strcasecmp(semi, ";type=b") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeBinary;
		} else if (strcasecmp(semi, ";type=d") == 0) {
			if (wantnlst != NULL) {
				*semi++ = '\0';
				*wantnlst = 1;
				if (xtype != NULL)
					*xtype = kTypeAscii;
			} else {
				/* You didn't want these. */
				return (kMalformedURL);
			}
		}
#else	/* HAVE_STRCASECMP */
		if (strcmp(semi, ";type=i") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeBinary;
		} else if (strcmp(semi, ";type=a") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeAscii;
		} else if (strcmp(semi, ";type=b") == 0) {
			*semi++ = '\0';
			if (xtype != NULL)
				*xtype = kTypeBinary;
		} else if (strcmp(semi, ";type=d") == 0) {
			if (wantnlst != NULL) {
				*semi++ = '\0';
				*wantnlst = 1;
				if (xtype != NULL)
					*xtype = kTypeAscii;
			} else {
				/* You didn't want these. */
				return (kMalformedURL);
			}
		}
#endif	/* HAVE_STRCASECMP */
	}
	URLCopyToken(fn, fnsize, lastslash + 1, strlen(lastslash + 1));
	for (parsestr = hend, ctext = NULL; (tok = strtokc(parsestr, "/", &ctext)) != NULL; parsestr = NULL) {
		URLCopyToken(subdir, sizeof(subdir), tok, strlen(tok));
		(void) AddLine(cdlist, subdir);
	}
	*lastslash = '/';
	return (kNoErr);
}	/* FTPDecodeURL */
