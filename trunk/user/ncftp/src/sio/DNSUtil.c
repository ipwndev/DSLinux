#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef STRNCPY
#	define STRNCPY(a,b) strncpy(a, b, sizeof(a)); a[sizeof(a) - 1] = '\0'
#	define Strncpy(a,b,s) strncpy(a, b, s); a[s - 1] = '\0' 
#	define Strncat(a,b,s)\
	{ \
		size_t alen = strlen(a); \
		if (alen < s) { \
			strncpy(a + alen, b, s - alen); \
		} \
		a[s - 1] = '\0'  \
	}
#endif

#if (((defined(MACOSX)) && (MACOSX < 10300)) || (defined(AIX) && (AIX < 430)) || (defined(DIGITAL_UNIX)) || (defined(SOLARIS)) || (defined(SCO)) || (defined(HPUX)))
extern int getdomainname(char *name, gethostname_size_t namelen);
#endif


int
GetHostByName(struct hostent *const hp, const char *const name, char *const hpbuf, size_t hpbufsize)
{
#if defined(HAVE_GETHOSTBYNAME_R) && (defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	struct hostent *h;
	int h_errno_unused = 0;
	memset(hpbuf, 0, hpbufsize);
	h = gethostbyname_r(name, hp, hpbuf, hpbufsize, &h_errno_unused);
	if (h != NULL)
		return (0);
#elif defined(HAVE_GETHOSTBYNAME2_R) && defined(LINUX) && defined(HAVE_ALLOCA)
	char *usehpbuf;
	struct hostent *h;
	int my_h_errno, rc;

	usehpbuf = hpbuf;
	forever {
		errno = 0;
		my_h_errno = 0;
		h = NULL;
		memset(usehpbuf, 0, hpbufsize);
		rc = gethostbyname2_r(name, AF_INET, hp, usehpbuf, hpbufsize, &h, &my_h_errno);
		if ((rc == 0) && (h != NULL))
			return (0);
		if ((rc == ERANGE) || ((rc == -1) && (errno == ERANGE))) {
			hpbufsize *= 2;
			usehpbuf = alloca(hpbufsize);
			if (usehpbuf == NULL) {
				errno = ENOMEM;
				return (-1);
			}
			continue;
		}
		if ((rc == 0) && (my_h_errno != 0))
			errno = ENOENT;
		break;
	}
#elif defined(HAVE_GETHOSTBYNAME_R) && defined(LINUX) && defined(HAVE_ALLOCA)
	char *usehpbuf;
	struct hostent *h;
	int my_h_errno, rc;

	usehpbuf = hpbuf;
	forever {
		errno = 0;
		my_h_errno = 0;
		h = NULL;
		memset(usehpbuf, 0, hpbufsize);
		rc = gethostbyname_r(name, hp, usehpbuf, hpbufsize, &h, &my_h_errno);
		if ((rc == 0) && (h != NULL))
			return (0);
		if ((rc == ERANGE) || ((rc == -1) && (errno == ERANGE))) {
			hpbufsize *= 2;
			usehpbuf = alloca(hpbufsize);
			if (usehpbuf == NULL) {
				errno = ENOMEM;
				return (-1);
			}
			continue;
		}
		if ((rc == 0) && (my_h_errno != 0))
			errno = ENOENT;
		break;
	}
#elif defined(HAVE_GETHOSTBYNAME_R) && defined(AIX)
	struct hostent_data hed;
	memset(hpbuf, 0, hpbufsize);
	memset(&hed, 0, sizeof(hed));
	if (gethostbyname_r(name, hp, &hed) == 0)
		return (0);
#else
	/* Note: gethostbyname is already threadsafe on: HP-UX, Tru64 */
	struct hostent *h;
	h = gethostbyname(name);
	if (h != NULL) {
		memcpy(hp, h, sizeof(struct hostent));
		return (0);
	} else {
		memset(hp, 0, sizeof(struct hostent));
		memset(hpbuf, 0, hpbufsize);
	}
#endif
	return (-1);
}	/* GetHostByName */




int
GetHostByAddr(struct hostent *const hp, void *addr, int asize, int atype, char *const hpbuf, size_t hpbufsize)
{
#if defined(HAVE_GETHOSTBYADDR_R) && (defined(SOLARIS) || defined(IRIX) || defined(BSDOS))
	struct hostent *h;
	int h_errno_unused = 0;
	memset(hpbuf, 0, hpbufsize);
	h = gethostbyaddr_r((gethost_addrptr_t) addr, asize, atype, hp, hpbuf, hpbufsize, &h_errno_unused);
	if (h != NULL)
		return (0);
#elif defined(HAVE_GETHOSTBYADDR_R) && defined(LINUX) && defined(HAVE_ALLOCA)
	char *usehpbuf;
	struct hostent *h;
	int my_h_errno, rc;

	usehpbuf = hpbuf;
	forever {
		errno = 0;
		my_h_errno = 0;
		h = NULL;
		memset(usehpbuf, 0, hpbufsize);
		rc = gethostbyaddr_r((gethost_addrptr_t) addr, asize, atype, hp, usehpbuf, hpbufsize, &h, &my_h_errno);
		if ((rc == 0) && (h != NULL))
			return (0);
		if ((rc == ERANGE) || ((rc == -1) && (errno == ERANGE))) {
			hpbufsize *= 2;
			usehpbuf = alloca(hpbufsize);
			if (usehpbuf == NULL) {
				errno = ENOMEM;
				return (-1);
			}
			continue;
		}
		if ((rc == 0) && (my_h_errno != 0))
			errno = ENOENT;
		break;
	}
#elif defined(HAVE_GETHOSTBYADDR_R) && defined(AIX)
	struct hostent_data hed;
	memset(hpbuf, 0, hpbufsize);
	memset(&hed, 0, sizeof(hed));
	if (gethostbyaddr_r(addr, asize, atype, hp, &hed) == 0)
		return (0);
#else
	/* Note: gethostbyaddr is already threadsafe on: HP-UX, Tru64 */
	struct hostent *h;
	h = gethostbyaddr((gethost_addrptr_t) addr, asize, atype);
	if (h != NULL) {
		memcpy(hp, h, sizeof(struct hostent));
		return (0);
	} else {
		memset(hp, 0, sizeof(struct hostent));
		memset(hpbuf, 0, hpbufsize);
	}
#endif
	return (-1);
}	/* GetHostByAddr */




/* On entry, you should have 'host' be set to a symbolic name (like
 * cse.unl.edu), or set to a numeric address (like 129.93.3.1).
 * If the function fails, it will return NULL, but if the host was
 * a numeric style address, you'll have the ip_address to fall back on.
 */

int
GetHostEntry(struct hostent *const hp, const char *const host, struct in_addr *const ip_address, char *const hpbuf, size_t hpbufsize)
{
	struct in_addr ip;
	int rc = -1;
	
	/* See if the host was given in the dotted IP format, like "36.44.0.2."
	 * If it was, inet_addr will convert that to a 32-bit binary value;
	 * if not, inet_addr will return (-1L).
	 */
	ip.s_addr = inet_addr(host);
	if (ip.s_addr != INADDR_NONE) {
		if (GetHostByAddr(hp, (char *) &ip, (int) sizeof(ip), AF_INET, hpbuf, hpbufsize) == 0) {
			rc = 0;
			if (ip_address != NULL)
				(void) memcpy(&ip_address->s_addr, hp->h_addr_list[0], (size_t) hp->h_length);
		} else if (ip_address != NULL) {
			(void) memcpy(ip_address, &ip, sizeof(struct in_addr));
		}
	} else {
		/* No IP address, so it must be a hostname, like ftp.wustl.edu. */
		if (ip_address != NULL)
			ip_address->s_addr = INADDR_NONE;
		if (GetHostByName(hp, host, hpbuf, hpbufsize) == 0) {
			rc = 0;
			if (ip_address != NULL)
				(void) memcpy(&ip_address->s_addr, hp->h_addr_list[0], (size_t) hp->h_length);
		}
	}
	return (rc);
}	/* GetHostEntry */




static char *
strtokc(char *parsestr, const char *delims, char **context)
{
	char *cp;
	const char *cp2;
	char c, c2;
	char *start;

	if (parsestr == NULL)
		start = *context;
	else
		start = parsestr;

	if ((start == NULL) || (delims == NULL)) {
		*context = NULL;
		return NULL;
	}

	/* Eat leading delimiters. */
	for (cp = start; ; ) {
next1:
		c = *cp++;
		if (c == '\0') {
			/* No more tokens. */
			*context = NULL;
			return (NULL);
		}
		for (cp2 = delims; ; ) {
			c2 = (char) *cp2++;
			if (c2 == '\0') {
				/* This character was not a delimiter.
				 * The token starts here.
				 */
				start = cp - 1;
				goto starttok;
			}
			if (c2 == c) {
				/* This char was a delimiter. */
				/* Skip it, look at next character. */
				goto next1;
			}
		}
		/*NOTREACHED*/
	}

starttok:
	for ( ; ; cp++) {
		c = *cp;
		if (c == '\0') {
			/* Token is finished. */
			*context = cp;
			break;
		}
		for (cp2 = delims; ; ) {
			c2 = (char) *cp2++;
			if (c2 == '\0') {
				/* This character was not a delimiter.
				 * Keep it as part of current token.
				 */
				break;
			}
			if (c2 == c) {
				/* This char was a delimiter. */
				/* End of token. */
				*cp++ = '\0';
				*context = cp;
				return (start);
			}
		}
	}
	return (start);
}	/* strtokc */



#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
int
getdomainname(char *const domain, unsigned int dsize)
{
	HKEY hkey;
	DWORD rc;
	DWORD valSize;

	/* Works for Win NT/2000/XP */
	rc = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
			0,
			KEY_READ,
			&hkey
	);

	if (rc == ERROR_SUCCESS) {
		valSize = (DWORD) (dsize - 1);
		memset(domain, 0, dsize);
		rc = RegQueryValueEx(
			hkey,
			"DhcpDomain",
			NULL,
			NULL,
			(LPBYTE) domain,
			&valSize
		);

		if ((rc == ERROR_SUCCESS) && (domain[0] != '\0')) {
			RegCloseKey(hkey);
			return (0);
		}

		valSize = (DWORD) (dsize - 1);
		memset(domain, 0, dsize);
		rc = RegQueryValueEx(
			hkey,
			"Domain",
			NULL,
			NULL,
			(LPBYTE) domain,
			&valSize
		);

		if ((rc == ERROR_SUCCESS) && (domain[0] != '\0')) {
			RegCloseKey(hkey);
			return (0);
		}

		RegCloseKey(hkey);
	}

	/* Works for Win 9x */
	rc = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			"System\\CurrentControlSet\\Services\\VxD\\MSTCP",
			0,
			KEY_READ,
			&hkey
	);

	if (rc == ERROR_SUCCESS) {
		valSize = (DWORD) (dsize - 1);
		memset(domain, 0, dsize);
		rc = RegQueryValueEx(
			hkey,
			"Domain",
			NULL,
			NULL,
			(LPBYTE) domain,
			&valSize
		);

		if ((rc == ERROR_SUCCESS) && (domain[0] != '\0')) {
			RegCloseKey(hkey);
			return (0);
		}

		RegCloseKey(hkey);
	}

	memset(domain, 0, dsize);
	return (-1);
}	/* getdomainname */
#endif	/* WINDOWS */



/* Makes every effort to return a fully qualified domain name. */
int
GetOurHostName(char *const host, const size_t siz)
{
#ifdef HOSTNAME
	/* You can hardcode in the name if this routine doesn't work
	 * the way you want it to.
	 */
	(void) Strncpy(host, HOSTNAME, siz);
	return (kHostnameHardCoded);		/* Success */
#else
	struct hostent hp;
	struct in_addr ip;
	int result;
	char **curAlias;
	char domain[128];
	char hpbuf[1024];
	char *cp;
	char *dlim, *dcp;
	char *ctext;
	int rc = 0;

	memset(host, 0, siz);
	result = gethostname(host, (gethostname_size_t) siz);
	if ((result < 0) || (host[0] == '\0')) {
		rc = kGethostnameFailed;
		goto done;	/* Failure */
	}

	if (strchr(host, '.') != NULL) {
		/* gethostname returned full name (like "cse.unl.edu"), instead
		 * of just the node name (like "cse").
		 */
		rc = kGethostnameFullyQualified;
		goto done;	/* Success */
	}

	if ((GetHostByName(&hp, host, hpbuf, sizeof(hpbuf)) == 0) && (hp.h_name != NULL) && (hp.h_name[0] != '\0')) {
		/* Maybe the host entry has the full name. */
		cp = strchr((char *) hp.h_name, '.');
		if ((cp != NULL) && (cp[1] != '\0')) {
			/* The 'name' field for the host entry had full name. */
			(void) Strncpy(host, (char *) hp.h_name, siz);
			rc = kGethostbynameFullyQualified;
			goto done;	/* Success */
		}

		/* Make note of the IP address. */
		ip = * ((struct in_addr **) hp.h_addr_list)[0];

		/* Now try the list of aliases, to see if any of those look real. */
		for (curAlias = hp.h_aliases; *curAlias != NULL; curAlias++) {
			cp = strchr(*curAlias, '.');
			if ((cp != NULL) && (cp[1] != '\0')) {
				(void) Strncpy(host, *curAlias, siz);
				rc = kGethostbynameHostAliasFullyQualified;
				goto done;	/* Success */
			}
		}

		/* Use saved IP address to lookup the record by IP address. */
		if (ip.s_addr != INADDR_NONE) {
			if (GetHostByAddr(&hp, (char *) &ip, (int) sizeof(ip), AF_INET, hpbuf, sizeof(hpbuf)) == 0) {
				/* Maybe the host entry has the full name. */
				cp = strchr((char *) hp.h_name, '.');
				if ((cp != NULL) && (cp[1] != '\0')) {
					/* The 'name' field for the host entry had full name. */
					(void) Strncpy(host, (char *) hp.h_name, siz);
					rc = kGethostbyaddrFullyQualified;
					goto done;	/* Success */
				}

				/* Now try the list of aliases, to see if any of those look real. */
				for (curAlias = hp.h_aliases; *curAlias != NULL; curAlias++) {
					cp = strchr(*curAlias, '.');
					if ((cp != NULL) && (cp[1] != '\0')) {
						(void) Strncpy(host, *curAlias, siz);
						rc = kGethostbyaddrHostAliasFullyQualified;
						goto done;	/* Success */
					}
				}
			}
		}
	}

	/* Otherwise, we just have the node name.  See if we can get the
	 * domain name ourselves.
	 */
#ifdef DOMAINNAME
	(void) STRNCPY(domain, DOMAINNAME);
	rc = kDomainnameHardCoded;
#else
	rc = kDomainnameUnknown;
	domain[0] = '\0';

#	if defined(HAVE_RES_INIT) && defined(HAVE__RES_DEFDNAME)
	if (domain[0] == '\0') {
		res_init();
		if ((_res.defdname != NULL) && (_res.defdname[0] != '\0')) {
			STRNCPY(domain, _res.defdname);
			rc = kResInitDomainnameFound;
		}
	}
#	endif	/* HAVE_RES_INIT && HAVE__RES_DEFDNAME */
	
	if (domain[0] == '\0') {
		FILE *fp;
		char line[256];
		char srch[128];
		char *tok;

		fp = fopen("/etc/resolv.conf", "r");
		if (fp != NULL) {
			srch[0] = '\0';
			memset(line, 0, sizeof(line));
			while (fgets(line, sizeof(line) - 1, fp) != NULL) {
				if (!isalpha((int) line[0]))
					continue;	/* Skip comment lines. */
				ctext = NULL;
				tok = strtokc(line, " \t\n\r", &ctext);
				if (tok == NULL)
					continue;	/* Impossible */
				if (strcmp(tok, "domain") == 0) {
					tok = strtokc(NULL, " \t\n\r", &ctext);
					if (tok == NULL)
						continue;	/* syntax error */
					(void) STRNCPY(domain, tok);
					rc = kEtcResolvConfDomainFound;
					break;	/* Done. */
				} else if (strcmp(tok, "search") == 0) {
					tok = strtokc(NULL, " \t\n\r", &ctext);
					if (tok == NULL)
						continue;	/* syntax error */
					(void) STRNCPY(srch, tok);
					/* continue */
				}
			}
			(void) fclose(fp);

			if ((domain[0] == '\0') && (srch[0] != '\0')) {
				(void) STRNCPY(domain, srch);
				rc = kEtcResolvConfSearchFound;
			}
		}
	}

#	if defined(HAVE_GETDOMAINNAME) || \
		((defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__))
	if (domain[0] == '\0') {
		if (getdomainname(domain, (gethostname_size_t) (sizeof(domain) - 1)) != 0) {
			domain[0] = '\0';
		} else if (strchr(domain, '.') == NULL) {
			/* Probably a NIS domain, not a DNS domain name */
			domain[0] = '\0';
		}
	}
#	endif	/* HAVE_GETDOMAINNAME */
#endif	/* DOMAINNAME */

	if (domain[0] != '\0') {
		/* Supposedly, it's legal for a domain name with
		 * a period at the end.
		 */
		cp = domain + strlen(domain) - 1;
		if (*cp == '.')
			*cp = '\0';
		cp = domain;
		dcp = host + strlen(host);
		dlim = host + siz - 1;
		if ((domain[0] != '.') && (dcp < dlim))
			*dcp++ = '.';
		while (*cp) {
			if (dcp < dlim)
				*dcp++ = *cp;
			cp++;
		}
		*dcp = '\0';
	}
done:
	if (rc < 0)
		memset(host, 0, siz);
	if (host[siz - 1] != '\0') {
		/* Hostname (most likely) couldn't fit.
		 * Return failure, but unlike other
		 * failures, leave what we had before
		 * it was truncated.
		 */
		rc = kFullyQualifiedHostNameTooLongForBuffer;
	}
	return (rc);	/* Success */
#endif	/* !HOSTNAME */
}	/* GetOurHostName */
