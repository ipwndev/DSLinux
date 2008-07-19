#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef INADDR_ANY
#	define INADDR_ANY              ((unsigned long int) 0x00000000)
#endif

#ifndef INADDR_NONE
#	define INADDR_NONE             ((unsigned long int) 0xffffffff)
#endif

/* Linux libc 5.3.x has a bug that causes isalnum() to not work! */
#define ISALNUM(c) ( (((c) >= 'A') && ((c) <= 'Z')) || (((c) >= 'a') && ((c) <= 'z')) || (((c) >= '0') && ((c) <= '9')) )

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	pragma warning(disable : 4711)	// warning: function 'ServiceNameToPortNumber' selected for automatic inline expansion
#endif
unsigned int
ServiceNameToPortNumber(const char *const s, const int proto)
{
	char str[64];
	char *cp;
#if defined(HAVE_GETSERVBYNAME_R) && (defined(AIX) || defined(TRU64UNIX) || defined(DIGITAL_UNIX))
	struct servent *sp;
#elif defined(HAVE_GETSERVBYNAME_R) && (defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	struct servent se, *sp;
	char spbuf[256];
#else
	struct servent *sp;
#endif
	strncpy(str, s, sizeof(str) - 1);
	str[sizeof(str) - 1] = '\0';
	cp = str;
	if (isdigit((int) *cp)) {
		while (isdigit((int) *cp))
			cp++;
		*cp = '\0';
		return (atoi(str));
	}
	for (;; cp++) {
		if ((*cp == '\0')
			|| ((!ISALNUM(*cp)) && (*cp != '-') && (*cp != '_')))
				break;
	}
	*cp = '\0';

	sp = NULL;
#if defined(HAVE_GETSERVBYNAME_R) && (defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		memset(spbuf, 0, sizeof(spbuf));
		sp = getservbyname_r(str, "tcp", &se, spbuf, sizeof(spbuf));
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		memset(spbuf, 0, sizeof(spbuf));
		sp = getservbyname_r(str, "udp", &se, spbuf, sizeof(spbuf));
	}
#elif defined(HAVE_GETSERVBYNAME_R) && defined(LINUX)
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		memset(spbuf, 0, sizeof(spbuf));
		if (getservbyname_r(str, "tcp", &se, spbuf, sizeof(spbuf), &sp) != 0)
			sp = NULL;
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		memset(spbuf, 0, sizeof(spbuf));
		if (getservbyname_r(str, "udp", &se, spbuf, sizeof(spbuf), &sp) != 0)
			sp = NULL;
	}
#elif defined(HAVE_GETSERVBYNAME_R) && defined(AIX)
	{
		struct servent_data sed;
		if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
			memset(&sed, 0, sizeof(sed));
			if (getservbyname_r(str, "tcp", sp, &sed) != 0)
				sp = NULL;
		}
		if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
			memset(&sed, 0, sizeof(sed));
			if (getservbyname_r(str, "udp", sp, &sed) != 0)
				sp = NULL;
		}
	}
#else
	/* Note: getservbyname is already threadsafe on: HP-UX, Tru64 */
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		sp = getservbyname(str, "tcp");
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		sp = getservbyname(str, "udp");
	}
#endif

	if (sp != NULL) {
		return ((unsigned int) ntohs((unsigned short) sp->s_port));
	}
	return (0);	/* error */
}	/* ServiceNameToPortNumber */




int
ServicePortNumberToName(unsigned short port, char *const dst, const size_t dsize, const int proto)
{
#if defined(HAVE_GETSERVBYNAME_R) && (defined(AIX) || defined(TRU64UNIX) || defined(DIGITAL_UNIX))
	struct servent *sp;
#elif defined(HAVE_GETSERVBYNAME_R) && (defined(LINUX) || defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	struct servent se, *sp;
	char spbuf[256];
#else
	struct servent *sp;
#endif

	sp = NULL;
#if defined(HAVE_GETSERVBYPORT_R) && (defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		memset(spbuf, 0, sizeof(spbuf));
		sp = getservbyport_r((int) htons(port), "tcp", &se, spbuf, sizeof(spbuf));
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		memset(spbuf, 0, sizeof(spbuf));
		sp = getservbyport_r((int) htons(port), "udp", &se, spbuf, sizeof(spbuf));
	}
#elif defined(HAVE_GETSERVBYPORT_R) && defined(LINUX)
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		memset(spbuf, 0, sizeof(spbuf));
		if (getservbyport_r((int) htons(port), "tcp", &se, spbuf, sizeof(spbuf), &sp) != 0)
			sp = NULL;
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		memset(spbuf, 0, sizeof(spbuf));
		if (getservbyport_r((int) htons(port), "udp", &se, spbuf, sizeof(spbuf), &sp) != 0)
			sp = NULL;
	}
#elif defined(HAVE_GETSERVBYPORT_R) && defined(AIX)
	{
		struct servent_data sed;
		if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
			memset(&sed, 0, sizeof(sed));
			if (getservbyport_r((int) htons(port), "tcp", sp, &sed) != 0)
				sp = NULL;
		}
		if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
			memset(&sed, 0, sizeof(sed));
			if (getservbyport_r((int) htons(port), "udp", sp, &sed) != 0)
				sp = NULL;
		}
	}
#else
	/* Note: getservbyport is already threadsafe on: HP-UX, Tru64 */
	if ((sp == NULL) && ((proto == 0) || (proto == 't'))) {
		sp = getservbyport((int) htons(port), "tcp");
	}
	if ((sp == NULL) && ((proto == 0) || (proto == 'u'))) {
		sp = getservbyport((int) htons(port), "ucp");
	}
#endif

	if (sp != NULL) {
		strncpy(dst, sp->s_name, dsize);
		dst[dsize - 1] = '\0';
		return (1);
	}

#ifdef HAVE_SNPRINTF
	snprintf(dst, dsize,
#else
	sprintf(dst,
#endif
		"%u", (unsigned int) port);

	return (0);	/* error */
}	/* ServicePortNumberToName */




void
InetNtoA(char *dst, struct in_addr *ia, size_t siz)
{
#if defined(HAVE_INET_NTOP) && !defined(MACOSX)
	/* Mostly to workaround bug in IRIX 6.5's inet_ntoa */
	/* For OS X, don't use inet_ntop yet since it was just introduced
	 * for 10.2.
	 */
	memset(dst, 0, siz);
	(void) inet_ntop(AF_INET, ia, dst, siz - 1);
#else
	char *cp;
	memset(dst, 0, siz);
	cp = inet_ntoa(*ia);
	if ((cp != (char *) 0) && (cp != (char *) -1) && (cp[0] != '\0')) {
		(void) strncpy(dst, cp, siz - 1);
	}
#endif
}	/* InetNtoA */




int
AddrStrToAddr(const char * const s, struct sockaddr_in * const sa, const int defaultport)
{
	char portstr[128];
	unsigned int ipnum;
	unsigned int port;
	struct hostent *hp;
	char *hostcp, *atsign, *colon, *cp, *p2;

	memset(sa, 0, sizeof(struct sockaddr_in));
	strncpy(portstr, s, sizeof(portstr));
	portstr[sizeof(portstr) - 1] = '\0';

	if ((colon = strchr(portstr, ':')) != NULL) {
		/* Does it look like a URL?  http://host ? */
		if ((colon[1] == '/') && (colon[2] == '/')) {
			*colon = '\0';
			port = 0;
			hostcp = colon + 3;
			for (cp = hostcp; *cp != '\0'; cp++) {
				if ((!ISALNUM(*cp)) && (*cp != '.')) {
					/* http://host:port */
					if ((*cp == ':') && (isdigit((int) cp[1]))) {
						*cp++ = '\0';
						p2 = cp;
						while (isdigit((int) *cp))
							cp++;
						*cp = '\0';
						port = atoi(p2);
					}
					*cp = '\0';
					break;
				}
			}
			if (port == 0)
				port = ServiceNameToPortNumber(portstr, 0);
		} else {
			/* Look for host.name.domain:port */
			*colon = '\0';
			hostcp = portstr;
			port = (unsigned int) atoi(colon + 1);
		}
	} else if ((atsign = strchr(portstr, '@')) != NULL) {
		/* Look for port@host.name.domain */
		*atsign = '\0';
		hostcp = atsign + 1;
		port = (unsigned int) atoi(portstr);
	} else if (defaultport > 0) {
		/* Have just host.name.domain, use that w/ default port. */
		port = (unsigned int) defaultport;
		hostcp = portstr;
	} else {
		/* If defaultport <= 0, they must supply a port number
		 * in the host/port string.
		 */
		errno = EADDRNOTAVAIL;
		return (kAddrStrToAddrMiscErr);
	}

	sa->sin_port = htons((short) port);

	ipnum = inet_addr(hostcp);
	if (ipnum != INADDR_NONE) {
		sa->sin_family = AF_INET;
		sa->sin_addr.s_addr = ipnum;
	} else {
		errno = 0;
		hp = gethostbyname(hostcp);
		if (hp == NULL) {
			if (errno == 0)
				errno = ENOENT;
			return (kAddrStrToAddrBadHost);
		}
		sa->sin_family = hp->h_addrtype;
		memcpy(&sa->sin_addr.s_addr, hp->h_addr_list[0],
			(size_t) hp->h_length);
	}
	return (0);
}	/* AddrStrToAddr */



char *
AddrToAddrStr(char *const dst, size_t dsize, struct sockaddr_in * const saddrp, int dns, const char *fmt)
{
	char addrName[128];
	char *addrNamePtr;
	struct hostent *hp;
	char str[128];
	char s_name[64];
	char *dlim, *dp;
	const char *cp;

	if (dsize == 0)
		return NULL;
	memset(dst, 0, dsize);

	addrNamePtr = NULL;
	if (dns == 0) {
		InetNtoA(addrName, &saddrp->sin_addr, sizeof(addrName));
		addrNamePtr = addrName;
	} else {
		hp = gethostbyaddr((gethost_addrptr_t) &saddrp->sin_addr, sizeof(struct in_addr), AF_INET);
		if ((hp != NULL) && (hp->h_name != NULL) && (hp->h_name[0] != '\0')) {
			addrNamePtr = hp->h_name;
		} else {
			InetNtoA(addrName, &saddrp->sin_addr, sizeof(addrName));
			addrNamePtr = addrName;
		}
	}
	if (fmt == NULL)
		fmt = "%h:%p";
	for (dp = dst, dlim = dp + dsize - 1; ; fmt++) {
		if (*fmt == '\0') {
			break;	/* done */
		} else if (*fmt == '%') {
			fmt++;
			if (*fmt == '%') {
				if (dp < dlim)
					*dp++ = '%';
			} else if (*fmt == 'p') {
				sprintf(str, "%u", (unsigned int) ntohs(saddrp->sin_port));
				for (cp = str; *cp != '\0'; cp++)
					if (dp < dlim)
						*dp++ = *cp;
				*dp = '\0';
			} else if (*fmt == 'h') {
				if (addrNamePtr != NULL) {
					cp = addrNamePtr;
				} else {
					cp = "unknown";
				}
				for ( ; *cp != '\0'; cp++)
					if (dp < dlim)
						*dp++ = *cp;
				*dp = '\0';
			} else if (*fmt == 's') {
				cp = s_name;
				(void) ServicePortNumberToName(ntohs(saddrp->sin_port), s_name, sizeof(s_name), 0);
				for ( ; *cp != '\0'; cp++)
					if (dp < dlim)
						*dp++ = *cp;
				/* endservent(); */
				*dp = '\0';
			} else if ((*fmt == 't') || (*fmt == 'u')) {
				cp = s_name;
				(void) ServicePortNumberToName(ntohs(saddrp->sin_port), s_name, sizeof(s_name), (int) *fmt);
				for ( ; *cp != '\0'; cp++)
					if (dp < dlim)
						*dp++ = *cp;
				/* endservent(); */
				*dp = '\0';
			} else if (*fmt == '\0') {
				break;
			} else {
				if (dp < dlim)
					*dp++ = *fmt;
			}
		} else if (dp < dlim) {
			*dp++ = *fmt;
		}
	}
	*dp = '\0';
	return (dst);
}	/* AddrToAddrStr */
