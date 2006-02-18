/*
 *  Copyright (C) 2002 Nigel Horne <njh@bandsman.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
static	char	const	rcsid[] = "$Id$";

#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#ifndef	CL_DEBUG
#define	NDEBUG	/* map CLAMAV debug onto standard */
#endif

#ifdef CL_THREAD_SAFE
#ifndef	_REENTRANT
#define	_REENTRANT	/* for Solaris 2.8 */
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <clamav.h>
#include <dirent.h>
#include <limits.h>

#if defined(HAVE_READDIR_R_3) || defined(HAVE_READDIR_R_2)
#include <stddef.h>
#endif

#ifdef	CL_THREAD_SAFE
#include <pthread.h>
#endif

#include "table.h"
#include "mbox.h"
#include "blob.h"
#include "line.h"
#include "text.h"
#include "message.h"
#include "others.h"
#include "defaults.h"
#include "str.h"
#include "filetypes.h"

#ifdef	CL_DEBUG
#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 1
#define HAVE_BACKTRACE
#endif
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#include <signal.h>
#include <syslog.h>

static	void	sigsegv(int sig);
static	void	print_trace(int use_syslog);
#endif

#if	defined(NO_STRTOK_R) || !defined(CL_THREAD_SAFE)
#undef strtok_r
#undef __strtok_r
#define strtok_r(a,b,c)	strtok(a,b)
#endif

/* required for AIX and Tru64 */
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

typedef enum	{ FALSE = 0, TRUE = 1 } bool;

#define	SAVE_TO_DISC	/* multipart/message are saved in a temporary file */

/*
 * Code does exist to run FOLLORURLS on systems without libcurl, however that
 * is not recommended so it is not compiled by default
 *
 * On Solaris, when using the GNU C compiler, the clamAV build system uses the
 * Sun supplied ld instead of the GNU ld causing an error. Therefore you cannot
 * use WITH_CURL on Solaris with gcc, you must configure with
 * "--without-libcurl". I don't know if it works with Sun's own compiler
 *
 * Fails to link on Solaris 10 with this error:
 *      Undefined                       first referenced
 *  symbol                             in file
 *  __floatdidf                         /opt/sfw/lib/libcurl.s
 */
#if	C_SOLARIS && __GNUC__
#undef	WITH_CURL
#endif

#ifdef	WITH_CURL
#define	FOLLOWURLS	5	/*
				 * Maximum number of URLs scanned in a message
				 * part. Helps to find Dialier.gen-45. If
				 * not defined, don't check any URLs
				 */
#endif

#ifdef	FOLLOWURLS

#include "htmlnorm.h"

#ifdef	WITH_CURL	/* Set in configure */
/*
 * To build with WITH_CURL:
 * LDFLAGS=`curl-config --libs` ./configure ...
 */
#include <curl/curl.h>

/*
 * Needs curl >= 7.11 (I've heard that 7.9 can cause crashes and 7.10 is
 * untested)
 */
#if	(LIBCURL_VERSION_MAJOR < 7)
#undef	WITH_CURL	/* also undef FOLLOWURLS? */
#endif

#if	(LIBCURL_VERSION_MAJOR == 7) && (LIBCURL_VERSION_MINOR < 10)
#undef	WITH_CURL	/* also undef FOLLOWURLS? */
#endif

#endif	/*WITH_CURL*/

#else	/*!FOLLOWURLS*/
#undef	WITH_CURL
#endif	/*FOLLOWURLS*/

/*
 * Define this to handle messages covered by section 7.3.2 of RFC1341.
 *	This is experimental code so it is up to YOU to (1) ensure it's secure
 * (2) periodically trim the directory of old files
 *
 * If you use the load balancing feature of clamav-milter to run clamd on
 * more than one machine you must make sure that .../partial is on a shared
 * network filesystem
 */
#define	PARTIAL_DIR

/*#define	NEW_WORLD*/

static	int	cli_parse_mbox(const char *dir, int desc, unsigned int options);
static	message	*parseEmailFile(FILE *fin, const table_t *rfc821Table, const char *firstLine, const char *dir);
static	message	*parseEmailHeaders(const message *m, const table_t *rfc821Table);
static	int	parseEmailHeader(message *m, const char *line, const table_t *rfc821Table);
static	int	parseEmailBody(message *messageIn, text *textIn, const char *dir, const table_t *rfc821Table, const table_t *subtypeTable, unsigned int options);
static	int	boundaryStart(const char *line, const char *boundary);
static	int	endOfMessage(const char *line, const char *boundary);
static	int	initialiseTables(table_t **rfc821Table, table_t **subtypeTable);
static	int	getTextPart(message *const messages[], size_t size);
static	size_t	strip(char *buf, int len);
static	bool	continuationMarker(const char *line);
static	int	parseMimeHeader(message *m, const char *cmd, const table_t *rfc821Table, const char *arg);
static	void	saveTextPart(message *m, const char *dir);
static	char	*rfc2047(const char *in);
static	char	*rfc822comments(const char *in, char *out);
#ifdef	PARTIAL_DIR
static	int	rfc1341(message *m, const char *dir);
#endif
static	bool	usefulHeader(int commandNumber, const char *cmd);
static        int     uufasttrack(message *m, const char *firstline, const char *dir, FILE *fin);
static	char	*getline_from_mbox(char *buffer, size_t len, FILE *fin);
#ifdef	NEW_WORLD
static	const	char	*cli_pmemstr(const char *haystack, size_t hs, const char *needle, size_t ns);
#endif

static	void	checkURLs(message *m, const char *dir);
#ifdef	WITH_CURL
struct arg {
	const char *url;
	const char *dir;
	char *filename;
};
#ifdef	CL_THREAD_SAFE
static	void	*getURL(void *a);
#else
static	void	*getURL(struct arg *arg);
#endif
#endif

/* Maximum line length according to RFC821 */
#define	LINE_LENGTH	1000

/* Hashcodes for our hash tables */
#define	CONTENT_TYPE			1
#define	CONTENT_TRANSFER_ENCODING	2
#define	CONTENT_DISPOSITION		3

/* Mime sub types */
#define	PLAIN		1
#define	ENRICHED	2
#define	HTML		3
#define	RICHTEXT	4
#define	MIXED		5
#define	ALTERNATIVE	6
#define	DIGEST		7
#define	SIGNED		8
#define	PARALLEL	9
#define	RELATED		10	/* RFC2387 */
#define	REPORT		11	/* RFC1892 */
#define	APPLEDOUBLE	12	/* Handling of this in only noddy for now */
#define	FAX		MIXED	/*
				 * RFC3458
				 * Drafts stated to treat is as mixed if it is
				 * not known.  This disappeared in the final
				 * version (except when talking about
				 * voice-message), but it is good enough for us
				 * since we do no validation of coversheet
				 * presence etc. (which also has disappeared
				 * in the final version)
				 */
#define	ENCRYPTED	13	/*
				 * e.g. RFC2015
				 * Content-Type: multipart/encrypted;
				 * boundary="nextPart1383049.XCRrrar2yq";
				 * protocol="application/pgp-encrypted"
				 */
#define	X_BFILE		RELATED	/*
				 * BeOS, expert two parts: the file and it's
				 * attributes. The attributes part comes as
				 *	Content-Type: application/x-be_attribute
				 *		name="foo"
				 * I can't find where it is defined, any
				 * pointers would be appreciated. For now
				 * we treat it as multipart/related
				 */
#define	KNOWBOT		14	/* Unknown and undocumented format? */

static	const	struct tableinit {
	const	char	*key;
	int	value;
} rfc821headers[] = {
	/* TODO: make these regular expressions */
	{	"Content-Type",			CONTENT_TYPE		},
	{	"Content-Transfer-Encoding",	CONTENT_TRANSFER_ENCODING	},
	{	"Content-Disposition",		CONTENT_DISPOSITION	},
	{	NULL,				0			}
}, mimeSubtypes[] = {	/* see RFC2045 */
		/* subtypes of Text */
	{	"plain",	PLAIN		},
	{	"enriched",	ENRICHED	},
	{	"html",		HTML		},
	{	"richtext",	RICHTEXT	},
		/* subtypes of Multipart */
	{	"mixed",	MIXED		},
	{	"alternative",	ALTERNATIVE	},
	{	"digest",	DIGEST		},
	{	"signed",	SIGNED		},
	{	"parallel",	PARALLEL	},
	{	"related",	RELATED		},
	{	"report",	REPORT		},
	{	"appledouble",	APPLEDOUBLE	},
	{	"fax-message",	FAX		},
	{	"encrypted",	ENCRYPTED	},
	{	"x-bfile",	X_BFILE		},	/* BeOS */
	{	"knowbot",		KNOWBOT		},	/* ??? */
	{	"knowbot-metadata",	KNOWBOT		},	/* ??? */
	{	"knowbot-code",		KNOWBOT		},	/* ??? */
	{	"knowbot-state",	KNOWBOT		},	/* ??? */
	{	NULL,		0		}
};

#ifdef	CL_THREAD_SAFE
static	pthread_mutex_t	tables_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifndef	O_BINARY
#define	O_BINARY	0
#endif

#ifdef	NEW_WORLD

#if HAVE_MMAP
#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#else /* HAVE_SYS_MMAN_H */
#undef HAVE_MMAP
#endif
#endif

struct scanlist {
	char *start;
	size_t size;
	encoding_type decoder;	/* only BASE64 and QUOTEDPRINTABLE for now */
	struct scanlist *next;
};

/*
 * This could be the future. Instead of parsing and decoding it just decodes.
 *
 * USE IT AT YOUR PERIL, a large number of viruses are not detected with this
 * method, possibly because the decoded files must be exact and not have
 * extra data at the start or end, which this code will produce.
 *
 * Currently only supports base64 and quoted-printable
 *
 * You may also see a lot of warnings. For the moment it falls back to old
 *	world mode if it doesn't know what to do - that'll be removed.
 * The code is untidy...
 *
 * FIXME: Some mailbox scans are slower with this method. I suspect that it's
 * because the scan can proceed to the end of the file rather than the end
 * of the attachment which can mean than later emails are scanned many times
 *
 * TODO: Also all those pmemstr()s are slow, so we need to reduce the number
 *	and size of data scanned each time, and we fall through to
 *	cli_parse_mbox() too often
 */
int
cli_mbox(const char *dir, int desc, unsigned int options)
{
	char *start, *ptr, *line, *p, *q;
	const char *last;
	size_t size, s;
	struct stat statb;
	message *m;
	fileblob *fb;
	int ret = 0;
	int wasAlloced;
	struct scanlist *scanlist, *scanelem;

	if(dir == NULL) {
		cli_warnmsg("cli_mbox called with NULL dir\n");
		return CL_ENULLARG;
	}
	if(fstat(desc, &statb) < 0)
		return CL_EOPEN;

	size = statb.st_size;

	if(size == 0)
		return CL_CLEAN;

	if(size > 10*1024*1024)
		return cli_parse_mbox(dir, desc, options);	/* should be StreamMaxLength, I guess */

	cli_warnmsg("NEW_WORLD is new code - use at your own risk.\n");

	start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, desc, 0);
	if(start == MAP_FAILED)
		return CL_EMEM;

	cli_dbgmsg("mmap'ed mbox\n");

	/* last points to the last *valid* address in the array */
	last = &start[size - 1];

	ptr = cli_malloc(size);
	if(ptr) {
		wasAlloced = 1;
		memcpy(ptr, start, size);
		munmap(start, size);
		start = ptr;
		last = &start[size - 1];
	} else
		wasAlloced = 0;

	/*
	 * Would be nice to have a case insensitive cli_memstr()
	 */
	scanelem = scanlist = NULL;
	q = start;
	s = size;
	while((p = (char *)cli_pmemstr(q, s, "base64", 6)) != NULL) {
		cli_dbgmsg("Found base64\n");
		if(scanelem) {
			scanelem->next = cli_malloc(sizeof(struct scanlist));
			scanelem = scanelem->next;
		} else
			scanlist = scanelem = cli_malloc(sizeof(struct scanlist));
		scanelem->next = NULL;
		scanelem->decoder = BASE64;
		s -= (p - q) + 6;
		q = scanelem->start = &p[6];
		if(((p = (char *)cli_pmemstr(q, s, "\nFrom ", 6)) != NULL) ||
		   ((p = (char *)cli_pmemstr(q, s, "base64", 6)) != NULL) ||
		   ((p = (char *)cli_pmemstr(q, s, "quoted-printable", 16)) != NULL)) {
			scanelem->size = (size_t)(p - q);
			q = p;
			s -= scanelem->size;
		} else {
			scanelem->size = (size_t)(last - scanelem->start) + 1;
			break;
		}
		cli_dbgmsg("base64: last %u q %u s %u\n", (unsigned int)last, (unsigned int)q, s);
		assert(scanelem->size <= size);
		assert(&q[s - 1] <= last);
	}
	q = start;
	s = size;
	while((p = (char *)cli_pmemstr(q, s, "quoted-printable", 16)) != NULL) {
		if(p != q)
			switch(p[-1]) {
				case ' ':
				case ':':
				case '=':	/* wrong but allow it */
					break;
				default:
					s -= (p - q) + 16;
					q = &p[16];
					cli_dbgmsg("Ignore quoted-printable false positive\n");
					cli_dbgmsg("s = %u\n", s);
					continue;	/* false positive */
			}

		cli_dbgmsg("Found quoted-printable\n");
		if(scanelem) {
			scanelem->next = cli_malloc(sizeof(struct scanlist));
			scanelem = scanelem->next;
		} else
			scanlist = scanelem = cli_malloc(sizeof(struct scanlist));
		scanelem->next = NULL;
		scanelem->decoder = QUOTEDPRINTABLE;
		s -= (p - q) + 16;
		q = scanelem->start = &p[16];
		cli_dbgmsg("qp: last %u q %u s %u\n", (unsigned int)last, (unsigned int)q, s);
		if(((p = (char *)cli_pmemstr(q, s, "\nFrom ", 6)) != NULL) ||
		   ((p = (char *)cli_pmemstr(q, s, "quoted-printable", 16)) != NULL) ||
		   ((p = (char *)cli_pmemstr(q, s, "base64", 6)) != NULL)) {
			scanelem->size = (size_t)(p - q);
			q = p;
			s -= scanelem->size;
			cli_dbgmsg("qp: scanelem->size = %u\n", scanelem->size);
		} else {
			scanelem->size = (size_t)(last - scanelem->start) + 1;
			break;
		}
		assert(scanelem->size <= size);
		assert(&q[s - 1] <= last);
	}

	if(scanlist == NULL) {
		const struct tableinit *tableinit;
		bool anyHeadersFound = FALSE;
		bool hasuuencode = FALSE;

		/* FIXME: message: There could of course be no decoder needed... */
		for(tableinit = rfc821headers; tableinit->key; tableinit++)
			if(cli_pmemstr(start, size, tableinit->key, strlen(tableinit->key))) {
				anyHeadersFound = TRUE;
				break;
			}

		if((!anyHeadersFound) && cli_pmemstr(start, size, "\nbegin ", 7))
			/* uuencoded part */
			hasuuencode = TRUE;

		if(wasAlloced)
			free(start);
		else
			munmap(start, size);

		if(anyHeadersFound || hasuuencode) {
			/* TODO: reduce the number of falls through here */
			cli_warnmsg("cli_mbox: uuencode or unknown encoder\n");
			return cli_parse_mbox(dir, desc, options);
		}

		cli_warnmsg("cli_mbox: I believe it's plain text which must be clean\n");
		return CL_CLEAN;
	}

	for(scanelem = scanlist; scanelem; scanelem = scanelem->next) {
		if(scanelem->decoder == BASE64) {
			char *b64start = scanelem->start;
			long b64size = scanelem->size;

			cli_dbgmsg("b64size = %lu\n", b64size);
			while(*b64start != '\n') {
				b64start++;
				b64size--;
			}
			/*
			 * Look for the end of the headers
			 */
			while(b64start < last) {
				if(*b64start == ';') {
					b64start++;
					b64size--;
				} else if(*b64start == '\n') {
					b64start++;
					b64size--;
					if((*b64start == '\n') || (*b64start == '\r')) {
						b64start++;
						b64size--;
						break;
					}
				}
				b64start++;
				b64size--;
			}

			if(b64size > 0L)
				while((!isalnum(*b64start)) && (*b64start != '/')) {
					if(b64size-- == 0L)
						break;
					b64start++;
				}

			if(b64size > 0L) {
				cli_dbgmsg("cli_mbox: decoding %ld base64 bytes\n", b64size);

				line = NULL;

				m = messageCreate();
				if(m == NULL)
					return CL_EMEM;
				messageSetEncoding(m, "base64");

				do {
					int length = 0;

					/*printf("%ld: ", b64size); fflush(stdout);*/

					for(ptr = b64start; b64size && (*ptr != '\n') && (*ptr != '\r'); ptr++) {
						length++;
						--b64size;
					}

					/*printf("%d: ", length); fflush(stdout);*/

					line = cli_realloc(line, length + 1);

					memcpy(line, b64start, length);
					line[length] = '\0';

					/*puts(line);*/

					if(messageAddStr(m, line) < 0)
						break;

					if((b64size > 0) && (*ptr == '\r')) {
						ptr++;
						--b64size;
					}
					b64start = ++ptr;
					--b64size;
					if(strchr(line, '='))
						break;
				} while(b64size > 0L);

				free(line);
				fb = messageToFileblob(m, dir);
				messageDestroy(m);

				if(fb)
					fileblobDestroy(fb);
				else
					ret = -1;
			}
		} else if(scanelem->decoder == QUOTEDPRINTABLE) {
			char *quotedstart = scanelem->start;
			long quotedsize = scanelem->size;

			cli_dbgmsg("quotedsize = %lu\n", quotedsize);
			while(*quotedstart != '\n') {
				quotedstart++;
				quotedsize--;
			}
			/*
			 * Look for the end of the headers
			 */
			while(quotedstart < last) {
				if(*quotedstart == ';') {
					quotedstart++;
					quotedsize--;
				} else if(*quotedstart == '\n') {
					quotedstart++;
					quotedsize--;
					if((*quotedstart == '\n') || (*quotedstart == '\r')) {
						quotedstart++;
						quotedsize--;
						break;
					}
				}
				quotedstart++;
				quotedsize--;
			}

			while(!isalnum(*quotedstart)) {
				quotedstart++;
				quotedsize--;
			}

			if(quotedsize > 0L) {
				cli_dbgmsg("cli_mbox: decoding %ld quoted-printable bytes\n", quotedsize);

				m = messageCreate();
				if(m == NULL)
					return CL_EMEM;
				messageSetEncoding(m, "quoted-printable");

				line = NULL;

				do {
					int length = 0;

					/*printf("%ld: ", quotedsize); fflush(stdout);*/

					for(ptr = quotedstart; quotedsize && (*ptr != '\n') && (*ptr != '\r'); ptr++) {
						length++;
						--quotedsize;
					}

					/*printf("%d: ", length); fflush(stdout);*/

					line = cli_realloc(line, length + 1);

					memcpy(line, quotedstart, length);
					line[length] = '\0';

					/*puts(line);*/

					if(messageAddStr(m, line) < 0)
						break;

					if((quotedsize > 0) && (*ptr == '\r')) {
						ptr++;
						--quotedsize;
					}
					quotedstart = ++ptr;
					--quotedsize;
				} while(quotedsize > 0L);

				free(line);
				fb = messageToFileblob(m, dir);
				messageDestroy(m);

				if(fb)
					fileblobDestroy(fb);
				else
					ret = -1;
			}
		}
	}
	scanelem = scanlist;

	while(scanelem) {
		struct scanlist *n = scanelem->next;

		free(scanelem);
		scanelem = n;
	}

	if(wasAlloced)
		free(start);
	else
		munmap(start, size);

	/*
	 * FIXME: Need to run cl_scandir() here and return that value
	 */
	if(ret == 0)
		return CL_CLEAN;	/* a lie - but it gets things going */

	/* Fall back for now */
	lseek(desc, 0L, SEEK_SET);
	return cli_parse_mbox(dir, desc, options);
}
#else
int
cli_mbox(const char *dir, int desc, unsigned int options)
{
	if(dir == NULL) {
		cli_warnmsg("cli_mbox called with NULL dir\n");
		return CL_ENULLARG;
	}
	return cli_parse_mbox(dir, desc, options);
}
#endif

/*
 * TODO: when signal handling is added, need to remove temp files when a
 *	signal is received
 * TODO: add option to scan in memory not via temp files, perhaps with a
 * named pipe or memory mapped file, though this won't work on big e-mails
 * containing many levels of encapsulated messages - it'd just take too much
 * RAM
 * TODO: parse .msg format files
 * TODO: fully handle AppleDouble format, see
 *	http://www.lazerware.com/formats/Specs/AppleSingle_AppleDouble.pdf
 * TODO: ensure parseEmailHeaders is always called before parseEmailBody
 * TODO: create parseEmail which calls parseEmailHeaders then parseEmailBody
 * TODO: Look into TNEF. Is there anything that needs to be done here?
 */
static int
cli_parse_mbox(const char *dir, int desc, unsigned int options)
{
	int retcode, i;
	message *body;
	FILE *fd;
	char buffer[LINE_LENGTH + 1];
#ifdef HAVE_BACKTRACE
	void (*segv)(int);
#endif
	static table_t *rfc821, *subtype;
#ifdef	CL_DEBUG
	char tmpfilename[16];
	int tmpfd;
#endif

	cli_dbgmsg("in mbox()\n");

	i = dup(desc);
	if((fd = fdopen(i, "rb")) == NULL) {
		cli_errmsg("Can't open descriptor %d\n", desc);
		close(i);
		return CL_EOPEN;
	}
#ifdef	CL_DEBUG
	/*
	 * Copy the incoming mail for debugging, so that if it falls over
	 * we have a copy of the offending email. This is debugging code
	 * that you shouldn't of course install in a live environment. I am
	 * not interested in hearing about security issues with this section
	 * of the parser.
	 */
	strcpy(tmpfilename, "/tmp/mboxXXXXXX");
	tmpfd = mkstemp(tmpfilename);
	if(tmpfd < 0) {
		perror(tmpfilename);
		cli_errmsg("Can't make debugging file\n");
	} else {
		FILE *tmpfp = fdopen(tmpfd, "w");

		if(tmpfp) {
			while(fgets(buffer, sizeof(buffer) - 1, fd) != NULL)
				fputs(buffer, tmpfp);
			fclose(tmpfp);
			rewind(fd);
		} else
			cli_errmsg("Can't fdopen debugging file\n");
	}
#endif
	if(fgets(buffer, sizeof(buffer) - 1, fd) == NULL) {
		/* empty message */
		fclose(fd);
#ifdef	CL_DEBUG
		unlink(tmpfilename);
#endif
		return CL_CLEAN;
	}
#ifdef	CL_THREAD_SAFE
	pthread_mutex_lock(&tables_mutex);
#endif
	if(rfc821 == NULL) {
		assert(subtype == NULL);

		if(initialiseTables(&rfc821, &subtype) < 0) {
			rfc821 = NULL;
			subtype = NULL;
#ifdef	CL_THREAD_SAFE
			pthread_mutex_unlock(&tables_mutex);
#endif
			fclose(fd);
#ifdef	CL_DEBUG
			unlink(tmpfilename);
#endif
			return CL_EMEM;
		}
	}
#ifdef	CL_THREAD_SAFE
	pthread_mutex_unlock(&tables_mutex);
#endif

#ifdef HAVE_BACKTRACE
	segv = signal(SIGSEGV, sigsegv);
#endif

	/*
	 * Is it a UNIX style mbox with more than one
	 * mail message, or just a single mail message?
	 *
	 * TODO: It would be better if we called cli_scandir here rather than
	 * in cli_scanmail. Then we could improve the way mailboxes with more
	 * than one message is handled, e.g. stopping parsing when an infected
	 * message is stopped, and giving a better indication of which message
	 * within the mailbox is infected
	 */
	if(strncmp(buffer, "From ", 5) == 0) {
		/*
		 * Have been asked to check a UNIX style mbox file, which
		 * may contain more than one e-mail message to decode
		 *
		 * It would be far better for scanners.c to do this splitting
		 * and do this
		 *	FOR EACH mail in the mailbox
		 *	DO
		 *		pass this mail to cli_mbox --
		 *		scan this file
		 *		IF this file has a virus quit
		 *		THEN
		 *			return CL_VIRUS
		 *		FI
		 *	END
		 * This would remove a problem with this code that it can
		 * fill up the tmp directory before it starts scanning
		 */
		bool lastLineWasEmpty;
		int messagenumber;
		message *m = messageCreate();

		if(m == NULL) {
			fclose(fd);
#ifdef HAVE_BACKTRACE
			signal(SIGSEGV, segv);
#endif
#ifdef	CL_DEBUG
			unlink(tmpfilename);
#endif
			return CL_EMEM;
		}

		lastLineWasEmpty = FALSE;
		messagenumber = 1;

		do {
			cli_chomp(buffer);
			if(lastLineWasEmpty && (strncmp(buffer, "From ", 5) == 0)) {
				cli_dbgmsg("Deal with email number %d\n", messagenumber++);
				/*
				 * End of a message in the mail box
				 */
				body = parseEmailHeaders(m, rfc821);
				if(body == NULL) {
					messageReset(m);
					continue;
				}
				messageDestroy(m);
				if(messageGetBody(body))
					if(!parseEmailBody(body, NULL, dir, rfc821, subtype, options)) {
						messageReset(body);
						m = body;
						continue;
					}
				/*
				 * Starting a new message, throw away all the
				 * information about the old one. It would
				 * be best to be able to scan this message
				 * now, but cli_scanfile needs arguments
				 * that haven't been passed here so it can't be
				 * called
				 */
				m = body;
				messageReset(body);

				cli_dbgmsg("Finished processing message\n");
			} else
				lastLineWasEmpty = (bool)(buffer[0] == '\0');

			if(isuuencodebegin(buffer)) {
				/*
				 * Fast track visa to uudecode.
				 * TODO: binhex, yenc
				 */
				if(uufasttrack(m, buffer, dir, fd) < 0)
					if(messageAddStr(m, buffer) < 0)
					    break;
			} else
				if(messageAddStr(m, buffer) < 0)
					break;
		} while(fgets(buffer, sizeof(buffer) - 1, fd) != NULL);

		fclose(fd);

		cli_dbgmsg("Extract attachments from email %d\n", messagenumber);
		body = parseEmailHeaders(m, rfc821);
		messageDestroy(m);
	} else {
		/*
		 * It's a single message, parse the headers then the body
		 */
		if(strncmp(buffer, "P I ", 4) == 0)
			/*
			 * CommuniGate Pro format: ignore headers until
			 * blank line
			 */
			while((fgets(buffer, sizeof(buffer) - 1, fd) != NULL) &&
				(strchr("\r\n", buffer[0]) == NULL))
					;
		/*
		 * Ignore any blank lines at the top of the message
		 */
		while(strchr("\r\n", buffer[0]) &&
		     (getline_from_mbox(buffer, sizeof(buffer) - 1, fd) != NULL))
			;

		buffer[sizeof(buffer) - 1] = '\0';

		body = parseEmailFile(fd, rfc821, buffer, dir);
		fclose(fd);
	}

	/*
	 * This is not necessarily true, but since the only options are
	 * CL_CLEAN and CL_VIRUS this is the better choice. It would be
	 * nice to have CL_CONTINUESCANNING or something like that
	 */
	retcode = CL_CLEAN;

	if(body) {
		/*
		 * Write out the last entry in the mailbox
		 */
		if(messageGetBody(body))
			if(!parseEmailBody(body, NULL, dir, rfc821, subtype, options))
				retcode = CL_EFORMAT;

		/*
		 * Tidy up and quit
		 */
		messageDestroy(body);
	}

	cli_dbgmsg("cli_mbox returning %d\n", retcode);

#ifdef HAVE_BACKTRACE
	signal(SIGSEGV, segv);
#endif

#ifdef	CL_DEBUG
	unlink(tmpfilename);
#endif
	return retcode;
}

/*
 * Read in an email message from fin, parse it, and return the message
 *
 * FIXME: files full of new lines and nothing else are
 * handled ungracefully...
 */
static message *
parseEmailFile(FILE *fin, const table_t *rfc821, const char *firstLine, const char *dir)
{
	bool inHeader = TRUE;
	bool contMarker = FALSE;
	bool lastWasBlank = FALSE;
	message *ret;
	bool anyHeadersFound = FALSE;
	int commandNumber = -1;
	char *fullline = NULL, *boundary = NULL;
	size_t fulllinelength = 0;
	char buffer[LINE_LENGTH+1];

	cli_dbgmsg("parseEmailFile\n");

	ret = messageCreate();
	if(ret == NULL)
		return NULL;

	strcpy(buffer, firstLine);
	do {
		char *line;

		(void)cli_chomp(buffer);

		line = buffer;

		if(line[0] == '\0')
			line = NULL;

		/*
		 * Don't blank lines which are only spaces from headers,
		 * otherwise they'll be treated as the end of header marker
		 */
		if(lastWasBlank) {
			lastWasBlank = FALSE;
			if(boundaryStart(buffer, boundary)) {
				cli_dbgmsg("Found a header line with space that should be blank\n");
				inHeader = FALSE;
			}
		}
		if(boundary) {
			free(boundary);
			boundary = NULL;
		}
		if(inHeader) {
			cli_dbgmsg("parseEmailFile: check '%s' contMarker %d fullline 0x%p\n",
				buffer ? buffer : "", (int)contMarker, fullline);
			if(line && isspace(line[0])) {
				char copy[sizeof(buffer)];

				strcpy(copy, buffer);
				strstrip(copy);
				if(copy[0] == '\0') {
					/*
					 * The header line contains only white
					 * space. This is not the end of the
					 * headers according to RFC2822, but
					 * some MUAs will handle it as though
					 * it were, and virus writers exploit
					 * this bug. We can't just break from
					 * the loop here since that would allow
					 * other exploits such as inserting a
					 * white space line before the
					 * content-type line. So we just have
					 * to make a best guess. Sigh.
					 */
					if(fullline) {
						if(parseEmailHeader(ret, fullline, rfc821) < 0)
							continue;

						free(fullline);
						fullline = NULL;
					}
					if((boundary = (char *)messageFindArgument(ret, "boundary")) != NULL) {
						lastWasBlank = TRUE;
						continue;
					}
				}
			}
			lastWasBlank = FALSE;
			if((line == NULL) && (fullline == NULL)) {	/* empty line */
				if(!contMarker) {
					/*
					 * A blank line signifies the end of
					 * the header and the start of the text
					 */
					if(!anyHeadersFound)
						/* Ignore the junk at the top */
						continue;

					cli_dbgmsg("End of header information\n");
					inHeader = FALSE;
				} else
					contMarker = FALSE;
			} else {
				char *ptr;
				const char *qptr;
				int lookahead;

				if(fullline == NULL) {
					char cmd[LINE_LENGTH + 1], out[LINE_LENGTH + 1];

					/*
					 * Continuation of line we're ignoring?
					 */
					if((line[0] == '\t') || (line[0] == ' ') || contMarker) {
						contMarker = continuationMarker(line);
						continue;
					}

					/*
					 * Is this a header we're interested in?
					 */
					if((strchr(line, ':') == NULL) ||
					   (cli_strtokbuf(line, 0, ":", cmd) == NULL)) {
						if(strncmp(line, "From ", 5) == 0)
							anyHeadersFound = TRUE;
						continue;
					}

					ptr = rfc822comments(cmd, out);
					commandNumber = tableFind(rfc821, ptr ? ptr : cmd);

					switch(commandNumber) {
						case CONTENT_TRANSFER_ENCODING:
						case CONTENT_DISPOSITION:
						case CONTENT_TYPE:
							anyHeadersFound = TRUE;
							break;
						default:
							if(!anyHeadersFound)
								anyHeadersFound = usefulHeader(commandNumber, cmd);
							continue;
					}
					fullline = strdup(line);
					fulllinelength = strlen(line) + 1;
				} else if(line != NULL) {
					fulllinelength += strlen(line);
					fullline = cli_realloc(fullline, fulllinelength);
					strcat(fullline, line);
				}

				if(line) {
					contMarker = continuationMarker(line);

					if(contMarker)
						continue;
				} else
					contMarker = FALSE;

				assert(fullline != NULL);

				lookahead = getc(fin);
				if(lookahead != EOF) {
					ungetc(lookahead, fin);

					/*
					 * Section B.2 of RFC822 says TAB or
					 * SPACE means a continuation of the
					 * previous entry.
					 *
					 * Add all the arguments on the line
					 */
					if((lookahead == '\t') || (lookahead == ' '))
						continue;
				}

				if(line) {
					int quotes = 0;
					for(qptr = fullline; *qptr; qptr++)
						if(*qptr == '\"')
							quotes++;

					if(quotes & 1)
						continue;
				}

				ptr = rfc822comments(fullline, NULL);
				if(ptr) {
					free(fullline);
					fullline = ptr;
				}

				if(parseEmailHeader(ret, fullline, rfc821) < 0)
					continue;

				free(fullline);
				fullline = NULL;
			}
		} else if(line && isuuencodebegin(line)) {
			/*
			 * Fast track visa to uudecode.
			 * TODO: binhex, yenc
			 */
			if(uufasttrack(ret, line, dir, fin) < 0)
				if(messageAddStr(ret, line) < 0)
					break;
		} else
			if(messageAddStr(ret, line) < 0)
				break;
	} while(getline_from_mbox(buffer, sizeof(buffer) - 1, fin) != NULL);

	if(fullline) {
		if(*fullline) switch(commandNumber) {
			case CONTENT_TRANSFER_ENCODING:
			case CONTENT_DISPOSITION:
			case CONTENT_TYPE:
				cli_dbgmsg("parseEmailHeaders: Fullline unparsed '%s'\n", fullline);
		}
		free(fullline);
	}

	if(!anyHeadersFound) {
		/*
		 * False positive in believing we have an e-mail when we don't
		 */
		messageDestroy(ret);
		cli_dbgmsg("parseEmailFile: no headers found, assuming it isn't an email\n");
		return NULL;
	}

	messageClean(ret);

	cli_dbgmsg("parseEmailFile: return\n");

	return ret;
}

/*
 * The given message contains a raw e-mail.
 *
 * Returns the message's body with the correct arguments set
 *
 * The downside of this approach is that for a short time we have two copies
 * of the message in memory, the upside is that it makes for easier parsing
 * of encapsulated messages, and in the long run uses less memory in those
 * scenarios
 *
 * TODO: remove the duplication with parseEmailFile
 */
static message *
parseEmailHeaders(const message *m, const table_t *rfc821)
{
	bool inHeader = TRUE;
	const text *t;
	message *ret;
	bool anyHeadersFound = FALSE;
	int commandNumber = -1;
	char *fullline = NULL;
	size_t fulllinelength = 0;

	cli_dbgmsg("parseEmailHeaders\n");

	if(m == NULL)
		return NULL;

	ret = messageCreate();

	for(t = messageGetBody(m); t; t = t->t_next) {
		const char *buffer;

		if(t->t_line)
			buffer = lineGetData(t->t_line);
		else
			buffer = NULL;

		if(inHeader) {
			cli_dbgmsg("parseEmailHeaders: check '%s'\n",
				buffer ? buffer : "");
			if(buffer == NULL) {
				/*
				 * A blank line signifies the end of
				 * the header and the start of the text
				 */
				cli_dbgmsg("End of header information\n");
				inHeader = FALSE;
				if(!anyHeadersFound) {
					cli_dbgmsg("Nothing interesting in the header\n");
					break;
				}
			} else {
				char *ptr;
				const char *qptr;
				int quotes;

				if(fullline == NULL) {
					char cmd[LINE_LENGTH + 1];

					/*
					 * Continuation of line we're ignoring?
					 */
					if((buffer[0] == '\t') || (buffer[0] == ' '))
						continue;

					/*
					 * Is this a header we're interested in?
					 */
					if((strchr(buffer, ':') == NULL) ||
					   (cli_strtokbuf(buffer, 0, ":", cmd) == NULL)) {
						if(strncmp(buffer, "From ", 5) == 0)
							anyHeadersFound = TRUE;
						continue;
					}

					ptr = rfc822comments(cmd, NULL);
					commandNumber = tableFind(rfc821, ptr ? ptr : cmd);
					if(ptr)
						free(ptr);

					switch(commandNumber) {
						case CONTENT_TRANSFER_ENCODING:
						case CONTENT_DISPOSITION:
						case CONTENT_TYPE:
							anyHeadersFound = TRUE;
							break;
						default:
							if(!anyHeadersFound)
								anyHeadersFound = usefulHeader(commandNumber, cmd);
							continue;
					}
					fullline = strdup(buffer);
					fulllinelength = strlen(buffer) + 1;
				} else if(buffer) {
					fulllinelength += strlen(buffer);
					fullline = cli_realloc(fullline, fulllinelength);
					strcat(fullline, buffer);
				}

				assert(fullline != NULL);

				if(t->t_next && (t->t_next->t_line != NULL))
					/*
					 * Section B.2 of RFC822 says TAB or
					 * SPACE means a continuation of the
					 * previous entry.
					 *
					 * Add all the arguments on the line
					 */
					switch(lineGetData(t->t_next->t_line)[0]) {
						case ' ':
						case '\t':
							continue;
					}

				quotes = 0;
				for(qptr = fullline; *qptr; qptr++)
					if(*qptr == '\"')
						quotes++;

				if(quotes & 1)
					continue;

				ptr = rfc822comments(fullline, NULL);
				if(ptr) {
					free(fullline);
					fullline = ptr;
				}

				if(parseEmailHeader(ret, fullline, rfc821) < 0)
					continue;

				free(fullline);
				fullline = NULL;
			}
		} else
			/*cli_dbgmsg("Add line to body '%s'\n", buffer);*/
			if(messageAddLine(ret, t->t_line) < 0)
				break;
	}

	if(fullline) {
		if(*fullline) switch(commandNumber) {
			case CONTENT_TRANSFER_ENCODING:
			case CONTENT_DISPOSITION:
			case CONTENT_TYPE:
				cli_dbgmsg("parseEmailHeaders: Fullline unparsed '%s'\n", fullline);
		}
		free(fullline);
	}

	if(!anyHeadersFound) {
		/*
		 * False positive in believing we have an e-mail when we don't
		 */
		messageDestroy(ret);
		cli_dbgmsg("parseEmailHeaders: no headers found, assuming it isn't an email\n");
		return NULL;
	}

	messageClean(ret);

	cli_dbgmsg("parseEmailHeaders: return\n");

	return ret;
}

/*
 * Handle a header line of an email message
 */
static int
parseEmailHeader(message *m, const char *line, const table_t *rfc821)
{
	char *cmd;
	int ret = -1;
#ifdef CL_THREAD_SAFE
	char *strptr;
#endif
	const char *separater;
	char *copy, tokenseparater[2];

	cli_dbgmsg("parseEmailHeader '%s'\n", line);

	/*
	 * In RFC822 the separater between the key a value is a colon,
	 * e.g.	Content-Transfer-Encoding: base64
	 * However some MUA's are lapse about this and virus writers exploit
	 * this hole, so we need to check all known possiblities
	 */
	for(separater = ":= "; *separater; separater++)
		if(strchr(line, *separater) != NULL)
			break;

	if(*separater == '\0')
		return -1;

	copy = rfc2047(line);
	if(copy == NULL)
		/* an RFC checker would return -1 here */
		copy = strdup(line);

	tokenseparater[0] = *separater;
	tokenseparater[1] = '\0';

#ifdef	CL_THREAD_SAFE
	cmd = strtok_r(copy, tokenseparater, &strptr);
#else
	cmd = strtok(copy, tokenseparater);
#endif

	if(cmd && (strstrip(cmd) > 0)) {
#ifdef	CL_THREAD_SAFE
		char *arg = strtok_r(NULL, "", &strptr);
#else
		char *arg = strtok(NULL, "");
#endif

		if(arg)
			/*
			 * Found a header such as
			 * Content-Type: multipart/mixed;
			 * set arg to be
			 * "multipart/mixed" and cmd to
			 * be "Content-Type"
			 */
			ret = parseMimeHeader(m, cmd, rfc821, arg);
	}
	free(copy);
	return ret;
}

/*
 * This is a recursive routine.
 * FIXME: We are not passed &mrec so we can't check against MAX_MAIL_RECURSION
 *
 * This function parses the body of mainMessage and saves its attachments in dir
 *
 * mainMessage is the buffer to be parsed, it contains an e-mail's body, without
 * any headers. First time of calling it'll be
 * the whole message. Later it'll be parts of a multipart message
 * textIn is the plain text message being built up so far
 *
 * Returns:
 *	0 for fail
 *	1 for success, attachments saved
 *	2 for success, attachments not saved
 */
static int	/* success or fail */
parseEmailBody(message *messageIn, text *textIn, const char *dir, const table_t *rfc821Table, const table_t *subtypeTable, unsigned int options)
{
	message **messages;	/* parts of a multipart message */
	int inMimeHead, i, rc = 1, htmltextPart, multiparts = 0;
	text *aText;
	const char *cptr;
	message *mainMessage;
	fileblob *fb;

	cli_dbgmsg("in parseEmailBody\n");

	aText = textIn;
	messages = NULL;
	mainMessage = messageIn;

	/* Anything left to be parsed? */
	if(mainMessage && (messageGetBody(mainMessage) != NULL)) {
		mime_type mimeType;
		int subtype, inhead;
		const char *mimeSubtype, *boundary;
		char *protocol;
		const text *t_line;
		/*bool isAlternative;*/
		message *aMessage;

		cli_dbgmsg("Parsing mail file\n");

		mimeType = messageGetMimeType(mainMessage);
		mimeSubtype = messageGetMimeSubtype(mainMessage);

		/* pre-process */
		subtype = tableFind(subtypeTable, mimeSubtype);
		if((mimeType == TEXT) && (subtype == PLAIN)) {
			/*
			 * This is effectively no encoding, notice that we
			 * don't check that charset is us-ascii
			 */
			cli_dbgmsg("assume no encoding\n");
			mimeType = NOMIME;
			messageSetMimeSubtype(mainMessage, "");
		} else if((mimeType == MESSAGE) &&
			  (strcasecmp(mimeSubtype, "rfc822-headers") == 0)) {
			/*
			 * RFC1892/RFC3462: section 2 text/rfc822-headers
			 * incorrectly sent as message/rfc822-headers
			 *
			 * Parse as text/plain, i.e. no mime
			 */
			cli_dbgmsg("Changing message/rfc822-headers to text/rfc822-headers\n");
			mimeType = NOMIME;
			messageSetMimeSubtype(mainMessage, "");
		}

		cli_dbgmsg("mimeType = %d\n", mimeType);

		switch(mimeType) {
		case NOMIME:
			cli_dbgmsg("Not a mime encoded message\n");
			aText = textAddMessage(aText, mainMessage);
			break;
		case TEXT:
			/* text/plain has been preprocessed as no encoding */
			if((options&CL_SCAN_MAILURL) && (subtype == HTML))
				checkURLs(mainMessage, dir);
			break;
		case MULTIPART:
			cli_dbgmsg("Content-type 'multipart' handler\n");
			boundary = messageFindArgument(mainMessage, "boundary");

			if(boundary == NULL) {
				cli_warnmsg("Multipart MIME message contains no boundaries\n");
				/* Broken e-mail message */
				mimeType = NOMIME;
				/*
				 * The break means that we will still
				 * check if the file contains a uuencoded file
				 */
				break;
			}

			/* Perhaps it should assume mixed? */
			if(mimeSubtype[0] == '\0') {
				cli_warnmsg("Multipart has no subtype assuming alternative\n");
				mimeSubtype = "alternative";
				messageSetMimeSubtype(mainMessage, "alternative");
			}

			/*
			 * Get to the start of the first message
			 */
			t_line = messageGetBody(mainMessage);

			if(t_line == NULL) {
				cli_warnmsg("Multipart MIME message has no body\n");
				free((char *)boundary);
				mimeType = NOMIME;
				break;
			}

			do
				if(t_line->t_line) {
					if(boundaryStart(lineGetData(t_line->t_line), boundary))
						break;
					/*
					 * Found a uuencoded/binhex file before
					 *	the first multipart
					 * TODO: check yEnc
					 */
					if(uuencodeBegin(mainMessage) == t_line) {
						if(messageGetEncoding(mainMessage) == NOENCODING) {
							messageSetEncoding(mainMessage, "x-uuencode");
							fb = messageToFileblob(mainMessage, dir);

							if(fb)
								fileblobDestroy(fb);
						}
					} else if(binhexBegin(mainMessage) == t_line) {
						if(messageGetEncoding(mainMessage) == NOENCODING) {
							messageSetEncoding(mainMessage, "x-binhex");
							fb = messageToFileblob(mainMessage, dir);

							if(fb)
								fileblobDestroy(fb);
						}
					} else if(encodingLine(mainMessage) == t_line->t_next) {
						/*
						 * We look for the next line
						 * since later on we'll skip
						 * over the important line when
						 * we think it's a blank line
						 * at the top of the message -
						 * which it would have been in
						 * an RFC compliant world
						 */
						cli_dbgmsg("Found MIME attachment before the first MIME section\n");
						if(messageGetEncoding(mainMessage) == NOENCODING)
							break;
					}
				}
			while((t_line = t_line->t_next) != NULL);

			if(t_line == NULL) {
				cli_dbgmsg("Multipart MIME message contains no boundary lines\n");
				/*
				 * Free added by Thomas Lamy
				 * <Thomas.Lamy@in-online.net>
				 */
				free((char *)boundary);
				mimeType = NOMIME;
				/*
				 * The break means that we will still
				 * check if the file contains a uuencoded file
				 */
				break;
			}
			/*
			 * Build up a table of all of the parts of this
			 * multipart message. Remember, each part may itself
			 * be a multipart message.
			 */
			inhead = 1;
			inMimeHead = 0;

			/*
			 * Parse the mainMessage object and create an array
			 * of objects called messages, one for each of the
			 * multiparts that mainMessage contains
			 *
			 * This looks like parseEmailHeaders() - maybe there's
			 * some duplication of code to be cleaned up
			 */
			for(multiparts = 0; t_line; multiparts++) {
				int lines = 0;
				message **m;

				m = cli_realloc(messages, ((multiparts + 1) * sizeof(message *)));
				if(m == NULL)
					break;
				messages = m;

				aMessage = messages[multiparts] = messageCreate();
				if(aMessage == NULL) {
					multiparts--;
					continue;
				}

				cli_dbgmsg("Now read in part %d\n", multiparts);

				/*
				 * Ignore blank lines. There shouldn't be ANY
				 * but some viruses insert them
				 */
				while((t_line = t_line->t_next) != NULL)
					if(t_line->t_line &&
					   /*(cli_chomp(t_line->t_text) > 0))*/
					   (strlen(lineGetData(t_line->t_line)) > 0))
						break;

				if(t_line == NULL) {
					cli_dbgmsg("Empty part\n");
					/*
					 * Remove this part unless there's
					 * a uuencoded portion somewhere in
					 * the complete message that we may
					 * throw away by mistake if the MIME
					 * encoding information is incorrect
					 */
					if(uuencodeBegin(mainMessage) == NULL) {
						messageDestroy(aMessage);
						--multiparts;
					}
					continue;
				}

				do {
					const char *line = lineGetData(t_line->t_line);

					/*cli_dbgmsg("inMimeHead %d inhead %d boundary '%s' line '%s' next '%s'\n",
						inMimeHead, inhead, boundary, line,
						t_line->t_next && t_line->t_next->t_line ? lineGetData(t_line->t_next->t_line) : "(null)");*/

					if(inMimeHead) {	/* continuation line */
						if(line == NULL) {
							/*inhead =*/ inMimeHead = 0;
							continue;
						}
						/*
						 * Handle continuation lines
						 * because the previous line
						 * ended with a ; or this line
						 * starts with a white space
						 */
						cli_dbgmsg("Multipart %d: About to add mime Argument '%s'\n",
							multiparts, line);
						/*
						 * Handle the case when it
						 * isn't really a continuation
						 * line:
						 * Content-Type: application/octet-stream;
						 * Content-Transfer-Encoding: base64
						 */
						parseEmailHeader(aMessage, line, rfc821Table);

						while(isspace((int)*line))
							line++;

						if(*line == '\0') {
							inhead = inMimeHead = 0;
							continue;
						}
						/*
						 * This may cause a trailing ';'
						 * to be added if this test
						 * fails - TODO: verify this
						 */
						inMimeHead = continuationMarker(line);
						messageAddArgument(aMessage, line);
					} else if(inhead) {	/* handling normal headers */
						int quotes;
						char *fullline, *ptr;
						const char *qptr;
						const text *next;

						if(line == NULL) {
							/*
							 * empty line, should the end of the headers,
							 * but some base64 decoders, e.g. uudeview, are broken
							 * and will handle this type of entry, decoding the
							 * base64 content...
							 * Content-Type: application/octet-stream; name=text.zip
							 * Content-Transfer-Encoding: base64
							 * Content-Disposition: attachment; filename="text.zip"
							 *
							 * Content-Disposition: attachment;
							 *	filename=text.zip
							 * Content-Type: application/octet-stream;
							 *	name=text.zip
							 * Content-Transfer-Encoding: base64
							 *
							 * UEsDBAoAAAAAAACgPjJ2RHw676gAAO+oAABEAAAAbWFpbF90ZXh0LWluZm8udHh0ICAgICAgICAg
							 */
							next = t_line->t_next;
							if(next && next->t_line) {
								const char *data = lineGetData(next->t_line);

								if((messageGetEncoding(aMessage) == NOENCODING) &&
								   (messageGetMimeType(aMessage) == APPLICATION))
									/*
									 * Handle this nightmare (note the blank
									 * line in the header and the incorrect
									 * content-transfer-encoding header)
									 *
									 * Content-Type: application/octet-stream; name="zipped_files.EXEX-Spanska: Yes
									 *
									 * r-Encoding: base64
									 * Content-Disposition: attachment; filename="zipped_files.EXE"
									 */
									if(strstr(data, "base64")) {
										messageSetEncoding(aMessage, "base64");
										cli_dbgmsg("Ignoring fake end of headers\n");
										continue;
									}
								if((strncmp(data, "Content", 7) == 0) ||
								   (strncmp(data, "filename=", 9) == 0)) {
									cli_dbgmsg("Ignoring fake end of headers\n");
									continue;
								}
							}
							cli_dbgmsg("Multipart %d: End of header information\n",
								multiparts);
							inhead = 0;
							continue;
						}
						if(isspace((int)*line)) {
							/*
							 * The first line is
							 * continuation line.
							 * This is tricky
							 * to handle, but
							 * all we can do is our
							 * best
							 */
							cli_dbgmsg("Part %d starts with a continuation line\n",
								multiparts);
							messageAddArgument(aMessage, line);
							/*
							 * Give it a default
							 * MIME type since
							 * that may be the
							 * missing line
							 *
							 * Choose application to
							 * force a save
							 */
							if(messageGetMimeType(aMessage) == NOMIME)
								messageSetMimeType(aMessage, "application");
							continue;
						}

						inMimeHead = FALSE;

						assert(strlen(line) <= LINE_LENGTH);

						fullline = rfc822comments(line, NULL);
						if(fullline == NULL)
							fullline = strdup(line);

						quotes = 0;
						for(qptr = fullline; *qptr; qptr++)
							if(*qptr == '\"')
								quotes++;

						/*
						 * Fold next lines to the end of this
						 * if they start with a white space
						 * or if this line has an odd number of quotes:
						 * Content-Type: application/octet-stream; name="foo
						 * "
						 */
						next = t_line->t_next;
						while(next && next->t_line) {
							const char *data = lineGetData(next->t_line);

							/*if((!isspace(data[0])) &&
							   ((quotes & 1) == 0))
								break;*/
							if(!isspace(data[0]))
								break;

							ptr = cli_realloc(fullline,
								strlen(fullline) + strlen(data) + 1);

							if(ptr == NULL)
								break;

							fullline = ptr;
							strcat(fullline, data);

							/*for(qptr = data; *qptr; qptr++)
								if(*qptr == '\"')
									quotes++;*/

							t_line = next;
							next = next->t_next;
						}
						cli_dbgmsg("Multipart %d: About to parse folded header '%s'\n",
							multiparts, fullline);

						parseEmailHeader(aMessage, fullline, rfc821Table);
						free(fullline);
					} else if(endOfMessage(line, boundary)) {
						/*
						 * Some viruses put information
						 * *after* the end of message,
						 * which presumably some broken
						 * mail clients find, so we
						 * can't assume that this
						 * is the end of the message
						 */
						/* t_line = NULL;*/
						break;
					} else if(boundaryStart(line, boundary)) {
						inhead = 1;
						break;
					} else {
						if(messageAddLine(aMessage, t_line->t_line) < 0)
							break;
						lines++;
					}
				} while((t_line = t_line->t_next) != NULL);

				messageClean(aMessage);

				cli_dbgmsg("Part %d has %d lines\n",
					multiparts, lines);
			}

			free((char *)boundary);

			/*
			 * Preprocess. Anything special to be done before
			 * we handle the multiparts?
			 */
			switch(tableFind(subtypeTable, mimeSubtype)) {
				case KNOWBOT:
					/* TODO */
					cli_dbgmsg("multipart/knowbot parsed as multipart/mixed for now\n");
					mimeSubtype = "mixed";
					break;
				case -1:
					/*
					 * According to section 7.2.6 of
					 * RFC1521, unrecognised multiparts
					 * should be treated as multipart/mixed.
					 */
					cli_dbgmsg("Unsupported multipart format `%s', parsed as mixed\n", mimeSubtype);
					mimeSubtype = "mixed";
					break;
			}

			/*
			 * We've finished message we're parsing
			 */
			if(mainMessage && (mainMessage != messageIn)) {
				messageDestroy(mainMessage);
				mainMessage = NULL;
			}

			if(multiparts == 0) {
				if(messages)
					free(messages);
				return 2;	/* Nothing to do */
			}

			cli_dbgmsg("The message has %d parts\n", multiparts);
			cli_dbgmsg("Find out the multipart type (%s)\n", mimeSubtype);

			/*
			 * We now have all the parts of the multipart message
			 * in the messages array:
			 *	message *messages[multiparts]
			 * Let's decide what to do with them all
			 */
			switch(tableFind(subtypeTable, mimeSubtype)) {
			case RELATED:
				cli_dbgmsg("Multipart related handler\n");
				/*
				 * Have a look to see if there's HTML code
				 * which will need scanning
				 */
				aMessage = NULL;
				assert(multiparts > 0);

				htmltextPart = getTextPart(messages, multiparts);

				if(htmltextPart >= 0)
					aText = textAddMessage(aText, messages[htmltextPart]);
				else
					/*
					 * There isn't an HTML bit. If there's a
					 * multipart bit, it'll may be in there
					 * somewhere
					 */
					for(i = 0; i < multiparts; i++)
						if(messageGetMimeType(messages[i]) == MULTIPART) {
							aMessage = messages[i];
							htmltextPart = i;
							break;
						}

				if(htmltextPart == -1)
					cli_dbgmsg("No HTML code found to be scanned");
				else {
					rc = parseEmailBody(aMessage, aText, dir, rfc821Table, subtypeTable, options);
					if(rc == 1) {
						assert(aMessage == messages[htmltextPart]);
						messageDestroy(aMessage);
						messages[htmltextPart] = NULL;
					}
				}

				/*
				 * Fixed based on an idea from Stephen White <stephen@earth.li>
				 * The message is confused about the difference
				 * between alternative and related. Badtrans.B
				 * suffers from this problem.
				 *
				 * Fall through in this case:
				 * Content-Type: multipart/related;
				 *	type="multipart/alternative"
				 */
				/*
				 * Changed to always fall through based on
				 * an idea from Michael Dankov <misha@btrc.ru>
				 * that some viruses are completely confused
				 * about the difference between related
				 * and mixed
				 */
				/*cptr = messageFindArgument(mainMessage, "type");
				if(cptr == NULL)
					break;
				isAlternative = (bool)(strcasecmp(cptr, "multipart/alternative") == 0);
				free((char *)cptr);
				if(!isAlternative)
					break;*/
			case DIGEST:
				/*
				 * According to section 5.1.5 RFC2046, the
				 * default mime type of multipart/digest parts
				 * is message/rfc822
				 *
				 * We consider them as alternative, wrong in
				 * the strictest sense since they aren't
				 * alternatives - all parts a valid - but it's
				 * OK for our needs since it means each part
				 * will be scanned
				 */
			case ALTERNATIVE:
				cli_dbgmsg("Multipart alternative handler\n");

				/*
				 * Fall through - some clients are broken and
				 * say alternative instead of mixed. The Klez
				 * virus is broken that way, and anyway we
				 * wish to scan all of the alternatives
				 */
			case REPORT:
				/*
				 * According to section 1 of RFC1892, the
				 * syntax of multipart/report is the same
				 * as multipart/mixed. There are some required
				 * parameters, but there's no need for us to
				 * verify that they exist
				 */
			case MIXED:
			case APPLEDOUBLE:	/* not really supported */
				/*
				 * Look for attachments
				 *
				 * Not all formats are supported. If an
				 * unsupported format turns out to be
				 * common enough to implement, it is a simple
				 * matter to add it
				 */
				if(aText) {
					if(mainMessage && (mainMessage != messageIn))
						messageDestroy(mainMessage);
					mainMessage = NULL;
				}

				cli_dbgmsg("Mixed message with %d parts\n", multiparts);
				for(i = 0; i < multiparts; i++) {
					bool addToText = FALSE;
					const char *dtype;
#ifndef	SAVE_TO_DISC
					message *body;
#endif

					aMessage = messages[i];

					if(aMessage == NULL)
						continue;

					cli_dbgmsg("Mixed message part %d is of type %d\n",
						i, messageGetMimeType(aMessage));

					switch(messageGetMimeType(aMessage)) {
					case APPLICATION:
					case AUDIO:
					case IMAGE:
					case VIDEO:
						break;
					case NOMIME:
						cli_dbgmsg("No mime headers found in multipart part %d\n", i);
						if(mainMessage) {
							if(uuencodeBegin(aMessage)) {
								cli_dbgmsg("Found uuencoded message in multipart/mixed mainMessage\n");
								messageSetEncoding(mainMessage, "x-uuencode");
								fb = messageToFileblob(mainMessage, dir);

								if(fb)
									fileblobDestroy(fb);
							}
							if(mainMessage != messageIn)
								messageDestroy(mainMessage);
							mainMessage = NULL;
						} else if(aMessage) {
							if(uuencodeBegin(aMessage)) {
								cli_dbgmsg("Found uuencoded message in multipart/mixed non mime part\n");
								messageSetEncoding(aMessage, "x-uuencode");
								fb = messageToFileblob(aMessage, dir);

								if(fb)
									fileblobDestroy(fb);
								assert(aMessage == messages[i]);
								messageReset(messages[i]);
							} else if(binhexBegin(aMessage)) {
								cli_dbgmsg("Found binhex message in multipart/mixed non mime part\n");
								messageSetEncoding(aMessage, "x-binhex");
								fb = messageToFileblob(aMessage, dir);

								if(fb)
									fileblobDestroy(fb);
								assert(aMessage == messages[i]);
								messageReset(messages[i]);
							}
						}
						addToText = TRUE;
						if(messageGetBody(aMessage) == NULL)
							/*
							 * No plain text version
							 */
							messageAddStr(aMessage, "No plain text alternative");
						assert(messageGetBody(aMessage) != NULL);
						break;
					case TEXT:
						dtype = messageGetDispositionType(aMessage);
						cli_dbgmsg("Mixed message text part disposition \"%s\"\n",
							dtype);
						if(strcasecmp(dtype, "attachment") == 0)
							break;
						if((*dtype == '\0') || (strcasecmp(dtype, "inline") == 0)) {
							if(mainMessage && (mainMessage != messageIn))
								messageDestroy(mainMessage);
							mainMessage = NULL;
							cptr = messageGetMimeSubtype(aMessage);
							cli_dbgmsg("Mime subtype \"%s\"\n", cptr);
							if(uuencodeBegin(aMessage)) {
								cli_dbgmsg("Found uuencoded message in multipart/mixed text portion\n");
								messageSetEncoding(aMessage, "x-uuencode");
							} else if((tableFind(subtypeTable, cptr) == PLAIN) &&
								  (messageGetEncoding(aMessage) == NOENCODING)) {
								char *filename;
								/*
								 * Strictly speaking
								 * a text/plain part is
								 * not an attachment. We
								 * pretend it is so that
								 * we can decode and
								 * scan it
								 */
								filename = (char *)messageFindArgument(aMessage, "filename");
								if(filename == NULL)
									filename = (char *)messageFindArgument(aMessage, "name");

								if(filename == NULL) {
									cli_dbgmsg("Adding part to main message\n");
									addToText = TRUE;
								} else {
									cli_dbgmsg("Treating %s as attachment\n",
										filename);
									free(filename);
								}
							} else {
								if(options&CL_SCAN_MAILURL)
									if(tableFind(subtypeTable, cptr) == HTML)
										checkURLs(aMessage, dir);
								messageAddArgument(aMessage, "filename=mixedtextportion");
							}
						} else {
							cli_dbgmsg("Text type %s is not supported\n", dtype);
							continue;
						}
						break;
					case MESSAGE:
						/* Content-Type: message/rfc822 */
						cli_dbgmsg("Found message inside multipart (encoding type %d)\n",
							messageGetEncoding(aMessage));
						switch(messageGetEncoding(aMessage)) {
							case NOENCODING:
							case EIGHTBIT:
							case BINARY:
								if(encodingLine(aMessage) == NULL) {
									/*
									 * This means that the message has no attachments
									 * The test for messageGetEncoding is needed since
									 * encodingLine won't have been set if the message
									 * itself has been encoded
									 */
									cli_dbgmsg("No encoding line found in the multipart/message\n");
									assert(aMessage == messages[i]);
									messageDestroy(messages[i]);
									messages[i] = NULL;
									continue;
								}
						}
#if	0
						messageAddStrAtTop(aMessage,
							"Received: by clamd (message/rfc822)");
#endif
#ifdef	SAVE_TO_DISC
						/*
						 * Save this embedded message
						 * to a temporary file
						 */
						saveTextPart(aMessage, dir);
						assert(aMessage == messages[i]);
						messageDestroy(messages[i]);
						messages[i] = NULL;
#else
						/*
						 * Scan in memory, faster but
						 * is open to DoS attacks when
						 * many nested levels are
						 * involved.
						 */
						body = parseEmailHeaders(aMessage, rfc821Table, TRUE);
						/*
						 * We've fininished with the
						 * original copy of the message,
						 * so throw that away and
						 * deal with the encapsulated
						 * message as a message.
						 * This can save a lot of memory
						 */
						assert(aMessage == messages[i]);
						messageDestroy(messages[i]);
						messages[i] = NULL;
						if(body) {
							rc = parseEmailBody(body, NULL, dir, rfc821Table, subtypeTable, options);
							messageDestroy(body);
						}
#endif
						continue;
					case MULTIPART:
						/*
						 * It's a multi part within a multi part
						 * Run the message parser on this bit, it won't
						 * be an attachment
						 */
						cli_dbgmsg("Found multipart inside multipart\n");
						if(aMessage) {
							/*
							 * The headers were parsed when reading in the
							 * whole multipart section
							 */
							rc = parseEmailBody(aMessage, aText, dir, rfc821Table, subtypeTable, options);
							cli_dbgmsg("Finished recursion\n");
							assert(aMessage == messages[i]);
							messageDestroy(messages[i]);
							messages[i] = NULL;
						} else {
							rc = parseEmailBody(NULL, NULL, dir, rfc821Table, subtypeTable, options);
							if(mainMessage && (mainMessage != messageIn))
								messageDestroy(mainMessage);
							mainMessage = NULL;
						}
						continue;
					default:
						cli_warnmsg("Only text and application attachments are supported, type = %d\n",
							messageGetMimeType(aMessage));
						continue;
					}

					if(addToText) {
						cli_dbgmsg("Adding to non mime-part\n");
						aText = textAdd(aText, messageGetBody(aMessage));
					} else {
						fb = messageToFileblob(aMessage, dir);

						if(fb)
							fileblobDestroy(fb);
					}
					assert(aMessage == messages[i]);
					messageDestroy(messages[i]);
					messages[i] = NULL;
				}

				/* rc = parseEmailBody(NULL, NULL, dir, rfc821Table, subtypeTable, options); */
				break;
			case SIGNED:
			case PARALLEL:
				/*
				 * If we're here it could be because we have a
				 * multipart/mixed message, consisting of a
				 * message followed by an attachment. That
				 * message itself is a multipart/alternative
				 * message and we need to dig out the plain
				 * text part of that alternative
				 */
				htmltextPart = getTextPart(messages, multiparts);
				if(htmltextPart == -1)
					htmltextPart = 0;

				rc = parseEmailBody(messages[htmltextPart], aText, dir, rfc821Table, subtypeTable, options);
				break;
			case ENCRYPTED:
				rc = 0;
				protocol = (char *)messageFindArgument(mainMessage, "protocol");
				if(protocol) {
					if(strcasecmp(protocol, "application/pgp-encrypted") == 0) {
						/* RFC2015 */
						cli_warnmsg("PGP encoded attachment not scanned\n");
						rc = 2;
					} else
						cli_warnmsg("Unknown encryption protocol '%s' - if you believe this file contains a virus, report it to bugs@clamav.net\n");
					free(protocol);
				} else
					cli_dbgmsg("Encryption method missing protocol name\n");

				break;
			default:
				assert(0);
			}

			if(mainMessage && (mainMessage != messageIn))
				messageDestroy(mainMessage);

			if(aText && (textIn == NULL)) {
				if((fb = fileblobCreate()) != NULL) {
					cli_dbgmsg("Save non mime and/or text/plain part\n");
					fileblobSetFilename(fb, dir, "textpart");
					/*fileblobAddData(fb, "Received: by clamd (textpart)\n", 30);*/
					(void)textToFileblob(aText, fb);

					fileblobDestroy(fb);
				}
				textDestroy(aText);
			}

			for(i = 0; i < multiparts; i++)
				if(messages[i])
					messageDestroy(messages[i]);

			if(messages)
				free(messages);

			return rc;

		case MESSAGE:
			/*
			 * Check for forbidden encodings
			 */
			switch(messageGetEncoding(mainMessage)) {
				case NOENCODING:
				case EIGHTBIT:
				case BINARY:
					break;
				default:
					cli_warnmsg("MIME type 'message' cannot be decoded\n");
					break;
			}
			rc = 0;
			if((strcasecmp(mimeSubtype, "rfc822") == 0) ||
			   (strcasecmp(mimeSubtype, "delivery-status") == 0)) {
				message *m = parseEmailHeaders(mainMessage, rfc821Table);
				if(m) {
					cli_dbgmsg("Decode rfc822");

					if(mainMessage && (mainMessage != messageIn)) {
						messageDestroy(mainMessage);
						mainMessage = NULL;
					} else
						messageReset(mainMessage);
					if(messageGetBody(m))
						rc = parseEmailBody(m, NULL, dir, rfc821Table, subtypeTable, options);

					messageDestroy(m);
				}
				break;
			} else if(strcasecmp(mimeSubtype, "disposition-notification") == 0) {
				/* RFC 2298 - handle like a normal email */
				rc = 1;
				break;
			} else if(strcasecmp(mimeSubtype, "partial") == 0) {
#ifdef	PARTIAL_DIR
				/* RFC1341 message split over many emails */
				if(rfc1341(mainMessage, dir) >= 0)
					rc = 1;
#else
				cli_warnmsg("Partial message received from MUA/MTA - message cannot be scanned\n");
				rc = 0;
#endif
			} else if(strcasecmp(mimeSubtype, "external-body") == 0)
				/* TODO */
				cli_warnmsg("Attempt to send Content-type message/external-body trapped");
			else
				cli_warnmsg("Unsupported message format `%s' - if you believe this file contains a virus, report it to bugs@clamav.net\n", mimeSubtype);


			if(mainMessage && (mainMessage != messageIn))
				messageDestroy(mainMessage);
			if(messages)
				free(messages);
			return rc;

		case APPLICATION:
			/*cptr = messageGetMimeSubtype(mainMessage);

			if((strcasecmp(cptr, "octet-stream") == 0) ||
			   (strcasecmp(cptr, "x-msdownload") == 0)) {*/
			{
				fb = messageToFileblob(mainMessage, dir);

				if(fb) {
					cli_dbgmsg("Saving main message as attachment\n");
					fileblobDestroy(fb);
					if(mainMessage != messageIn) {
						messageDestroy(mainMessage);
						mainMessage = NULL;
					} else
						messageReset(mainMessage);
				}
			} /*else
				cli_warnmsg("Discarded application not sent as attachment\n");*/
			break;

		case AUDIO:
		case VIDEO:
		case IMAGE:
			break;

		default:
			cli_warnmsg("Message received with unknown mime encoding");
			break;
		}
	}

	if(aText && (textIn == NULL)) {
		/* Look for a bounce in the text (non mime encoded) portion */
		const text *t;

		for(t = aText; t; t = t->t_next) {
			const line_t *l = t->t_line;
			const text *lookahead, *topofbounce;
			const char *s;
			bool inheader;

			if(l == NULL)
				continue;

			s = lineGetData(l);

			if(cli_filetype(s, strlen(s)) != CL_TYPE_MAIL)
				continue;

			/*
			 * We've found what looks like the start of a bounce
			 * message. Only bother saving if it really is a bounce
			 * message, this helps to speed up scanning of ping-pong
			 * messages that have lots of bounces within bounces in
			 * them
			 */
			for(lookahead = t->t_next; lookahead; lookahead = lookahead->t_next) {
				l = lookahead->t_line;

				if(l == NULL)
					break;
				s = lineGetData(l);
				if(strncasecmp(s, "Content-Type:", 13) == 0)
					/*
					 * Don't bother with plain/text or
					 * plain/html
					 */
					if(strstr(s, "text/") == NULL)
						/*
						 * Don't bother to save the unuseful
						 * part
						 */
						break;
			}

			if(lookahead && (lookahead->t_line == NULL)) {
				cli_dbgmsg("Non mime part bounce message is not mime encoded, so it will not be scanned\n");
				t = lookahead;
				/* look for next bounce message */
				continue;
			}

			/*
			 * Prescan the bounce message to see if there's likely
			 * to be anything nasty.
			 * This algorithm is hand crafted and may be breakable
			 * so all submissions are welcome. It's best NOT to
			 * remove this however you may be tempted, because it
			 * significantly speeds up the scanning of multiple
			 * bounces (i.e. bounces within many bounces)
			 */
			for(; lookahead; lookahead = lookahead->t_next) {
				l = lookahead->t_line;

				if(l) {
					s = lineGetData(l);
					if((strncasecmp(s, "Content-Type:", 13) == 0) &&
					   (strstr(s, "multipart/") == NULL) &&
					   (strstr(s, "message/rfc822") == NULL) &&
					   (strstr(s, "text/plain") == NULL))
						break;
				}
			}
			if(lookahead == NULL) {
				cli_dbgmsg("cli_mbox: I believe it's plain text which must be clean\n");
				/* nothing here, move along please */
				break;
			}
			if((fb = fileblobCreate()) == NULL)
				break;
			cli_dbgmsg("Save non mime part bounce message\n");
			fileblobSetFilename(fb, dir, "bounce");
			fileblobAddData(fb, (unsigned char *)"Received: by clamd (bounce)\n", 28);

			inheader = TRUE;
			topofbounce = NULL;
			for(;;) {
				l = t->t_line;

				if(l == NULL) {
					if(inheader) {
						inheader = FALSE;
						topofbounce = t;
					}
				} else {
					s = lineGetData(l);
					fileblobAddData(fb, (unsigned char *)s, strlen(s));
				}
				fileblobAddData(fb, (unsigned char *)"\n", 1);
				lookahead = t->t_next;
				if(lookahead == NULL)
					break;
				t = lookahead;
				l = t->t_line;
				if((!inheader) && l) {
					s = lineGetData(l);
					if(cli_filetype(s, strlen(s)) == CL_TYPE_MAIL) {
						cli_dbgmsg("Found the start of another bounce candidate\n");
						break;
					}
				}
			}

			fileblobDestroy(fb);
			if(topofbounce)
				t = topofbounce;
			/*
			 * Don't do this - it slows bugs.txt
			 */
			/*if(mainMessage)
				mainMessage->bounce = NULL;*/
		}
		textDestroy(aText);
		aText = NULL;
	}

	/*
	 * No attachments - scan the text portions, often files
	 * are hidden in HTML code
	 */
	cli_dbgmsg("%d multiparts found\n", multiparts);
	for(i = 0; i < multiparts; i++) {
		fb = messageToFileblob(messages[i], dir);

		if(fb) {
			cli_dbgmsg("Saving multipart %d\n", i);

			fileblobDestroy(fb);
		}
	}

	if(mainMessage) {
		/*
		 * Look for uu-encoded main file
		 */
		const text *t_line;

		if((t_line = uuencodeBegin(mainMessage)) != NULL) {
			cli_dbgmsg("Found uuencoded file\n");

			/*
			 * Main part contains uuencoded section
			 */
			messageSetEncoding(mainMessage, "x-uuencode");

			if((fb = messageToFileblob(mainMessage, dir)) != NULL) {
				if((cptr = fileblobGetFilename(fb)) != NULL)
					cli_dbgmsg("Saving uuencoded message %s\n", cptr);
				fileblobDestroy(fb);
			}
			rc = 1;
		} else if((encodingLine(mainMessage) != NULL) &&
			  ((t_line = bounceBegin(mainMessage)) != NULL)) {
			const text *t, *start;
			/*
			 * Attempt to save the original (unbounced)
			 * message - clamscan will find that in the
			 * directory and call us again (with any luck)
			 * having found an e-mail message to handle.
			 *
			 * This finds a lot of false positives, the
			 * search that a content type is in the
			 * bounce (i.e. it's after the bounce header)
			 * helps a bit.
			 *
			 * messageAddLine
			 * optimisation could help here, but needs
			 * careful thought, do it with line numbers
			 * would be best, since the current method in
			 * messageAddLine of checking encoding first
			 * must remain otherwise non bounce messages
			 * won't be scanned
			 */
			for(t = start = t_line; t; t = t->t_next) {
				char cmd[LINE_LENGTH + 1];
				const char *txt = lineGetData(t->t_line);

				if(txt == NULL)
					continue;
				if(cli_strtokbuf(txt, 0, ":", cmd) == NULL)
					continue;

				switch(tableFind(rfc821Table, cmd)) {
					case CONTENT_TRANSFER_ENCODING:
						if((strstr(txt, "7bit") == NULL) &&
						   (strstr(txt, "8bit") == NULL))
							break;
						continue;
					case CONTENT_DISPOSITION:
						break;
					case CONTENT_TYPE:
						if(strstr(txt, "text/plain") != NULL)
							t = NULL;
						break;
					default:
						if(strcasecmp(cmd, "From") == 0)
							start = t_line;
						else if(strcasecmp(cmd, "Received") == 0)
							start = t_line;
						continue;
				}
				break;
			}
			if(t && ((fb = fileblobCreate()) != NULL)) {
				cli_dbgmsg("Found a bounce message\n");
				fileblobSetFilename(fb, dir, "bounce");
				if(textToFileblob(start, fb) == NULL)
					cli_dbgmsg("Nothing new to save in the bounce message");
				else
					rc = 1;
				fileblobDestroy(fb);
			} else
				cli_dbgmsg("Not found a bounce message\n");
		} else {
			bool saveIt;

			cli_dbgmsg("Not found uuencoded file\n");

			if(messageGetMimeType(mainMessage) == MESSAGE)
				/*
				 * Quick peek, if the encapsulated
				 * message has no
				 * content encoding statement don't
				 * bother saving to scan, it's safe
				 */
				saveIt = (encodingLine(mainMessage) != NULL);
			else if((t_line = encodingLine(mainMessage)) != NULL) {
				/*
				 * Some bounces include the message
				 * body without the headers.
				 * FIXME: Unfortunately this generates a
				 * lot of false positives that a bounce
				 * has been found when it hasn't.
				 */
				if((fb = fileblobCreate()) != NULL) {
					cli_dbgmsg("Found a bounce message with no header at '%s'\n",
						lineGetData(t_line->t_line));
					fileblobSetFilename(fb, dir, "bounce");
					fileblobAddData(fb,
						(const unsigned char *)"Received: by clamd (bounce)\n",
						28);

					fb = textToFileblob(t_line, fb);

					fileblobDestroy(fb);
				}
				saveIt = FALSE;
			} else if(multiparts == 0)
				/*
				 * Save the entire text portion,
				 * since it it may be an HTML file with
				 * a JavaScript virus
				 */
				saveIt = TRUE;
			else
				saveIt = FALSE;

			if(saveIt) {
				cli_dbgmsg("Saving text part to scan\n");
				/*
				 * TODO: May be better to save aText
				 */
				saveTextPart(mainMessage, dir);
				if(mainMessage != messageIn) {
					messageDestroy(mainMessage);
					mainMessage = NULL;
				} else
					messageReset(mainMessage);
				rc = 1;
			}
		}
	} else
		rc = (multiparts) ? 1 : 2;	/* anything saved? */

	if(mainMessage && (mainMessage != messageIn))
		messageDestroy(mainMessage);

	if(messages)
		free(messages);

	cli_dbgmsg("parseEmailBody() returning %d\n", rc);

	return rc;
}

/*
 * Is the current line the start of a new section?
 *
 * New sections start with --boundary
 */
static int
boundaryStart(const char *line, const char *boundary)
{
	char *ptr, *out;
	int rc;
	char buf[LINE_LENGTH + 1];

	if(line == NULL)
		return 0;	/* empty line */

	/*cli_dbgmsg("boundaryStart: line = '%s' boundary = '%s'\n", line, boundary);*/

	if((*line != '-') && (*line != '('))
		return 0;

	if(strchr(line, '-') == NULL)
		return 0;

	if(strlen(line) <= sizeof(buf)) {
		out = NULL;
		ptr = rfc822comments(line, buf);
	} else
		out = ptr = rfc822comments(line, NULL);

	if(ptr == NULL)
		ptr = (char *)line;

	if(*ptr++ != '-') {
		if(out)
			free(out);
		return 0;
	}

	/*
	 * Gibe.B3 is broken, it has:
	 *	boundary="---- =_NextPart_000_01C31177.9DC7C000"
	 * but it's boundaries look like
	 *	------ =_NextPart_000_01C31177.9DC7C000
	 * notice the one too few '-'.
	 * Presumably this is a deliberate exploitation of a bug in some mail
	 * clients.
	 *
	 * The trouble is that this creates a lot of false positives for
	 * boundary conditions, if we're too lax about matches. We do our level
	 * best to avoid these false positives. For example if we have
	 * boundary="1" we want to ensure that we don't break out of every line
	 * that has -1 in it instead of starting --1. This needs some more work.
	 *
	 * Look with and without RFC822 comments stripped, I've seen some
	 * samples where () are taken as comments in boundaries and some where
	 * they're not. Irrespective of whatever RFC2822 says we need to find
	 * viruses in both types of mails
	 */
	if((strstr(ptr, boundary) != NULL) || (strstr(line, boundary) != NULL))
		rc = 1;
	else if(*ptr++ != '-')
		rc = 0;
	else
		rc = (strcasecmp(ptr, boundary) == 0);

	if(out)
		free(out);

	if(rc == 1)
		cli_dbgmsg("boundaryStart: found %s in %s\n", boundary, line);

	return rc;
}

/*
 * Is the current line the end?
 *
 * The message ends with with --boundary--
 */
static int
endOfMessage(const char *line, const char *boundary)
{
	size_t len;

	if(line == NULL)
		return 0;
	/*cli_dbgmsg("endOfMessage: line = '%s' boundary = '%s'\n", line, boundary);*/
	if(*line++ != '-')
		return 0;
	if(*line++ != '-')
		return 0;
	len = strlen(boundary);
	if(strncasecmp(line, boundary, len) != 0)
		return 0;
	/*
	 * Use < rather than == because some broken mails have white
	 * space after the boundary
	 */
	if(strlen(line) < (len + 2))
		return 0;
	line = &line[len];
	if(*line++ != '-')
		return 0;
	return *line == '-';
}

/*
 * Initialise the various lookup tables
 */
static int
initialiseTables(table_t **rfc821Table, table_t **subtypeTable)
{
	const struct tableinit *tableinit;

	/*
	 * Initialise the various look up tables
	 */
	*rfc821Table = tableCreate();
	assert(*rfc821Table != NULL);

	for(tableinit = rfc821headers; tableinit->key; tableinit++)
		if(tableInsert(*rfc821Table, tableinit->key, tableinit->value) < 0) {
			tableDestroy(*rfc821Table);
			*rfc821Table = NULL;
			return -1;
		}

	*subtypeTable = tableCreate();
	assert(*subtypeTable != NULL);

	for(tableinit = mimeSubtypes; tableinit->key; tableinit++)
		if(tableInsert(*subtypeTable, tableinit->key, tableinit->value) < 0) {
			tableDestroy(*rfc821Table);
			tableDestroy(*subtypeTable);
			*rfc821Table = NULL;
			*subtypeTable = NULL;
			return -1;
		}

	return 0;
}

/*
 * If there's a HTML text version use that, otherwise
 * use the first text part, otherwise just use the
 * first one around. HTML text is most likely to include
 * a scripting worm
 *
 * If we can't find one, return -1
 */
static int
getTextPart(message *const messages[], size_t size)
{
	size_t i;
	int textpart = -1;

	for(i = 0; i < size; i++) {
		assert(messages[i] != NULL);
		if(messageGetMimeType(messages[i]) == TEXT) {
			if(strcasecmp(messageGetMimeSubtype(messages[i]), "html") == 0)
				return (int)i;
			textpart = (int)i;
		}
	}
	return textpart;
}

/*
 * strip -
 *	Remove the trailing spaces from a buffer. Don't call this directly,
 * always call strstrip() which is a wrapper to this routine to be used with
 * NUL terminated strings. This code looks a bit strange because of it's
 * heritage from code that worked on strings that weren't necessarily NUL
 * terminated.
 * TODO: rewrite for clamAV
 *
 * Returns it's new length (a la strlen)
 *
 * len must be int not size_t because of the >= 0 test, it is sizeof(buf)
 *	not strlen(buf)
 */
static size_t
strip(char *buf, int len)
{
	register char *ptr;
	register size_t i;

	if((buf == NULL) || (len <= 0))
		return 0;

	i = strlen(buf);
	if(len > (int)(i + 1))
		return i;
	ptr = &buf[--len];

#if	defined(UNIX) || defined(C_LINUX) || defined(C_DARWIN)	/* watch - it may be in shared text area */
	do
		if(*ptr)
			*ptr = '\0';
	while((--len >= 0) && (!isgraph(*--ptr)) && (*ptr != '\n') && (*ptr != '\r'));
#else	/* more characters can be displayed on DOS */
	do
#ifndef	REAL_MODE_DOS
		if(*ptr)	/* C8.0 puts into a text area */
#endif
			*ptr = '\0';
	while((--len >= 0) && ((*--ptr == '\0') || (isspace((int)*ptr))));
#endif
	return((size_t)(len + 1));
}

/*
 * strstrip:
 *	Strip a given string
 */
size_t
strstrip(char *s)
{
	if(s == (char *)NULL)
		return(0);

	return(strip(s, strlen(s) + 1));
}

/*
 * Some broken email headers use ';' at the end of a line to continue
 * to the next line and don't add a leading white space on the next line
 */
static bool
continuationMarker(const char *line)
{
	const char *ptr;

	if(line == NULL)
		return FALSE;

#ifdef	CL_DEBUG
	cli_dbgmsg("continuationMarker(%s)\n", line);
#endif

	if(strlen(line) == 0)
		return FALSE;

	ptr = strchr(line, '\0');

	assert(ptr != NULL);

	while(ptr > line)
		switch(*--ptr) {
			case '\n':
			case '\r':
			case ' ':
			case '\t':
				continue;
			case ';':
				return TRUE;
			default:
				return FALSE;
		}

	return FALSE;
}

static int
parseMimeHeader(message *m, const char *cmd, const table_t *rfc821Table, const char *arg)
{
	char *copy, *p;
	const char *ptr;
	int commandNumber;

	cli_dbgmsg("parseMimeHeader: cmd='%s', arg='%s'\n", cmd, arg);

	copy = rfc822comments(cmd, NULL);
	if(copy) {
		commandNumber = tableFind(rfc821Table, copy);
		free(copy);
	} else
		commandNumber = tableFind(rfc821Table, cmd);

	copy = rfc822comments(arg, NULL);

	if(copy)
		ptr = copy;
	else
		ptr = arg;

	switch(commandNumber) {
		case CONTENT_TYPE:
			/*
			 * Fix for non RFC1521 compliant mailers
			 * that send content-type: Text instead
			 * of content-type: Text/Plain, or
			 * just simply "Content-Type:"
			 */
			if(arg == NULL)
				/*
				 * According to section 4 of RFC1521:
				 * "Note also that a subtype specification is
				 * MANDATORY. There are no default subtypes"
				 *
				 * We have to break this and make an assumption
				 * for the subtype because virus writers and
				 * email client writers don't get it right
				 */
				 cli_warnmsg("Empty content-type received, no subtype specified, assuming text/plain; charset=us-ascii\n");
			else if(strchr(ptr, '/') == NULL)
				/*
				 * Empty field, such as
				 *	Content-Type:
				 * which I believe is illegal according to
				 * RFC1521
				 */
				cli_dbgmsg("Invalid content-type '%s' received, no subtype specified, assuming text/plain; charset=us-ascii\n", ptr);
			else {
				int i;
				char *mimeArgs;	/* RHS of the ; */

				/*
				 * Some clients are broken and
				 * put white space after the ;
				 */
				if(*arg == '/') {
					cli_warnmsg("Content-type '/' received, assuming application/octet-stream\n");
					messageSetMimeType(m, "application");
					messageSetMimeSubtype(m, "octet-stream");
				} else {
					/*
					 * The content type could be in quotes:
					 *	Content-Type: "multipart/mixed"
					 * FIXME: this is a hack in that ignores
					 *	the quotes, it doesn't handle
					 *	them properly
					 */
					while(isspace(*ptr))
						ptr++;
					if(ptr[0] == '\"')
						ptr++;

					if(ptr[0] != '/') {
						char *s;
						char *mimeType;	/* LHS of the ; */
#ifdef CL_THREAD_SAFE
						char *strptr;
#endif

						s = mimeType = cli_strtok(ptr, 0, ";");
						/*
						 * Handle
						 * Content-Type: foo/bar multipart/mixed
						 * and
						 * Content-Type: multipart/mixed foo/bar
						 */
						if(s && *s) for(;;) {
#ifdef	CL_THREAD_SAFE
							int set = messageSetMimeType(m, strtok_r(s, "/", &strptr));
#else
							int set = messageSetMimeType(m, strtok(s, "/"));
#endif

							/*
							 * Stephen White <stephen@earth.li>
							 * Some clients put space after
							 * the mime type but before
							 * the ;
							 */
#ifdef	CL_THREAD_SAFE
							s = strtok_r(NULL, ";", &strptr);
#else
							s = strtok(NULL, ";");
#endif
							if(s == NULL)
								break;
							if(set) {
								size_t len = strstrip(s) - 1;
								if(s[len] == '\"') {
									s[len] = '\0';
									len = strstrip(s);
								}
								if(len) {
									if(strchr(s, ' ')) {
										char *t = cli_strtok(s, 0, " ");

										messageSetMimeSubtype(m, t);
										free(t);
									} else
										messageSetMimeSubtype(m, s);
								}
							}

							while(*s && !isspace(*s))
								s++;
							if(*s++ == '\0')
								break;
							if(*s == '\0')
								break;
						}
						if(mimeType)
							free(mimeType);
					}
				}

				/*
				 * Add in all rest of the the arguments.
				 * e.g. if the header is this:
				 * Content-Type:', arg='multipart/mixed; boundary=foo
				 * we find the boundary argument set it
				 */
				i = 1;
				while((mimeArgs = cli_strtok(ptr, i++, ";")) != NULL) {
					cli_dbgmsg("mimeArgs = '%s'\n", mimeArgs);

					messageAddArguments(m, mimeArgs);
					free(mimeArgs);
				}
			}
			break;
		case CONTENT_TRANSFER_ENCODING:
			messageSetEncoding(m, ptr);
			break;
		case CONTENT_DISPOSITION:
			p = cli_strtok(ptr, 0, ";");
			if(p) {
				if(*p) {
					messageSetDispositionType(m, p);
					free(p);
					p = cli_strtok(ptr, 1, ";");
					messageAddArgument(m, p);
				}
				free(p);
			}
	}
	if(copy)
		free(copy);

	return 0;
}

/*
 * Save the text portion of the message
 */
static void
saveTextPart(message *m, const char *dir)
{
	fileblob *fb;

	messageAddArgument(m, "filename=textportion");
	if((fb = messageToFileblob(m, dir)) != NULL) {
		/*
		 * Save main part to scan that
		 */
		cli_dbgmsg("Saving main message\n");

		fileblobDestroy(fb);
	}
}

/*
 * Handle RFC822 comments in headers.
 * If out == NULL, return a buffer without the comments, the caller must free
 *	the returned buffer
 * Return NULL on error or if the input * has no comments.
 * See secion 3.4.3 of RFC822
 * TODO: handle comments that go on to more than one line
 */
static char *
rfc822comments(const char *in, char *out)
{
	const char *iptr;
	char *optr;
	int backslash, inquote, commentlevel;

	if(in == NULL)
		return NULL;

	if(strchr(in, '(') == NULL)
		return NULL;

	assert(out != in);

	if(out == NULL) {
		out = cli_malloc(strlen(in) + 1);
		if(out == NULL)
			return NULL;
	}

	backslash = commentlevel = inquote = 0;
	optr = out;

	cli_dbgmsg("rfc822comments: contains a comment\n");

	for(iptr = in; *iptr; iptr++)
		if(backslash) {
			if(commentlevel == 0)
				*optr++ = *iptr;
			backslash = 0;
		} else switch(*iptr) {
			case '\\':
				backslash = 1;
				break;
			case '\"':
				*optr++ = '\"';
				inquote = !inquote;
				break;
			case '(':
				if(inquote)
					*optr++ = '(';
				else
					commentlevel++;
				break;
			case ')':
				if(inquote)
					*optr++ = ')';
				else if(commentlevel > 0)
					commentlevel--;
				break;
			default:
				if(commentlevel == 0)
					*optr++ = *iptr;
		}

	if(backslash)	/* last character was a single backslash */
		*optr++ = '\\';
	*optr = '\0';

	/*strstrip(out);*/

	cli_dbgmsg("rfc822comments '%s'=>'%s'\n", in, out);

	return out;
}

/*
 * Handle RFC2047 encoding. Returns a malloc'd buffer that the caller must
 * free, or NULL on error
 */
static char *
rfc2047(const char *in)
{
	char *out, *pout;
	size_t len;

	if((strstr(in, "=?") == NULL) || (strstr(in, "?=") == NULL))
		return strdup(in);

	cli_dbgmsg("rfc2047 '%s'\n", in);
	out = cli_malloc(strlen(in) + 1);

	if(out == NULL)
		return NULL;

	pout = out;

	/* For each RFC2047 string */
	while(*in) {
		char encoding, *ptr, *enctext;
		message *m;
		blob *b;

		/* Find next RFC2047 string */
		while(*in) {
			if((*in == '=') && (in[1] == '?')) {
				in += 2;
				break;
			}
			*pout++ = *in++;
		}
		/* Skip over charset, find encoding */
		while((*in != '?') && *in)
			in++;
		if(*in == '\0')
			break;
		encoding = *++in;
		encoding = tolower(encoding);

		if((encoding != 'q') && (encoding != 'b')) {
			cli_warnmsg("Unsupported RFC2047 encoding type '%c' - if you believe this file contains a virus that was missed, report it to bugs@clamav.net\n", encoding);
			free(out);
			out = NULL;
			break;
		}
		/* Skip to encoded text */
		if(*++in != '?')
			break;
		if(*++in == '\0')
			break;

		enctext = strdup(in);
		if(enctext == NULL) {
			free(out);
			out = NULL;
			break;
		}
		in = strstr(in, "?=");
		if(in == NULL) {
			free(enctext);
			break;
		}
		in += 2;
		ptr = strstr(enctext, "?=");
		assert(ptr != NULL);
		*ptr = '\0';
		/*cli_dbgmsg("Need to decode '%s' with method '%c'\n", enctext, encoding);*/

		m = messageCreate();
		if(m == NULL)
			break;
		messageAddStr(m, enctext);
		free(enctext);
		switch(encoding) {
			case 'q':
				messageSetEncoding(m, "quoted-printable");
				break;
			case 'b':
				messageSetEncoding(m, "base64");
				break;
		}
		b = messageToBlob(m);
		len = blobGetDataSize(b);
		cli_dbgmsg("Decoded as '%*.*s'\n", len, len, blobGetData(b));
		memcpy(pout, blobGetData(b), len);
		blobDestroy(b);
		messageDestroy(m);
		if(pout[len - 1] == '\n')
			pout += len - 1;
		else
			pout += len;

	}
	if(out == NULL)
		return NULL;

	*pout = '\0';

	cli_dbgmsg("rfc2047 returns '%s'\n", out);
	return out;
}

#ifdef	PARTIAL_DIR
/*
 * Handle partial messages
 */
static int
rfc1341(message *m, const char *dir)
{
	fileblob *fb;
	char *arg, *id, *number, *total, *oldfilename;
	const char *tmpdir;
	char pdir[NAME_MAX + 1];

	id = (char *)messageFindArgument(m, "id");
	if(id == NULL)
		return -1;

#ifdef  CYGWIN
	if((tmpdir = getenv("TEMP")) == (char *)NULL)
		if((tmpdir = getenv("TMP")) == (char *)NULL)
			if((tmpdir = getenv("TMPDIR")) == (char *)NULL)
				tmpdir = "C:\\";
#else
	if((tmpdir = getenv("TMPDIR")) == (char *)NULL)
		if((tmpdir = getenv("TMP")) == (char *)NULL)
			if((tmpdir = getenv("TEMP")) == (char *)NULL)
#ifdef	P_tmpdir
				tmpdir = P_tmpdir;
#else
				tmpdir = "/tmp";
#endif
#endif

	snprintf(pdir, sizeof(pdir) - 1, "%s/clamav-partial", tmpdir);

	if((mkdir(pdir, 0700) < 0) && (errno != EEXIST)) {
		cli_errmsg("Can't create the directory '%s'\n", pdir);
		return -1;
	} else {
		struct stat statb;

		if(stat(pdir, &statb) < 0) {
			cli_errmsg("Can't stat the directory '%s'\n", pdir);
			return -1;
		}
		if(statb.st_mode & 077)
			cli_warnmsg("Insecure partial directory %s (mode 0%o)\n",
				pdir, statb.st_mode & 0777);
	}

	number = (char *)messageFindArgument(m, "number");
	if(number == NULL) {
		free(id);
		return -1;
	}

	oldfilename = (char *)messageFindArgument(m, "filename");
	if(oldfilename == NULL)
		oldfilename = (char *)messageFindArgument(m, "name");

	arg = cli_malloc(10 + strlen(id) + strlen(number));
	sprintf(arg, "filename=%s%s", id, number);
	messageAddArgument(m, arg);
	free(arg);

	if(oldfilename) {
		cli_warnmsg("Must reset to %s\n", oldfilename);
		free(oldfilename);
	}

	if((fb = messageToFileblob(m, pdir)) == NULL) {
		free(id);
		free(number);
		return -1;
	}

	fileblobDestroy(fb);

	total = (char *)messageFindArgument(m, "total");
	cli_dbgmsg("rfc1341: %s, %s of %s\n", id, number, (total) ? total : "?");
	if(total) {
		int n = atoi(number);
		int t = atoi(total);
		DIR *dd = NULL;

		free(total);
		/*
		 * If it's the last one - reassemble it
		 * FIXME: this assumes that we receive the parts in order
		 */
		if((n == t) && ((dd = opendir(pdir)) != NULL)) {
			FILE *fout;
			char outname[NAME_MAX + 1];

			snprintf(outname, sizeof(outname) - 1, "%s/%s", dir, id);

			cli_dbgmsg("outname: %s\n", outname);

			fout = fopen(outname, "wb");
			if(fout == NULL) {
				cli_errmsg("Can't open '%s' for writing", outname);
				free(id);
				free(number);
				closedir(dd);
				return -1;
			}

			for(n = 1; n <= t; n++) {
				char filename[NAME_MAX + 1];
				const struct dirent *dent;
#if defined(HAVE_READDIR_R_3) || defined(HAVE_READDIR_R_2)
				union {
					struct dirent d;
					char b[offsetof(struct dirent, d_name) + NAME_MAX + 1];
				} result;
#endif

				snprintf(filename, sizeof(filename), "%s%d", id, n);

#ifdef HAVE_READDIR_R_3
				while((readdir_r(dd, &result.d, &dent) == 0) && dent) {
#elif defined(HAVE_READDIR_R_2)
				while((dent = (struct dirent *)readdir_r(dd, &result.d))) {
#else	/*!HAVE_READDIR_R*/
				while((dent = readdir(dd))) {
#endif
					FILE *fin;
					char buffer[BUFSIZ];
					int nblanks;
					extern short cli_leavetemps_flag;

					if(dent->d_ino == 0)
						continue;

					if(strncmp(filename, dent->d_name, strlen(filename)) != 0)
						continue;

					sprintf(filename, "%s/%s", pdir, dent->d_name);
					fin = fopen(filename, "rb");
					if(fin == NULL) {
						cli_errmsg("Can't open '%s' for reading", filename);
						fclose(fout);
						unlink(outname);
						free(id);
						free(number);
						closedir(dd);
						return -1;
					}
					nblanks = 0;
					while(fgets(buffer, sizeof(buffer), fin) != NULL)
						/*
						 * Ensure that trailing newlines
						 * aren't copied
						 */
						if(buffer[0] == '\n')
							nblanks++;
						else {
							if(nblanks)
								do
									putc('\n', fout);
								while(--nblanks > 0);
							fputs(buffer, fout);
						}
					fclose(fin);

					/* don't unlink if leave temps */
					if(!cli_leavetemps_flag)
						unlink(filename);
					break;
				}
				rewinddir(dd);
			}
			closedir(dd);
			fclose(fout);
		}
	}
	free(number);
	free(id);

	return 0;
}
#endif

#if	defined(FOLLOWURLS) && (FOLLOWURLS > 0)
static void
checkURLs(message *m, const char *dir)
{
	blob *b = messageToBlob(m);
	size_t len;
	table_t *t;
	int i, n;
#if	defined(WITH_CURL) && defined(CL_THREAD_SAFE)
	pthread_t tid[FOLLOWURLS];
	struct arg args[FOLLOWURLS];
#endif
	tag_arguments_t hrefs;

	if(b == NULL)
		return;

	len = blobGetDataSize(b);

	if(len == 0) {
		blobDestroy(b);
		return;
	}

	/* TODO: make this size customisable */
	if(len > 100*1024) {
		cli_warnmsg("Viruses pointed to by URL not scanned in large message\n");
		blobDestroy(b);
		return;
	}

	blobClose(b);
	t = tableCreate();
	if(t == NULL) {
		blobDestroy(b);
		return;
	}

	hrefs.count = 0;
	hrefs.tag = hrefs.value = NULL;

	cli_dbgmsg("checkURLs: calling html_normalise_mem\n");
	if(!html_normalise_mem(blobGetData(b), len, NULL, &hrefs)) {
		blobDestroy(b);
		tableDestroy(t);
		return;
	}
	cli_dbgmsg("checkURLs: html_normalise_mem returned\n");

	/* TODO: Do we need to call remove_html_comments? */

	n = 0;

	for(i = 0; i < hrefs.count; i++) {
		const char *url = (const char *)hrefs.value[i];

		if(strncasecmp("http://", url, 7) == 0) {
			char *ptr;
#ifdef	WITH_CURL
#ifndef	CL_THREAD_SAFE
			struct arg arg;
#endif

#else	/*!WITH_CURL*/
#ifdef	CL_THREAD_SAFE
			static pthread_mutex_t system_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
			struct stat statb;
			char cmd[512];
#endif	/*WITH_CURL*/
			char name[NAME_MAX + 1];

			if(tableFind(t, url) == 1) {
				cli_dbgmsg("URL %s already downloaded\n", url);
				continue;
			}
			if(n == FOLLOWURLS) {
				cli_warnmsg("Not all URLs will be scanned\n");
				break;
			}
			(void)tableInsert(t, url, 1);
			cli_dbgmsg("Downloading URL %s to be scanned\n", url);
			strncpy(name, url, sizeof(name) - 1);
			name[sizeof(name) - 1] = '\0';
			for(ptr = name; *ptr; ptr++)
				if(*ptr == '/')
					*ptr = '_';

#ifdef	WITH_CURL
#ifdef	CL_THREAD_SAFE
			args[n].dir = dir;
			args[n].url = url;
			args[n].filename = strdup(name);
			pthread_create(&tid[n], NULL, getURL, &args[n]);
#else
			arg.url = url;
			arg.dir = dir;
			arg.filename = name;
			getURL(&arg);
#endif

#else
			/*
			 * TODO: maximum size and timeouts
			 */
			len = sizeof(cmd) - 26 - strlen(dir) - strlen(name);
#ifdef	CL_DEBUG
			snprintf(cmd, sizeof(cmd) - 1, "GET -t10 %.*s >%s/%s", len, url, dir, name);
#else
			snprintf(cmd, sizeof(cmd) - 1, "GET -t10 %.*s >%s/%s 2>/dev/null", len, url, dir, name);
#endif
			cmd[sizeof(cmd) - 1] = '\0';

#ifndef	WITH_CURL
			for(ptr = cmd; *ptr; ptr++)
				if(strchr(";&", *ptr))
					*ptr = '_';
#endif

			cli_dbgmsg("%s\n", cmd);
#ifdef	CL_THREAD_SAFE
			pthread_mutex_lock(&system_mutex);
#endif
			system(cmd);
#ifdef	CL_THREAD_SAFE
			pthread_mutex_unlock(&system_mutex);
#endif
			snprintf(cmd, sizeof(cmd), "%s/%s", dir, name);
			if(stat(cmd, &statb) >= 0)
				if(statb.st_size == 0) {
					cli_warnmsg("URL %s failed to download\n", url);
					/*
					 * Don't bother scanning an empty file
					 */
					(void)unlink(cmd);
				}
#endif
			++n;
		}
	}
	blobDestroy(b);
	tableDestroy(t);

#if	defined(WITH_CURL) && defined(CL_THREAD_SAFE)
	assert(n <= FOLLOWURLS);
	cli_dbgmsg("checkURLs: waiting for %d thread(s) to finish\n", n);
	while(--n >= 0) {
		pthread_join(tid[n], NULL);
		free(args[n].filename);
	}
#endif
	html_tag_arg_free(&hrefs);
}

#ifdef	WITH_CURL
static void *
#ifdef	CL_THREAD_SAFE
getURL(void *a)
#else
getURL(struct arg *arg)
#endif
{
	CURL *curl;
	FILE *fp;
	struct curl_slist *headers;
	static int initialised = 0;
#ifdef	CL_THREAD_SAFE
	static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct arg *arg = (struct arg *)a;
#endif
	const char *url = arg->url;
	const char *dir = arg->dir;
	const char *filename = arg->filename;
	char fout[NAME_MAX + 1];
#ifdef	CURLOPT_ERRORBUFFER
	char errorbuffer[128];
#endif

#ifdef	CL_THREAD_SAFE
	pthread_mutex_lock(&init_mutex);
#endif
	if(!initialised) {
		if(curl_global_init(CURL_GLOBAL_NOTHING) != 0) {
#ifdef	CL_THREAD_SAFE
			pthread_mutex_unlock(&init_mutex);
#endif
			return NULL;
		}
		initialised = 1;
	}
#ifdef	CL_THREAD_SAFE
	pthread_mutex_unlock(&init_mutex);
#endif

	/* easy isn't the word I'd use... */
	curl = curl_easy_init();
	if(curl == NULL)
		return NULL;

	(void)curl_easy_setopt(curl, CURLOPT_USERAGENT, "www.clamav.net");

	if(curl_easy_setopt(curl, CURLOPT_URL, url) != 0)
		return NULL;

	snprintf(fout, NAME_MAX, "%s/%s", dir, filename);

	fp = fopen(fout, "w");

	if(fp == NULL) {
		cli_errmsg("Can't open '%s' for writing", fout);
		curl_easy_cleanup(curl);
		return NULL;
	}
#ifdef	CURLOPT_WRITEDATA
	if(curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp) != 0) {
		fclose(fp);
		curl_easy_cleanup(curl);
		return NULL;
	}
#else
	if(curl_easy_setopt(curl, CURLOPT_FILE, fp) != 0) {
		fclose(fp);
		curl_easy_cleanup(curl);
		return NULL;
	}
#endif

	/*
	 * If an item is in squid's cache get it from there (TCP_HIT/200)
	 * by default curl doesn't (TCP_CLIENT_REFRESH_MISS/200)
	 */
	headers = curl_slist_append(NULL, "Pragma:");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	/* These should be customisable */
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
#ifdef	CURLOPT_MAXFILESIZE
	curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 50*1024);
#endif

#ifdef  CL_THREAD_SAFE
#ifdef	CURLOPT_DNS_USE_GLOBAL_CACHE
	curl_easy_setopt(curl, CURLOPT_DNS_USE_GLOBAL_CACHE, 0);
#endif
#endif

	/*
	 * Prevent password: prompting with older versions
	 * FIXME: a better username?
	 */
	curl_easy_setopt(curl, CURLOPT_USERPWD, "username:password");

#ifdef	CURLOPT_ERRORBUFFER
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuffer);
#endif

	/*
	 * FIXME: valgrind reports "pthread_mutex_unlock: mutex is not locked"
	 * from gethostbyaddr_r within this. It may be a bug in libcurl
	 * rather than this code, but I need to check, see Curl_resolv()
	 * If pushed really hard it will sometimes say
	 * Conditional jump or move depends on uninitialised value(s) and
	 * quit. But the program seems to work OK without valgrind...
	 * Perhaps Curl_resolv() isn't thread safe?
	 */
	/*
	 * On some C libraries (notably with FC3, glibc-2.3.3-74) you get a
	 * memory leak * here in getaddrinfo(), see
	 *	https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=139559
	 */

	if(curl_easy_perform(curl) != CURLE_OK) {
#ifdef	CURLOPT_ERRORBUFFER
		cli_warnmsg("URL %s failed to download: %s\n", url, errorbuffer);
#else
		cli_warnmsg("URL %s failed to download\n", url);
#endif
	}

	fclose(fp);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return NULL;
}
#endif

#else
static void
checkURLs(message *m, const char *dir)
{
}
#endif

#ifdef HAVE_BACKTRACE
static void
sigsegv(int sig)
{
	signal(SIGSEGV, SIG_DFL);
	print_trace(1);
	exit(SIGSEGV);
}

static void
print_trace(int use_syslog)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	pid_t pid = getpid();

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	if(use_syslog == 0)
		cli_dbgmsg("Backtrace of pid %d:\n", pid);
	else
		syslog(LOG_ERR, "Backtrace of pid %d:", pid);

	for(i = 0; i < size; i++)
		if(use_syslog)
			syslog(LOG_ERR, "bt[%u]: %s", i, strings[i]);
		else
			cli_dbgmsg("%s\n", strings[i]);

	/* TODO: dump the current email */

	free(strings);
}
#endif

static bool
usefulHeader(int commandNumber, const char *cmd)
{
	switch(commandNumber) {
		case CONTENT_TRANSFER_ENCODING:
		case CONTENT_DISPOSITION:
		case CONTENT_TYPE:
			return TRUE;
		default:
			if(strcasecmp(cmd, "From") == 0)
				return TRUE;
			else if(strcasecmp(cmd, "Received") == 0)
				return TRUE;
			else if(strcasecmp(cmd, "De") == 0)
				return TRUE;
	}

	return FALSE;
}

/*
 * Save the uuencoded part of the file as it is read in since there's no need
 * to include it in the parse tree. Saves memory and parse time.
 * Return < 0 for failure
 */
static int
uufasttrack(message *m, const char *firstline, const char *dir, FILE *fin)
{
	fileblob *fb = fileblobCreate();
	char buffer[LINE_LENGTH + 1];
	char *filename = cli_strtok(firstline, 2, " ");

	if(filename == NULL)
		return -1;

	fileblobSetFilename(fb, dir, filename);
	cli_dbgmsg("Fast track uudecode %s\n", filename);
	free(filename);

	while(fgets(buffer, sizeof(buffer) - 1, fin) != NULL) {
		unsigned char data[1024];
		const unsigned char *uptr;
		size_t len;

		cli_chomp(buffer);
		if(strcasecmp(buffer, "end") == 0)
			break;
		if(buffer[0] == '\0')
			break;

		uptr = decodeLine(m, UUENCODE, buffer, data, sizeof(data));
		if(uptr == NULL)
			break;

		len = (size_t)(uptr - data);
		if((len > 62) || (len == 0))
			break;

		if(fileblobAddData(fb, data, len) < 0)
			break;
	}

	fileblobDestroy(fb);
	return 1;
}

/*
 * Like fgets but cope with end of line by "\n", "\r\n", "\n\r", "\r"
 */
static char *
getline_from_mbox(char *buffer, size_t len, FILE *fin)
{
	char *ret;

	if(feof(fin))
		return NULL;

	if((len == 0) || (buffer == NULL)) {
		cli_errmsg("Invalid call to getline_from_mbox(). Report to bugs@clamav.net\n");
		return NULL;
	}

	ret = buffer;

	do {
		int c = getc(fin);

		if(ferror(fin))
			return NULL;

		switch(c) {
			case '\n':
				*buffer++ = '\n';
				c = getc(fin);
				if((c != '\r') && !feof(fin))
					ungetc(c, fin);
				break;
			default:
				*buffer++ = c;
				continue;
			case EOF:
				break;
			case '\r':
				*buffer++ = '\n';
				c = getc(fin);
				if((c != '\n') && !feof(fin))
					ungetc(c, fin);
				break;
		}
		break;
	} while(--len > 0);

	if(len == 0) {
		/* probably, the email breaks RFC821 */
		cli_dbgmsg("getline_from_mbox: buffer overflow stopped\n");
		return NULL;
	}
	*buffer = '\0';

	return ret;
}

#ifdef	NEW_WORLD
/*
 * like cli_memstr - but returns the location of the match
 * FIXME: need a case insensitive version
 */
static const char *
cli_pmemstr(const char *haystack, size_t hs, const char *needle, size_t ns)
{
	const char *pt, *hay;
	size_t n;

	if(haystack == needle)
		return haystack;

	if(hs < ns)
		return NULL;

	if(memcmp(haystack, needle, ns) == 0)
		return haystack;

	pt = hay = haystack;
	n = hs;

	while((pt = memchr(hay, needle[0], n)) != NULL) {
		n -= (int) pt - (int) hay;
		if(n < ns)
			break;

		if(memcmp(pt, needle, ns) == 0)
			return pt;

		if(hay == pt) {
			n--;
			hay++;
		} else
			hay = pt;
	}

	return NULL;
}
#endif
