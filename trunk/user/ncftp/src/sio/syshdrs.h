/* syshdrs.h
 *
 * Copyright (c) 1996-2004 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	pragma once
#	pragma warning(disable : 4127)	// warning C4127: conditional expression is constant
#	pragma warning(disable : 4100)	// warning C4100: 'lpReserved' : unreferenced formal parameter
#	pragma warning(disable : 4514)	// warning C4514: unreferenced inline function has been removed
#	pragma warning(disable : 4115)	// warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#	pragma warning(disable : 4201)	// warning C4201: nonstandard extension used : nameless struct/union
#	pragma warning(disable : 4214)	// warning C4214: nonstandard extension used : bit field types other than int
#	pragma warning(disable : 4115)	// warning C4115: 'IRpcStubBuffer' : named type definition in parentheses
#	pragma warning(disable : 4711)	// warning C4711: function selected for automatic inline expansion
	/* Include "wincfg.h" in place of "config.h" */
#	include "wincfg.h"
#	define WINVER 0x0400
#	define _WIN32_WINNT 0x0400
#	include <windows.h>		/* includes <winsock2.h> if _WIN32_WINNT >= 0x400 */
#	include <io.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <stdarg.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <time.h>
#	include <fcntl.h>
#	define strcasecmp stricmp
#	define strncasecmp strnicmp
#	define sleep(a) Sleep(a * 1000)
#	ifndef S_ISREG
#		define S_ISREG(m)      (((m) & _S_IFMT) == _S_IFREG)
#		define S_ISDIR(m)      (((m) & _S_IFMT) == _S_IFDIR)
#	endif
#	ifndef open
#		define open _open
#		define write _write
#		define read _read
#		define close _close
#		define lseek _lseek
#		define stat _stat
#		define fstat _fstat
#		define dup _dup
#	endif
#	ifndef unlink
#		define unlink remove
#	endif
#	define MY_FD_ZERO FD_ZERO
#	define MY_FD_SET(s,set) FD_SET((SOCKET) (s), set)
#	define MY_FD_CLR(s,set) FD_CLR((SOCKET) (s), set)
#	define MY_FD_ISSET FD_ISSET
#	define NO_SIGNALS 1
#	define NO_UNIX_DOMAIN_SOCKETS 1
#else /* ---------------------------- UNIX ---------------------------- */
#	if defined(HAVE_CONFIG_H)
#		include <config.h>
#	endif
#	if defined(AIX) || defined(_AIX) || defined(__HOS_AIX__)
#		define _ALL_SOURCE 1
#	endif
#	ifdef HAVE_UNISTD_H
#		include <unistd.h>
#	endif
#	include <sys/types.h>
#	include <sys/time.h>
#	include <sys/stat.h>
#	include <sys/socket.h>
#	ifdef HAVE_SYS_UN_H
#		include <sys/un.h>
#	endif
#	include <sys/ioctl.h>

#	include <netinet/in_systm.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#	include <netinet/tcp.h>
#	include <arpa/inet.h>
#	include <arpa/telnet.h>
#	include <netdb.h>
#	include <errno.h>
#	include <stdio.h>
#	include <string.h>
#	ifdef HAVE_STRINGS_H
#		include <strings.h>
#	endif
#	include <stddef.h>
#	include <stdlib.h>
#	include <ctype.h>
#	include <signal.h>
#	include <setjmp.h>
#	include <stdarg.h>
#	include <time.h>
#	include <fcntl.h>

#	ifdef HAVE_ALLOCA_H
#		include <alloca.h>
#	endif

#	ifdef HAVE_NET_ERRNO_H
#		include <net/errno.h>
#	endif
#	ifdef HAVE_ARPA_NAMESER_H
#		include <arpa/nameser.h>
#	endif
#	ifdef HAVE_NSERVE_H
#		ifdef SCO
#			undef MAXDNAME
#		endif
#		include <nserve.h>
#	endif
#	ifdef HAVE_RESOLV_H
#		include <resolv.h>
#		ifdef HPUX
			extern int res_init(void);
#		endif
#	endif

#	ifdef CAN_USE_SYS_SELECT_H
#		include <sys/select.h>
#	endif
#	define MY_FD_ZERO FD_ZERO
#	define MY_FD_SET FD_SET
#	define MY_FD_CLR FD_CLR
#	define MY_FD_ISSET FD_ISSET

#	ifndef SETSOCKOPT_ARG4
#		define SETSOCKOPT_ARG4
#		define GETSOCKOPT_ARG4
#	endif
#	if defined(MACOSX) || defined(BSDOS)
#		undef SIG_DFL
#		undef SIG_IGN
#		undef SIG_ERR
#		define SIG_DFL         (void (*)(int))0
#		define SIG_IGN         (void (*)(int))1
#		define SIG_ERR         (void (*)(int))-1
#	endif
#endif /* ---------------------------- UNIX ---------------------------- */

#if (defined(AIX) && (AIX >= 430))
/* AIX 4.3's sys/socket.h doesn't properly prototype these for C */
extern int naccept(int, struct sockaddr *, socklen_t *);
extern int ngetpeername(int, struct sockaddr *, socklen_t *);
extern int ngetsockname(int, struct sockaddr *, socklen_t *);
extern ssize_t nrecvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
extern ssize_t nrecvmsg(int, struct msghdr *, int);
extern ssize_t nsendmsg(int, const struct msghdr *, int);
#endif


#ifndef IAC

/*
 * Definitions for the TELNET protocol.
 */
#define IAC     255             /* interpret as command: */
#define DONT    254             /* you are not to use option */
#define DO      253             /* please, you use option */
#define WONT    252             /* I won't use option */
#define WILL    251             /* I will use option */
#define SB      250             /* interpret as subnegotiation */
#define GA      249             /* you may reverse the line */
#define EL      248             /* erase the current line */
#define EC      247             /* erase the current character */
#define AYT     246             /* are you there */
#define AO      245             /* abort output--but let prog finish */
#define IP      244             /* interrupt process--permanently */
#define BREAK   243             /* break */
#define DM      242             /* data mark--for connect. cleaning */
#define NOP     241             /* nop */
#define SE      240             /* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */
#define ABORT   238             /* Abort process */
#define SUSP    237             /* Suspend process */
#define xEOF    236             /* End of file: EOF is already used... */

#define SYNCH   242             /* for telfunc calls */
#endif

#if ((defined(SIGALRM)) && (defined(SIGPIPE)))
#	define UNIX_SIGNALS 1
#endif

/* Private decl; only for use when compiling sio code. */
#ifdef HAVE_SIGSETJMP
#	define SSetjmp(a) sigsetjmp(a, 1)
#	define SLongjmp(a,b) siglongjmp(a, b)
#	define Sjmp_buf sigjmp_buf
#else
#	define SSetjmp(a) setjmp(a)
#	define SLongjmp(a,b) longjmp(a, b)
#	define Sjmp_buf jmp_buf
#endif

#include "sio.h"			/* Library header. */
#ifdef HAVE_UNIX_DOMAIN_SOCKETS
#	include "usio.h"
#endif

#if (defined(SOCKS)) && (SOCKS >= 5)
#	ifdef HAVE_SOCKS_H
#		ifdef HAVE_SOCKS5P_H
#			define INCLUDE_PROTOTYPES 1
#		endif
#		include <socks.h>
#	endif
#endif	/* SOCKS */

/* eof */
