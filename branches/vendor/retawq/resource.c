/* retawq/resource.c - resource handling (network, cache, DNS, ...)
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#include "stuff.h"
#include "resource.h"
#include "parser.h"

#if CAN_HANDLE_SIGNALS
#include <signal.h>
#endif

#if HAVE_DIRENT_H
#include <dirent.h>
#else
#define dirent direct
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if OPTION_TLS == TLS_GNUTLS

#include <gnutls/gnutls.h>

/* try to resolve identifier brain-damage... */
#define DO_GNUTLS_SUFFIX ( (GNUTLS_MAJOR_VERSION > 1) || \
  ( (GNUTLS_MAJOR_VERSION == 1) && (GNUTLS_MINOR_VERSION > 1) ) || \
  ( (GNUTLS_MAJOR_VERSION == 1) && (GNUTLS_MINOR_VERSION == 1) && \
    (GNUTLS_MICRO_VERSION >= 11) ) )

#if DO_GNUTLS_SUFFIX
typedef gnutls_session_t tTlsSession;
typedef gnutls_transport_ptr_t my_gnutls_transport_ptr;
#else
typedef gnutls_session tTlsSession;
typedef gnutls_transport_ptr my_gnutls_transport_ptr;
#endif

#elif OPTION_TLS == TLS_OPENSSL

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
typedef SSL* tTlsSession;

#elif OPTION_TLS == TLS_MATRIX

#include <matrixSsl.h>
typedef ssl_t* tTlsSession;

#elif OPTION_TLS == TLS_BUILTIN

struct __tTlsSession;
typedef struct __tTlsSession* tTlsSession;

#elif OPTION_TLS

#error "Bad OPTION_TLS value"

#endif

declare_local_i18n_buffer

#if OPTION_IPV6 && defined(AF_INET6)
/* "user wants" && "library can" */
#define USE_IPv6 1
#else
#define USE_IPv6 0
#endif


/** Strings */

static char strbuf[STRBUF_SIZE], strbuf2[STRBUF_SIZE];
#if CONFIG_DEBUG
static char debugstrbuf[STRBUF_SIZE];
#endif

static /*@observer@*/ const char strTcp[] = "tcp", strPop3[] = "pop3",
  strPercsColonPercs[] = "%s:%s", strNewlineBr[] ="\n<br>",
  strEndpNewlineEndBodyHtml[] = "</p>\n</body></html>",
  strAxwfu[] = "application/x-www-form-urlencoded",
  strHtmlPageTitle[] = "<html><head><title>%s</title></head>\n"
    "<body>\n<h3 align=\"center\"><b>%s</b></h3>\n";
#define strBr (strNewlineBr + 1)                              /* ug */
#define strNewlineEndBodyHtml (strEndpNewlineEndBodyHtml + 4) /* ly */
#define strEndBodyHtml (strNewlineEndBodyHtml + 1)            /* :-) */

const char strCrlf[] = "\r\n";
#if CONFIG_FTP
const char strList[] = "LIST", strRetr[] = "RETR";
#endif

#if CONFIG_FTP || OPTION_POP
static const char strNetcmdUser[] = "USER %s\r\n",
  strNetcmdPass[] = "PASS %s\r\n";
#endif
#if (CONFIG_HTTP & HTTP_AUTH_DIGEST) || OPTION_COOKIES || OPTION_BUILTIN_DNS
static const char strDomain[] = "domain";
#endif

static const char strStopped[] = N_("Stopped"), strError[] = N_("error"),
  strNoError[] = N_("(no error)"), strInitialization[] = N_("initialization"),
  strProxyAuthentReq[] = N_("Proxy authentication required");
const char strRedirection[] = N_("redirection");

const char* const strResourceState[MAX_RESOURCE_STATE + 1] =
{ N_("(resource state unknown - internal error)"), strError,
  N_("Connecting"), N_("Exchanging messages"), N_("Receiving"),
  N_("Document complete"), strStopped
};

const char* const strResourceError[MAX_RESOURCE_ERROR + 1] =
{ strNoError, N_("Bad request kind (internal error)"),
  N_("Can't create socket"), N_("Connection failed"),
  N_("Connection refused"), N_("Specific network unreachable"),
  N_("Can't lookup host name"), N_("Can't read/write file/directory"),
  N_("Bad URL"), N_("Server closed connection"), N_("Internal handshake bug"),
  N_("Port number invalid"), N_("Unknown scheme/protocol"),
  N_("Time-out (server busy)"), N_("Server response invalid"),
  strLoginFailed, N_("Redirection problem"),
  N_("Can't calculate random numbers"), N_("TLS-related error"),
  strProxyAuthentReq, N_("Can't create internal pipe"),
  N_("Can't create new process"), N_("Forbidden by configuration"),
  N_("Can't execute program"), N_("Scheme/protocol disabled"),
  N_("Hostname missing"), N_("Too many open file descriptors"),
  N_("Connection reset by peer")
};

#if OPTION_TLS
const char* const strTlsError[MAX_TLS_ERROR + 1] =
{ strNoError, strUnknown, N_("session expired"), N_("fatal alert"),
  N_("file"), N_("application data"), N_("memory"), N_("ciphers"),
  N_("(de-)compression"), N_("(de-)cryption"), "SRP", N_("public key"),
  N_("packet"), N_("library feature unimplemented"), N_("handshake"),
  strInitialization
};
#endif

const char* const strResourceRequestState[MAX_RESOURCE_REQUEST_STATE + 1] =
{ N_("(request state unknown - internal error)"), strError,
  N_("Preparing"), N_("(Attached resource)"), strStopped,
  N_("Looking up host name")
};


/** Resource protocols */

#define RPCFG(cfg, rp) (((cfg) * (rp)) + ((1 - (cfg)) * rpDisabled)) /* :-) */

const tRpData rp_data[rpMax + 1] =
{ { strUnknown, rpUnknown, rpfUsesAuthority },
  { strBracedDisabled, rpDisabled, rpfNone },
  { strHttp, rpHttp, rpfIsHttplike | rpfUsesAuthority | rpfIsPathHierarchical |
    rpfUiToupper },
  { strFtp, RPCFG((CONFIG_FTP ? 1 : 0), __rpFtp), rpfIsFtplike |
    rpfUsesAuthority | rpfIsPathHierarchical | rpfUiToupper },
  { strLocal, rpLocal, rpfIsLocallike | rpfIsPathHierarchical },
  { strAbout, rpAbout, rpfNone },
  { strFinger, RPCFG(CONFIG_FINGER, __rpFinger), rpfUsesAuthority },
  { strLocalCgi, RPCFG(OPTION_LOCAL_CGI, __rpLocalCgi), rpfIsLocallike |
    rpfIsPathHierarchical },
  { strNews, RPCFG(OPTION_NEWS, __rpNntp), rpfUsesAuthority | rpfUiAlternate |
    rpfUiToupper },
  { strPop, RPCFG(OPTION_POP, __rpPop), rpfUsesAuthority | rpfUiAlternate |
    rpfUiToupper | rpfIsPoplike },
  { strPops, RPCFG((OPTION_TLS ? 1 : 0) * OPTION_POP, __rpPops),
    rpfUsesAuthority | rpfIsPoplike | rpfIsTlslike },
  { strCvs, rpCvs, rpfUsesAuthority | rpfIsPathHierarchical | rpfUiToupper },
  { strGopher, RPCFG(CONFIG_GOPHER, __rpGopher), rpfUsesAuthority },
  { strInfo, rpInfo, rpfIsPathHierarchical },
  { strMailto, RPCFG((CONFIG_MAILTO ? 1 : 0), __rpMailto), rpfNone },
  { strJavascript, RPCFG(CONFIG_JAVASCRIPT, __rpJavascript), rpfNone },
  { strHttps, RPCFG((OPTION_TLS ? 1 : 0), __rpHttps), rpfIsHttplike |
    rpfIsTlslike | rpfUsesAuthority | rpfIsPathHierarchical },
  { strFtps, RPCFG(((OPTION_TLS ? 1 : 0) * CONFIG_FTP), __rpFtps),
    rpfIsFtplike | rpfIsTlslike | rpfUsesAuthority | rpfIsPathHierarchical },
  { strExecextShell, RPCFG(((OPTION_EXECEXT & EXECEXT_SHELL) ? 1 : 0),
    __rpExecextShell), rpfNone }
};

#undef RPCFG


/** Protocols, ports 'n' pipes (aka PPP:-) */

static const_after_init int ipprotocolnumber_tcp = IPPROTO_TCP; /* default */

static const_after_init tPortnumber portnumber_http, portnumber_ftp,
  portnumber_finger, portnumber_cvs, portnumber_gopher, portnumber_nntp,
  portnumber_pop3, portnumber_https;
  /* (all these port numbers are stored in network byte order) */

#if CAN_HANDLE_SIGNALS
const_after_init int fd_any2main_read, fd_any2main_write;
#endif
#if OPTION_THREADING
static const_after_init int fd_dns2resource_read, fd_dns2resource_write,
  fd_resource2dns_read, fd_resource2dns_write;
#endif
#if CONFIG_TG == TG_XCURSES
const_after_init int fd_xcurses2main_read, fd_xcurses2main_write;
#endif

#if OPTION_THREADING == 2
static pid_t pid_main = 0, pid_dns = 0; /* PIDs of our threads */
#endif


/** Socket addresses */

#if USE_IPv6
typedef struct sockaddr_storage tSockaddr;
#define HINT_AF (AF_UNSPEC)
#else
typedef struct sockaddr tSockaddr;
#define HINT_AF (AF_INET)
#endif

typedef signed char tSockaddrRating;
#define SOCKADDR_RATING_NONE (-1) /* (for "best rating" algorithms only) */
#define SOCKADDR_RATING_NEW (1) /* for new (not yet existing) sppi entries */
#define SOCKADDR_RATING_MAX (8)

#define MAXNUM_SOCKADDRS (8)
typedef signed char tSockaddrIndex; /* ("signed" for simplicity only) */
#define SOCKADDR_INDEX_INVALID (-1)
typedef unsigned char tSockaddrsBitfield;

my_enum1 enum
{ sppifNone = 0, sppifCannotHttp11 = 0x01, __sppifFtpCannotEpsv = 0x02,
  __sppifNntpCannotPostArticles = 0x04
} my_enum2(unsigned char) tSockaddrPortProtInfoFlags;
#if CONFIG_FTP
#define sppifFtpCannotEpsv (__sppifFtpCannotEpsv)
#endif
#if OPTION_NEWS
#define sppifNntpCannotPostArticles (__sppifNntpCannotPostArticles)
#endif

typedef struct tSockaddrPortProtInfo
{ struct tSockaddrPortProtInfo* next;
  const char* software_id;
  tPortnumber portnumber; /* (in network byte order) */
  tResourceProtocol protocol;
  tSockaddrPortProtInfoFlags sppif;
  tSockaddrRating rating; /* rates how connection attempts worked/failed */
} tSockaddrPortProtInfo;

typedef struct tSockaddrEntry
{ tSockaddr addr;
  struct tSockaddrEntry* next;
  tSockaddrPortProtInfo* sppi;
  size_t addrlen;
  int address_family;
} tSockaddrEntry;

#define HASHTABLESIZE (101) /* (general) */
typedef unsigned short tHashIndex;

#define HASHTABLESIZE_SOCKADDRS (16)
static tSockaddrEntry* sockaddr_entry_head[HASHTABLESIZE_SOCKADDRS];

static tSockaddrEntry** sockaddr2listhead(const tSockaddr* _s, size_t addrlen)
{ const unsigned char* s = (const unsigned char*) _s;
  unsigned char sum = 0;
  while (addrlen > 0) { sum += *s++; addrlen--; } /* get a hash value */
  sum ^= (sum >> 4); /* use all bits, for better hashing */
  return(&(sockaddr_entry_head[sum & 15]));
}

#if USE_S2U

static void sockaddr2uistr(const tSockaddrEntry* entry, /*@out@*/ char* buf,
  size_t size)
/* converts a network address to a UI string and puts that into <buf> */
{ const tSockaddr* addr = &(entry->addr);
  int address_family = entry->address_family;
  *buf = '\0';
  /* Try the library function with the lower bug probability first... */
#if HAVE_INET_NTOP
  if (address_family == AF_INET)
  { const struct sockaddr_in* a = (const struct sockaddr_in*) addr;
    if ( (inet_ntop(address_family, (const void*) (&(a->sin_addr.s_addr)),
          buf, size) != NULL) && (*buf != '\0') )
    { debugmsg("inet_ntop(AF_INET)\n"); return; }
  }
#if USE_IPv6
  else if (address_family == AF_INET6)
  { const struct sockaddr_in6* a = (const struct sockaddr_in6*) addr;
    if ( (inet_ntop(address_family, (const void*) (&(a->sin6_addr.s6_addr)),
          buf, size) != NULL) && (*buf != '\0') )
    { debugmsg("inet_ntop(AF_INET6)\n"); return; }
  }
#endif /* #if USE_IPv6 */
#endif /* #if HAVE_INET_NTOP */
#if HAVE_GETNAMEINFO
  if ( (getnameinfo((const struct sockaddr*) addr, entry->addrlen, buf, size,
    NULL, 0, NI_NUMERICHOST) == 0) && (*buf != '\0') )
  { debugmsg("getnameinfo()\n"); return; }
#endif
}

#else

#define sockaddr2uistr(a, buf, c) *buf = '\0'

#endif

static tSockaddrPortProtInfo* sockaddr2sppi(tSockaddrEntry* entry,
  tPortnumber portnumber, tResourceProtocol rp, tBoolean create_if_null)
{ tSockaddrPortProtInfo* sppi = entry->sppi;
  while (sppi != NULL)
  { if ( (sppi->portnumber == portnumber) && (sppi->protocol == rp) ) break;
    sppi = sppi->next;
  }
  if ( (sppi == NULL) && (create_if_null) )
  { sppi = memory_allocate(sizeof(tSockaddrPortProtInfo), mapPermanent);
    sppi->portnumber = portnumber; sppi->protocol = rp;
    sppi->next = entry->sppi; entry->sppi = sppi;
#if CONFIG_DEBUG
    { char buf[1024];
      sockaddr2uistr(entry, buf, sizeof(buf));
      sprint_safe(debugstrbuf, "sockaddr2sppi(): *%s*, %d, %d\n", buf,
        ntohs(portnumber), rp);
      debugmsg(debugstrbuf);
    }
#endif
  }
  return(sppi);
}


/** DNS lookups */

enum { dlfNone = 0, dlfTryIpv6 = 0x01 };
typedef unsigned char tDnsLookupFlags;

typedef struct
{ tSockaddrEntry sockaddrs[MAXNUM_SOCKADDRS]; /* the results of the lookup */
  struct tCachedHostInformation* hostinfo;
    /* the hostinfo for which the lookup is done; not for use by DNS handler */
  const char* hostname; /* the name which shall be looked up */
  tSockaddrIndex num; /* number of results */
  tDnsLookupFlags flags;
} tDnsLookup;

static void store_sockaddr(tDnsLookup* dns_lookup, const tSockaddr* addr,
  int address_family, size_t addrlen)
{ tSockaddrEntry* entry;
  const size_t num = dns_lookup->num;
  if (num >= MAXNUM_SOCKADDRS) return; /* got "enough" */
  entry = &(dns_lookup->sockaddrs[num]);
  entry->address_family = address_family; entry->addrlen = addrlen;
  my_memcpy(&(entry->addr), addr, addrlen); dns_lookup->num++;
#if CONFIG_DEBUG
  sockaddr2uistr(entry, strbuf, STRBUF_SIZE / 2);
  sprint_safe(debugstrbuf, "store_sockaddr(%p): af=%d, *%s*\n", dns_lookup,
    address_family, strbuf);
  debugmsg(debugstrbuf);
#endif
}

#if !HAVE_GETADDRINFO

static void store_rawaddr(tDnsLookup* dns_lookup, int address_family,
  const void* data)
{ tSockaddr addr;
  size_t addrlen;
  my_memclr_var(addr); ((struct sockaddr*)(&addr))->sa_family = address_family;
  if (address_family == AF_INET)
  { my_memcpy(&(((struct sockaddr_in*)(&addr))->sin_addr.s_addr), data, 4);
    addrlen = sizeof(struct sockaddr_in);
  }
#if USE_IPv6
  else if (address_family == AF_INET6)
  { my_memcpy(&(((struct sockaddr_in6*)(&addr))->sin6_addr.s6_addr), data, 16);
    addrlen = sizeof(struct sockaddr_in6);
  }
#endif
  else return; /* "should not happen" */
  store_sockaddr(dns_lookup, &addr, address_family, addrlen);
}

#define CONFIG_ADDR_LIST 1 /* CHECKME: use a configure script test? */

static void store_hostent(tDnsLookup* dns_lookup, const struct hostent* result)
{ const int address_family = result->h_addrtype;
  const size_t srcsize = result->h_length;
  size_t addrlen;
  const void* src;
  void* dest;
  tSockaddr addr;

  if (address_family == AF_INET)
  { if (srcsize != 4) return; /* "should not happen" */
    dest = &(((struct sockaddr_in*)(&addr))->sin_addr.s_addr);
    addrlen = sizeof(struct sockaddr_in);
  }
#if USE_IPv6
  else if (address_family == AF_INET6)
  { if (srcsize != 16) return; /* "should not happen" */
    dest = &(((struct sockaddr_in6*)(&addr))->sin6_addr.s6_addr);
    addrlen = sizeof(struct sockaddr_in6);
  }
#endif
  else return; /* "should not happen" */

  my_memclr_var(addr); ((struct sockaddr*)(&addr))->sa_family = address_family;
  {
#if CONFIG_ADDR_LIST
    size_t count = 0;
    if (result->h_addr_list == NULL) return; /* "should not happen" */
    while ( ( (src = result->h_addr_list[count++]) != NULL ) &&
            (dns_lookup->num < MAXNUM_SOCKADDRS) )
#else
    src = result->h_addr;
    if (src != NULL) /* "should" be true */
#endif
    { my_memcpy(dest, src, srcsize);
      store_sockaddr(dns_lookup, &addr, address_family, addrlen);
    }
  }
}

#endif /* #if !HAVE_GETADDRINFO */

static one_caller void my_getaddrinfo(tDnsLookup* dns_lookup)
/* performs a DNS hostname lookup; the monstrous size of the source code of
   this function plus all those store_....() functions is only a result of the
   attempt to make the DNS lookup as portable as possible; you could stop
   reading after the getaddrinfo() stuff... */
{ const char* hostname = dns_lookup->hostname;
#if HAVE_GETADDRINFO || HAVE_GETIPNODEBYNAME
  int err;
#endif
#if HAVE_GETADDRINFO
  static tBoolean did_init_hints = falsE;
  static struct addrinfo hints;
  /*const*/ struct addrinfo *orig_result, *result;
  if (!did_init_hints)
  { did_init_hints = truE; my_memclr_var(hints); hints.ai_family = HINT_AF;
    hints.ai_socktype = SOCK_STREAM; hints.ai_protocol = ipprotocolnumber_tcp;
  }
#else
  const struct hostent* result;
#endif

#if HAVE_GETADDRINFO

  debugmsg("DNS: getaddrinfo()\n");
  orig_result = NULL; /* (extra care for buggy libraries) */
  err = getaddrinfo(hostname, NULL, &hints, &orig_result);
  if ( (err == 0) && ( (result = orig_result) != NULL) )
  { do
    { const struct sockaddr* addr = result->ai_addr;
      const size_t addrlen = result->ai_addrlen;
      if ( (addr != NULL) && (addrlen > 0) && (addrlen <= sizeof(tSockaddr)) )
      { const int address_family = result->ai_family;
        if ( (address_family == AF_INET)
#if USE_IPv6
            || (address_family == AF_INET6)
#endif
           )
        { /* (Especially if <hints> contains AF_UNSPEC, getaddrinfo() might
              e.g. return an AF_UNIX address for hostname "localhost".) */
          store_sockaddr(dns_lookup, (const tSockaddr*) addr, address_family,
            addrlen);
          if (dns_lookup->num >= MAXNUM_SOCKADDRS) break; /* got "enough" */
        }
      }
      result = result->ai_next;
    } while (result != NULL);
  }
#if CONFIG_DEBUG
  else if (err != 0) { debugmsg(gai_strerror(err)); debugmsg(strNewline); }
#endif
  if (orig_result != NULL) freeaddrinfo(orig_result);

#elif HAVE_GETIPNODEBYNAME /* RFC2553; obsoleted by RFC3493, but... */

  result = getipnodebyname(hostname, AF_INET, 0, &err);
  if (result != NULL)
  { debugmsg("DNS: getipnodebyname(AF_INET)\n");
    store_hostent(dns_lookup, result); freehostent(result);
  }
#if USE_IPv6
  if (dns_lookup->num <= 0) /* wasn't IPv4, try IPv6 */
  { result = getipnodebyname(hostname, AF_INET6, 0, &err);
    if (result != NULL)
    { debugmsg("DNS: getipnodebyname(AF_INET6)\n");
      store_hostent(dns_lookup, result); freehostent(result);
    }
  }
#endif /* #if USE_IPv6 */

#else /* #if HAVE_GETADDRINFO / #elif HAVE_GETIPNODEBYNAME */

  /* The nice getaddrinfo() and the not-so-nice getipnodebyname() aren't
     available, so we have to work harder. We can't simply rely on
     gethostbyname() because "the behavior of gethostbyname() when passed a
     numeric address string is unspecified" (SUSv3); so we must try several
     other functions in advance. (The word "unspecified" could at least in
     theory mean that gethostbyname() returns a non-NULL value pointing to
     rubbish. And there are so many buggy libraries around...) */

  /* Check whether <hostname> is a numerical host address: */
#if HAVE_INET_PTON
#if USE_IPv6
  if (dns_lookup->flags & dlfTryIpv6)
  { char resbuf[16];
    if (inet_pton(AF_INET6, hostname, resbuf) == 1)
    { store_rawaddr(dns_lookup, AF_INET6, resbuf);
      debugmsg("DNS: inet_pton(AF_INET6)\n");
      return;
    }
  }
#endif /* #if USE_IPv6 */
  { char resbuf[16]; /* (unnecessarily large, just for buggy libraries) */
    if (inet_pton(AF_INET, hostname, resbuf) == 1)
    { store_rawaddr(dns_lookup, AF_INET, resbuf);
      debugmsg("DNS: inet_pton(AF_INET)\n");
      return;
    }
  }
#elif HAVE_INET_ADDR /* #if HAVE_INET_PTON */
#ifndef INADDR_NONE
#define INADDR_NONE ((tUint32) (-1))
#endif
  { tUint32 res = (tUint32) inet_addr(hostname);
      /* (The correct formal type would be in_addr_t, but glibc before
          2000-04-01 (and probably other libraries) doesn't have it...) */
    if (res != INADDR_NONE)
    { store_rawaddr(dns_lookup, AF_INET, &res);
      debugmsg("DNS: inet_addr()\n");
      return;
    }
  }
#endif /* #if HAVE_INET_PTON / #elif HAVE_INET_ADDR */

  /* Seems that <hostname> isn't a numerical host address... */
  result = gethostbyname(hostname);
  if (result != NULL)
  { debugmsg("DNS: gethostbyname()\n"); store_hostent(dns_lookup, result);
    /* (no freehostent() here - gethostbyname() uses static memory) */
  }
#if CONFIG_DEBUG
  else
  { char buf[100];
    sprint_safe(buf, "h_errno: %d\n", h_errno); debugmsg(buf);
  }
#endif

#endif /* #if HAVE_GETADDRINFO / #elif HAVE_GETIPNODEBYNAME */

}


/** DNS handler thread */

#if OPTION_THREADING == 0

/* nothing */

#elif OPTION_THREADING == 1

/* use the pthreads library */
#include <pthread.h>
#define THREADRETTYPE void*
#define THREADRETVAL NULL

#elif OPTION_THREADING == 2

/* use the Linux syscall clone() */
#include <sched.h>
#define THREADRETTYPE int
#define THREADRETVAL 0

#else /* #if OPTION_THREADING... */

#error "Bad multi-threading configuration; check option OPTION_THREADING!"

#endif /* #if OPTION_THREADING... */


#if OPTION_THREADING

typedef struct
{ int reader, writer; /* (from the handler's point of view) */
} tDnsThreadData;

static THREADRETTYPE dns_handler_thread(void* _data)
/* This function is the DNS handler thread; it checks for DNS lookup demands
   from the resource handler forever. Apart from its local variables, it only
   accesses the tDnsLookup structure given by the resource handler (which won't
   touch it while the lookup is running), so there aren't any multi-threading
   race conditions possible. */
{ const tDnsThreadData* data = (const tDnsThreadData*) _data;

#if HAVE_SIGFILLSET && (HAVE_SIGPROCMASK || (OPTION_THREADING == 1))

  { /* CHECKME: blocking the signals isn't "really" the right thing. It would
       be better to set signal handlers for this thread to SIG_IGN, but
       handlers can't be set per-thread, only per-process. Also it's not
       possible (portably) to find out which thread is signalled, e.g. SUSv3
       doesn't require in the list in functions/xsh_chap02_04.html that
       pthread_self() can be called in handlers. Look at all this mess...
       "Unix multi-threading is rubbish, because it's me who designed it",
       whispers the idiot. (Seems I'm not the only one with such problems, e.g.
       see <http://www.ussg.iu.edu/hypermail/linux/kernel/0403.3/1098.html>.)
       Even worse, certain signals must not be ignored/blocked by a thread
       resp. process because it might be left in an undefined state otherwise,
       says SUSv3. Add to this the buggy signal implementations of many
       operating systems and wonder how this program can work at all... */

    sigset_t set;
    if (sigfillset(&set) == 0)
    {
#if HAVE_SIGDELSET
      static const int sigs[] =
      {
#ifdef SIGTERM
        SIGTERM, /* (some users might like this one) */
#endif
#ifdef SIGFPE
        SIGFPE,
#endif
#ifdef SIGILL
        SIGILL,
#endif
#ifdef SIGSEGV
        SIGSEGV,
#endif
#ifdef SIGBUS
        SIGBUS,
#endif
        0 /* (to avoid trailing comma resp. empty array) */
      };
      unsigned char i;
      for (i = 0; i < ARRAY_ELEMNUM(sigs) - 1; i++) /* ("-1" for the 0) */
        (void) sigdelset(&set, sigs[i]);
#endif /* #if HAVE_SIGDELSET */

#if OPTION_THREADING == 1
      (void) pthread_sigmask(SIG_BLOCK, &set, NULL);
#else
      (void) sigprocmask(SIG_BLOCK, &set, NULL);
      /* (unspecified for multi-threading processes in SUSv3, but no choice
          available, just hope the best; this function call is only used under
          Linux, so...) */
#endif
    }
  }

#else

  /* It seems we have to accept some bad effects (mostly related to performance
     and cosmetics, so possibly acceptable)... */

#endif

  while (1)
  { static tDnsLookup* dns_lookup;
    static unsigned char count = 0;
    unsigned short loopcount;
    int err;

    loopcount = 0;
    loop:
    err = read/*_pipe*/(data->reader, ((char*)(&dns_lookup)) + count,
      sizeof(dns_lookup) - count); /* (can't use my_read() with FD register) */
    if ( (err == -1) && (errno == EINTR) && (++loopcount < 10000) ) goto loop;
    else if (err <= 0)
      fatal_error((err == -1) ? errno : 0, _("read(DNS) failed"));
    count += err;
    if (count >= sizeof(dns_lookup)) /* got a complete pointer */
    { my_getaddrinfo(dns_lookup); count = 0; writeloop0: loopcount = 0;
      writeloop:
      err = write/*_pipe*/(data->writer, ((char*)(&dns_lookup)) + count,
        sizeof(dns_lookup) - count);
      if ( (err == -1) && (errno == EINTR) && (++loopcount < 10000) )
        goto writeloop;
      else if (err <= 0)
        fatal_error((err == -1) ? errno : 0, _("write(DNS) failed"));
      count += err;
      if (count < sizeof(dns_lookup)) goto writeloop0;
      count = 0; /* wrote whole pointer, prepare for next round */
    }
  }
  /*@notreached@*/
  return(THREADRETVAL); /* avoid compiler warning for compilers which are too
    stupid to recognize simple infinite loops... */
}

#endif /* #if OPTION_THREADING */


/** Helper functions I */

static tBoolean got_activity;
void resource_preplex(void)
{ got_activity = falsE;
}

void resource_postplex(void)
{ i18n_cleanup
  /* IMPLEMENTME: if (got_activity) { .... } */
}

void uri_put(tUriData* u)
{ if (u->refcount > 1) u->refcount--;
  else
  { __dealloc(u->uri); __dealloc(u->hostname); __dealloc(u->path);
    __dealloc(u->query); __dealloc(u->post); __dealloc(u->username);
    __dealloc(u->password); memory_deallocate(u);
  }
}

void sinking_data_cleanup(tSinkingData* sd)
{ const tTransferData* td = sd->transfer_data;
  const tSinkingDataFlags flags = sd->flags;
  const tBoolean is_fd_valid = cond2boolean(flags & sdfIsDownloadFdValid);
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "sinking_data_cleanup(%p): flags=%d, fd=%d\n",
    sd, flags, (is_fd_valid ? (sd->download_fd) : (-42)));
  debugmsg(debugstrbuf);
#endif
  while (td != NULL)
  { const tTransferData* next = td->next;
    if (td->need_unmap) my_munmap(td->data, td->size);
    else memory_deallocate(td->data);
    __dealloc(td->name); memory_deallocate(td); td = next;
  }
  if (is_fd_valid) my_close(sd->download_fd);
}

void sinking_data_deallocate(tSinkingData** _sd)
{ tSinkingData* sd = *_sd;
  if (sd != NULL)
  { sinking_data_cleanup(sd); memory_deallocate(sd); *_sd = NULL; }
}

#if OPTION_LOCAL_CGI || OPTION_TLS || OPTION_EXECEXT
static tBoolean may_use_fd2(void)
{ static tBoolean did_calc = falsE, retval;
  if (!did_calc)
  { did_calc = truE;
    if ( (lfdmbs(2)) &&
         (
#if (TGC_IS_CURSES) && (CONFIG_TG != TG_XCURSES)
           (!is_environed) || /* don't ruin the curses screen handling... */
#endif
           (!my_isatty(fd_stderr)) ) )
      retval = truE;
    else retval = falsE;
  }
  return(retval);
}
#endif

#if OPTION_LOCAL_CGI || OPTION_EXECEXT
static tBoolean my_dup2(int fd1, int fd2)
/* returns whether it worked; IMPORTANT: this function is only intended for use
   in child processes after a fork(); otherwise we could get a race condition
   with retawq's other threads here because there's a window between close()
   and fcntl() - even within the C library's dup2(), unless e.g. an atomic
   dup2() in the OS is used */
{ unsigned char loopcount = 0;
  int err;
#if HAVE_DUP2
  do
  { err = dup2(fd1, fd2);
  } while ( (err == -1) && ( (errno == EINTR) || (errno == EBUSY) ) &&
            (++loopcount < 100) ); /* (EBUSY is for Linux) */
#else
  my_close_pipe(fd2);
  do
  { err = fcntl(fd1, F_DUPFD, fd2);
  } while ( (err == -1) && (errno == EINTR) && (++loopcount < 100) );
#endif
  return(cond2boolean(err >= 0));
}
#endif

static my_inline void resource_set_error(tResource* resource,
  tResourceError error)
{ if (resource->state != rsError)
  { resource->state = rsError; resource->error = error; }
}

static void resource_request_set_error(tResourceRequest* request,
  tResourceError error)
{ tResource* resource = request->resource;
  if (request->state != rrsError)
  { request->state = rrsError; request->error = error; }
  if (resource != NULL) resource_set_error(resource, error);
}

void resource_start_saving(tResource* resource, const tCantent* cantent,int fd)
{ const tContentblock* content = cantent->content;
  /* save the resource as far as we already got it */
  while (content != NULL)
  { size_t usedsize = content->used;
    if (usedsize > 0)
    { if (my_write(fd, content->data, usedsize) < (ssize_t) usedsize)
        goto do_close; /* IMPLEMENTME: alert user? */
    }
    content = content->next;
  }
  /* if there's more to come, remember the fd, otherwise close it */
  if ( (resource == NULL) || (resource->flags & rfFinal) )
  { do_close: my_close(fd); }
  else
  { tSaveAs* save_as = (tSaveAs*) __memory_allocate(sizeof(tSaveAs), mapOther);
    make_fd_cloexec(fd); save_as->fd = fd;
    save_as->next = resource->save_as; resource->save_as = save_as;
  }
}

static one_caller void do_save_as(tResource* resource, const char* src,
  size_t size)
{ tSaveAs* save_as = resource->save_as;
  while (save_as != NULL)
  { (void) my_write(save_as->fd, src, size);
      /* IMPLEMENTME: check for errors, remove record, alert user? */
      /* IMPROVEME: use a non-blocking algorithm? */
    save_as = save_as->next;
  }
}

static void stop_save_as(/*@notnull@*/ tResource* resource)
{ tSaveAs* save_as = resource->save_as;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "stop_save_as(%p,%p)\n", resource, save_as);
  debugmsg(debugstrbuf);
#endif
  while (save_as != NULL)
  { tSaveAs* next = save_as->next;
    my_close(save_as->fd);
    memory_deallocate(save_as);
    save_as = next;
  }
  resource->save_as = NULL;
}

#if CONFIG_HTTP & (HTTP_AUTH_BASIC | HTTP_PROXYAUTH)
static __sallocator char* __callocator base64_encode(const unsigned char* src)
/* converts an arbitrary string to base64 encoding (RFC3548, 3.) */
{ static /*@observer@*/ const unsigned char conv[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t len = strlen((const char*) src), destlen = ((len + 2) / 3) * 4 + 1;
  unsigned char ch, *dest = __memory_allocate(destlen, mapString), *ptr = dest,
    count = 0;
  unsigned int convval = 0;
#define PUT(offset) *ptr++ = conv[(convval >> offset) & 63]
  while ((ch = *src++) != '\0')
  { convval |= ((unsigned int) ch);
    if (++count < 3) convval <<= 8;
    else { PUT(18); PUT(12); PUT(6); PUT(0); convval = 0; count = 0; }
  }
  if (count > 0) /* something left to do */
  { if (count == 1) convval <<= 8;
    PUT(18); PUT(12);
    if (count == 2) PUT(6);
    else *ptr++ = '=';
    *ptr++ = '=';
  }
#undef PUT
  *ptr = '\0';
  return((char*) dest);
}
#endif

/* #if (CONFIG_HTTP & HTTP_AUTH_DIGEST) || OPTION_POP || OPTION_TRAP || CONFIG_DEBUG */
#if CONFIG_DEBUG

static one_caller void md5_decode(/*@out@*/ tUint32* dest, const tUint8* src,
  size_t count)
{ while (count-- > 0)
  { *dest++ = ((tUint32) src[0]) | (((tUint32) src[1]) << 8) |
     (((tUint32) src[2]) << 16) | (((tUint32) src[3]) << 24);
    src += 4;
  }
}

static void md5_encode(/*@out@*/ tUint8* dest, const tUint32* src,
  size_t count)
{ while (count-- > 0)
  { unsigned char cnt;
    tUint32 value = *src++;
    for (cnt = 0; cnt <= 3; cnt++) { *dest++ = value & 255; value >>= 8; }
  }
}

#define md5_prepare(roundnum) do { currfunc = func[roundnum]; currshift = shift[roundnum]; xidxval = xidxvals[roundnum]; xidxoff = xidxoffs[roundnum]; } while (0)
#define md5_rotate(val, cnt) (((val) << (cnt)) | ((val) >> (32-(cnt))))
#define md5_operate(a, b, c, d) do { (a) += (currfunc(b, c, d)) + (x[xidxval & 15]) + sineval[stepcount]; (a) = md5_rotate((a), currshift[stepcount & 3]); (a) += (b); stepcount++; si--; xidxval += xidxoff; } while (0)

typedef tUint32 (*tMd5Function)(tUint32, tUint32, tUint32);
static tUint32 md5_F(tUint32 a, tUint32 b, tUint32 c)
{ return(((a) & (b)) | ((~a) & (c))); }
static tUint32 md5_G(tUint32 a, tUint32 b, tUint32 c)
{ return(((a) & (c)) | ((b) & (~c))); }
static tUint32 md5_H(tUint32 a, tUint32 b, tUint32 c)
{ return((a) ^ (b) ^ (c)); }
static tUint32 md5_I(tUint32 a, tUint32 b, tUint32 c)
{ return((b) ^ ((a) | (~c))); }

static void md5_digest_block(tUint32* state, const tUint8* data)
/* Maybe slow, but small; speed doesn't matter much here because this is rarely
   used, if ever, and only with rather small amounts of data. */
{ static const tUint32 sineval[64] =
  { 0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085,  0x4881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
  };
  static const unsigned char shift1[4] = {7,12,17,22}, shift2[4] = {5,9,14,20},
    shift3[4] = {4,11,16,23}, shift4[4] = {6,10,15,21}, xidxvals[4] ={0,1,5,0},
    xidxoffs[4] = {1,5,3,7}, *shift[4] = { shift1, shift2, shift3, shift4 };
  static const tMd5Function func[4] = { md5_F, md5_G, md5_H, md5_I };
  unsigned char stepcount = 0, si = 128, roundnum, count, xidxval, xidxoff;
  const unsigned char* currshift;
  tMd5Function currfunc;
  tUint32 x[16], cs[4];

  md5_decode(x, data, 16); my_memcpy(cs, state, sizeof(cs));
  for (roundnum = 0; roundnum <= 3; roundnum++)
  { md5_prepare(roundnum);
    for (count = 0; count <= 15; count++)
      md5_operate(cs[si & 3], cs[(si+1) & 3], cs[(si+2) & 3], cs[(si+3) & 3]);
  }
  for (count = 0; count <= 3; count++) state[count] += cs[count];
}

static void md5_digest(const tUint8* data, size_t size,
  /*@out@*/ tUint8* result)
{ tUint32 state[4] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 };
  size_t numobits = size * 8, tsize;
  tUint8 tailbuf[128], *tail = tailbuf;

  while (size >= 64) { md5_digest_block(state, data); data += 64; size -= 64; }

  if (size > 0) my_memcpy(tailbuf, data, size);
  tsize = ( (size >= 56) ? 128 : 64 );
  tailbuf[size++] = 0x80; my_memclr(tailbuf + size, tsize - size);
  md5_encode(tailbuf + tsize - 8, &numobits, 1);
  while (tsize >= 64) { md5_digest_block(state, tail); tail += 64; tsize-=64; }

  md5_encode(result, state, 4);
}

#endif /* need-md5 */


/** Custom connections */

#if CONFIG_CUSTOM_CONN

#define doing_custom_conn(resource) ((resource)->flags & rfCustomConn)

static void __custom_conn_print(tResource* resource, const char* str,
  size_t len, tCustomConnPrintingKind ccpk)
{ tCustomConnPrintingData data;
  data.text = str; data.len = len; data.ccpk = ccpk;
  main_handle_custom_conn(resource, 0, &data);
}

static my_inline void custom_conn_print(tResource* resource, const char* str,
  tCustomConnPrintingKind ccpk)
{ tCustomConnPrintingData data;
  data.text = str; data.len = ( (ccpk == ccpkNetresp) ? strlen(str) : 0 );
  data.ccpk = ccpk; main_handle_custom_conn(resource, 0, &data);
}

static __my_inline void custom_conn_unbusify(tResource* resource)
/* what a name - "un-busy-i-fy"... :-) */
{ main_handle_custom_conn(resource, 1, NULL);
}

static __my_inline void custom_conn_tell_msg(tResource* resource)
{ main_handle_custom_conn(resource, 2, NULL);
}

static __my_inline void custom_conn_tell_error(tResource* resource,
  tResourceError re)
{ main_handle_custom_conn(resource, 3, &re);
}

static __my_inline void custom_conn_tell_hash(tResource* resource)
{ main_handle_custom_conn(resource, 4, NULL);
}

#else

#define doing_custom_conn(resource) (falsE)

#endif


/** Content blocks */

static __sallocator tContentblock* __callocator
  contentblock_create(size_t size, unsigned char flags)
/* creates a block for content data */
{ tContentblock* retval = memory_allocate(sizeof(tContentblock),
    mapContentblock);
  if (flags & 1) retval->data = __memory_allocate(size, mapOther);
  retval->usable = size;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "created contentblock: %p, %d\n", retval, size);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

#define HIGHEST_OCBS (131072)

static size_t optimal_contentblocksize(size_t count)
/* calculates the optimal :-) size for a new content block of a resource,
   depending on how many bytes we already got for it; the assumption behind
   this is that e.g. if we already received 100,000 bytes there might be
   100,000 more to come, so we allocate bigger blocks, read in larger chunks
   (as far as the socket buffer of the operating system allows), reserve memory
   less frequently and thus hopefully improve performance... */
{ size_t retval;
  if (count <= 10000) retval = 4096;
  else if (count <= 40000) retval = 8192;
  else if (count <= 200000) retval = 32768;
  else retval = HIGHEST_OCBS; /* Downloading Linux kernel sources, eh? :-) */
  return(retval);
}

static void cantent_set_firstcontent(tCantent* cantent,
  tContentblock* content)
{ cantent->content = cantent->lastcontent = cantent->lhpp_content = content;
  cantent->lhpp_byte = 0; /* cantent->flags &= ~cafFullyParsed; */
}

static tContentblock* cantent_append_new_contentblock(tCantent* cantent,
  size_t usablesize)
{ tContentblock* retval = contentblock_create(usablesize, 1);
  if (cantent->content == NULL) cantent_set_firstcontent(cantent, retval);
  else
  { if (cantent->lastcontent != NULL) cantent->lastcontent->next = retval;
    cantent->lastcontent = retval;
  }
  return(retval);
}

tCantent* cantent_create(void)
{ tCantent* retval = (tCantent*) memory_allocate(sizeof(tCantent), mapCantent);
  return(retval);
}

static void cantent_deallocate_tree(tCantent* cantent)
{ const tHtmlNode* node = cantent->tree;
  while (node != NULL)
  { const tHtmlNode* next = node->next;
    deallocate_html_node(node); node = next;
  }
  cantent->tree = NULL;
}

static void contentblocklist_deallocate(const tContentblock* block)
{ while (block != NULL)
  { const tContentblock* next = block->next;
#if CONFIG_DEBUG
    sprint_safe(debugstrbuf, "contentblocklist_deallocate(): %p\n", block);
    debugmsg(debugstrbuf);
#endif
    __dealloc(block->data); memory_deallocate(block); block = next;
  }
}

void cantent_put(tCantent* cantent)
{ tContentblock* content;
  if (cantent->refcount > 1) { cantent->refcount--; return; }
  content = cantent->content;
  if (cantent->caf & cafNeedUnmap) my_munmap(content->data, content->used);
  else contentblocklist_deallocate(content);
  if (cantent->aenum > 0) /* deallocate active elements */
  { const tActiveElementBase* aebase = cantent->aebase;
    tActiveElementNumber _ae;
    for (_ae = 0; _ae < cantent->aenum; _ae++)
      deallocate_one_aebase(&(aebase[_ae]));
    __dealloc(cantent->aebase);
  }
  if (cantent->hfnum > 0) /* deallocate forms */
  { const tHtmlForm* form = cantent->form;
    tHtmlFormNumber count;
    for (count = 0; count < cantent->hfnum; count++)
      __dealloc(form[count].action_uri);
    __dealloc(cantent->form);
  }
  __dealloc(cantent->redirection); __dealloc(cantent->major_html_title);
  cantent_deallocate_tree(cantent); memory_deallocate(cantent);
}

static void cantent_provide_room(tCantent* cantent, /*@out@*/ char** dest,
  /*@out@*/ size_t* size, size_t desired_size)
{ tContentblock* content = cantent->lastcontent;
  size_t usable, used;
  if (content != NULL) { usable = content->usable; used = content->used; }
  else { usable = used = 0; }
  if (usable <= used)
  { content = cantent_append_new_contentblock(cantent, MAX(desired_size,4096));
    usable = content->usable; used = content->used;
  }
  *dest = content->data + used; *size = usable - used;
}

void cantent_collect_str(tCantent* cantent, const char* str)
{ size_t todo = strlen(str), available, copysize;
  char* dest;
  loop:
  cantent_provide_room(cantent, &dest, &available, todo);
  copysize = MIN(todo, available); my_memcpy(dest, str, copysize);
  cantent->lastcontent->used += copysize;
  if (todo > copysize) { todo -= copysize; str += copysize; goto loop; }
}

void cantent_collect_title(tCantent* cantent, const char* title)
{ char* spfbuf;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, strHtmlPageTitle, title, title);
  cantent_collect_str(cantent, spfbuf); my_spf_cleanup(strbuf, spfbuf);
}


/** Host information cache; address lookups */

my_enum1 enum
{ hppifNone = 0, hppifUserQueryRunning = 0x01
} my_enum2(unsigned char) tHostPortProtInfoFlags;

typedef struct tLoginData
{ struct tLoginData* next;
  const char *username, *password /* , *account */;
} tLoginData;

typedef struct tHostPortProtInfo
{ struct tHostPortProtInfo* next;
  struct tCachedHostInformation* host;
  tLoginData* login_data;
  tPortnumber portnumber; /* (in network byte order) */
  tResourceProtocol protocol;
  tHostPortProtInfoFlags flags;
} tHostPortProtInfo;

my_enum1 enum
{ chifNone = 0, __chifAddressLookupRunning = 0x01
} my_enum2(unsigned char) tCachedHostInformationFlags;

#if CONFIG_ASYNC_DNS
#define chifAddressLookupRunning (__chifAddressLookupRunning)
#endif

#if OPTION_COOKIES
struct tCookie;
#endif

#if OPTION_NEWS
struct tNewsGroupInformation;
#endif

typedef struct tCachedHostInformation
{ struct tCachedHostInformation* next;
  tSockaddrEntry** sockaddrs;
  tHostPortProtInfo* hppi;
  const char* hostname;
#if CONFIG_ASYNC_DNS
  tDhmGenericData* dhm_data; /* (just for dhmnfOnce on DNS lookup) */
#endif
#if OPTION_COOKIES
  struct tCookie* cookies; /* cookies for this host */
#endif
#if OPTION_NEWS
  struct tNewsGroupInformation* ngi;
#endif
  /* time_t lookupfailuretime; */
  tCachedHostInformationFlags flags;
  tSockaddrIndex num_sockaddrs;
#if OPTION_COOKIES
  unsigned char cookiecount; /* how many cookies we've stored for this host */
#endif
} tCachedHostInformation;

#define resource2actual_host(resource)  ((resource)->actual_hppi->host)
#define resource2textual_host(resource) ((resource)->textual_hppi->host)

static tHostPortProtInfo* hppi_lookup(tCachedHostInformation* hostinfo,
  tPortnumber portnumber, tResourceProtocol protocol, tBoolean create_if_null)
{ tHostPortProtInfo* hppi = hostinfo->hppi;
  while (hppi != NULL)
  { if ( (hppi->portnumber == portnumber) && (hppi->protocol == protocol) )
      goto out; /* found */
    hppi = hppi->next;
  }
  if ( (hppi == NULL) && (create_if_null) )
  { hppi = (tHostPortProtInfo*) memory_allocate(sizeof(tHostPortProtInfo),
      mapPermanent);
    hppi->next = hostinfo->hppi; hostinfo->hppi = hppi; hppi->host = hostinfo;
    hppi->portnumber = portnumber; hppi->protocol = protocol;
  }
  out:
  return(hppi);
}

static void hppi_login_set(tHostPortProtInfo* hppi, const char* username,
  const char* password)
{ tLoginData* ld;
  if ( (username == NULL) || (password == NULL) ) /* "should not happen" */
    return;
  ld = hppi->login_data;
  while (ld != NULL)
  { const char* u = ld->username;
    if ( (u != NULL) && (!strcmp(u, username)) )
    { const char* p = ld->password;
      if ( (p == NULL) || (strcmp(p, password)) )
        my_strdedup(ld->password, password);
      return;
    }
    ld = ld->next;
  }
  ld = memory_allocate(sizeof(tLoginData), mapOther);
  ld->username = my_strdup(username); ld->password = my_strdup(password);
  ld->next = hppi->login_data; hppi->login_data = ld;
}

#if 0
static void hppi_login_get(const tHostPortProtInfo* hppi,
  /*@out@*/ const char** username, /*@out@*/ const char** password)
{
}
#endif

#define HASHTABLESIZE_CHI (HASHTABLESIZE)
static tCachedHostInformation* chi_head[HASHTABLESIZE_CHI];

static tHashIndex hostinfo_hostname2hashindex(const char* hostname)
/* calculates a hash table index from the given hostname; we take at most the
   first ten characters of the name into account - this "should be enough" for
   good hashing and we need not do "%" after each partial operation (because
   the maximum possible value, 10 * 255, fits into a tHashIndex variable). */
{ tHashIndex retval = 0;
  unsigned char count = 10;
  while (count-- > 0)
  { unsigned char ch = *((const unsigned char*) (hostname));
    if (ch == '\0') break; /* reached end of string */
    retval += ((tHashIndex) ch);
    hostname++;
  }
  return(retval % HASHTABLESIZE_CHI);
}

static /* __sallocator -- not an "only" reference... */
  tCachedHostInformation* __callocator hostinfo_create(const char* hostname)
/* creates a new record for the host information cache */
{ tHashIndex i = hostinfo_hostname2hashindex(hostname);
  tCachedHostInformation* retval = (tCachedHostInformation*)
    memory_allocate(sizeof(tCachedHostInformation), mapPermanent);
  retval->hostname = my_strdup(hostname);
  retval->next = chi_head[i];
  chi_head[i] = retval;
  return(retval);
}

static /*@observer@*/ tCachedHostInformation* hostinfo_lookup(const char*
  hostname)
/* tries to find known information about the host with the given <hostname> in
   the host information cache */
{ tHashIndex i = hostinfo_hostname2hashindex(hostname);
  tCachedHostInformation* retval = chi_head[i];
  while (retval != NULL)
  { if (streqcase(hostname, retval->hostname)) break; /* found */
        /* IMPROVEME: "!strcmp()" should be enough here. */
    retval = retval->next;
  }
  return(retval);
}

enum
{ halrFine = 0, halrLookupFailed = 1, halrNotInCache = 2
#if CONFIG_ASYNC_DNS
  , halrLookupRunning = 3
#endif
};
typedef unsigned char tHostAddressLookupResult;

enum { halfNone = 0, halfInCacheOnly = 0x01, halfEnforcedReload = 0x02 };
typedef unsigned char tHostAddressLookupFlags;

static one_caller void postprocess_dns_lookup(tDnsLookup* dns_lookup)
/* always executed in the main thread - that's the crucial point of this
   separate function */
{ tCachedHostInformation* hostinfo = dns_lookup->hostinfo;
  tSockaddrIndex num = dns_lookup->num, idx;
  if (num <= 0) { /* hostinfo->lookupfailuretime = my_time(); */ }
  else
  { const size_t size = num * sizeof(tSockaddrEntry*);
    tSockaddrEntry** s = hostinfo->sockaddrs =__memory_allocate(size,mapOther);
    hostinfo->num_sockaddrs = num;
    for (idx = 0; idx < num; idx++)
    { const tSockaddrEntry* entry = &(dns_lookup->sockaddrs[idx]);
      const size_t addrlen = entry->addrlen;
      const int address_family = entry->address_family;
      const tSockaddr* addr = &(entry->addr);
      tSockaddrEntry **head = sockaddr2listhead(addr, addrlen), *e = *head;
      while (e != NULL)
      { if ( (e->addrlen == addrlen) && (e->address_family == address_family)
             && (!my_memdiff(&(e->addr), addr, sizeof(tSockaddr))) )
          break; /* found */
        e = e->next;
      }
      if (e == NULL) /* create a new entry */
      { e = (tSockaddrEntry*) __memory_allocate(sizeof(tSockaddrEntry),
         mapPermanent);
        my_memcpy(e, entry, sizeof(tSockaddrEntry));
        e->next = *head; *head = e;
      }
      s[idx] = e;
    }
  }
  memory_deallocate(dns_lookup->hostname); memory_deallocate(dns_lookup);
}

static void check_proxies(tResourceRequest* request, const tConfigProxy* proxy,
  const char** _hostname)
{ tPortnumber reqport = request->uri_data->portnumber;
  const char* hostname = *_hostname;
  while (proxy != NULL)
  { tPortnumber hp = proxy->hosts_portnumber;
    if ( ( (hp == 0) || (hp == reqport) ) &&
         (my_pattern_matcher(proxy->hosts_pattern, hostname)) )
    { const char* h = proxy->proxy_hostname;
      if (h != NULL) { *_hostname = h; request->proxy = proxy; }
      /* "else": configuration explicitly said: "Don't use a proxy here!" */
      break;
    }
    proxy = proxy->next;
  }
}

static tHostAddressLookupResult lookup_hostaddress(tResourceRequest* request,
  /*@out@*/ tCachedHostInformation** hostinforet, tHostAddressLookupFlags half)
/* tries to lookup the address information for the given hostname in the host
   information cache etc.; if that fails, it tries to start a new DNS lookup */
{ const char* hostname = request->uri_data->hostname;
  tResourceProtocol protocol = request->uri_data->rp;
  const tBoolean in_cache_only = cond2boolean(half & halfInCacheOnly),
    enforced_reload = cond2boolean(half & halfEnforcedReload);
  tCachedHostInformation* hostinfo;
  tDnsLookup* dns_lookup;

  /* Check whether we shall use a proxy */

  if (protocol == rpHttp)
    check_proxies(request, config.http_proxies, &hostname);
#if OPTION_TLS
  else if (protocol == rpHttps)
    check_proxies(request, config.https_proxies, &hostname);
#endif

#if CONFIG_DEBUG
  { char* spfbuf;
    my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf,
      "lookup_hostaddress(): %p,%d,%s,%s\n", request, half,
      request->uri_data->hostname, hostname);
    debugmsg(spfbuf);
    my_spf_cleanup(debugstrbuf, spfbuf);
  }
#endif

  /* Try to find the address for the <hostname> in the cache */

  hostinfo = hostinfo_lookup(hostname);
  *hostinforet = hostinfo; /* (may be NULL) */
  if (hostinfo != NULL)
  {
#if CONFIG_ASYNC_DNS
    if (hostinfo->flags & chifAddressLookupRunning) /* not yet finished */
      return(halrLookupRunning);
    else
#endif
    if (hostinfo->sockaddrs != NULL) return(halrFine); /* already calculated */
    else if (enforced_reload) goto start_dns_lookup; /* enforce (re-)lookup */
    /* IMPLEMENTME: if ( (!in_cache_only) && (->lookupfailuretime is
       more than e.g. one minute in the past) ) goto start_dns_lookup; */
    return(halrLookupFailed); /* lookup failed or bug occurred */
  }
  if (in_cache_only) return(halrNotInCache);

  /* Seems we have to start a DNS lookup request; to do this, we set up a
     tDnsLookup structure and e.g. write its address to the DNS pipe. */

  start_dns_lookup:
  if (hostinfo == NULL) *hostinforet = hostinfo = hostinfo_create(hostname);
  dns_lookup = (tDnsLookup*) memory_allocate(sizeof(tDnsLookup), mapOther);
  dns_lookup->hostinfo = hostinfo;
  dns_lookup->hostname = my_strdup(hostname);
  if (request->uri_data->udf & udfTryIpv6) dns_lookup->flags |= dlfTryIpv6;
#if CONFIG_DEBUG
  { char* spfbuf;
    my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf,
      "starting DNS lookup at %p for hostinfo %p, hostname *%s*\n",
      dns_lookup, hostinfo, hostname);
    debugmsg(spfbuf);
    my_spf_cleanup(debugstrbuf, spfbuf);
  }
#endif
#if OPTION_THREADING
  hostinfo->flags |= chifAddressLookupRunning;
  dhm_init(hostinfo, NULL, "hostinfo");
  my_write_crucial/*_pipe*/(fd_resource2dns_write, &dns_lookup,
    sizeof(dns_lookup));
  return(halrLookupRunning);
#else
  my_getaddrinfo(dns_lookup); /* this call might block the program */
  is_time_valid = falsE; /* might have taken a long time */
  postprocess_dns_lookup(dns_lookup);
  return( (hostinfo->num_sockaddrs > 0) ? halrFine : halrLookupFailed );
#endif
}


/** Sockets and connections I */

#define NEED_QUITCMD_DISSOLVING (CONFIG_FTP || OPTION_NEWS || OPTION_POP)
#define NEED_DISSOLVING (OPTION_TLS || NEED_QUITCMD_DISSOLVING)

my_enum1 enum
{ cnfNone = 0, cnfConnected = 0x01, cnfReading = 0x02, cnfWriting = 0x04,
  cnfSuspended = 0x08, cnfDataIsResource = 0x10, cnfWantToWrite = 0x20,
  cnfDontReuse = 0x40
#if OPTION_TLS
  , cnfTlsDedication = 0x80, cnfTlsHandshaking = 0x100
#endif
#if NEED_DISSOLVING
  , cnfDissolving = 0x200
#endif
#if CONFIG_DEBUG
  , cnfDebugNodata = 0x400
#endif
} my_enum2(unsigned short) tConnectionFlags;

my_enum1 enum
{ ccekConnectSetup = 0, ccekConnectWorked = 1, ccekConnectFailed = 2,
  ccekRead = 3, ccekWrite = 4
} my_enum2(unsigned char) tConnCbEventKind;

typedef void (*tConnCbFunc)(struct tConnection*, tConnCbEventKind);

#if NEED_DISSOLVING
typedef tBoolean (*tConnDissolver)(struct tConnection*);
#endif

typedef struct
{ tSockaddrEntry** sockaddrs;
  tPortnumber portnumber; /* (in network byte order) */
  tSockaddrsBitfield tried; /* which IP addresses were already tried */
  tSockaddrIndex num, current; /* which IP address is currently tried/used */
} tConnectAttemptData;

#if OPTION_TLS == TLS_MATRIX
/* to handle insufficiencies of the "slightly" too simplistic API... */
my_enum1 enum
{ tmcfNone = 0, tmcfDidSetup = 0x01, tmcfWriting = 0x02
} my_enum2(unsigned char) tTmcFlags;
typedef struct
{ sslBuf_t incoming, outgoing, temp_in;
  tTmcFlags flags;
} tTlsMatrixCrutch;
#define TLS_MCBUFSIZE (((SSL_MAX_RECORD_LEN) & ~31) + 32) /* (nicer alloc?) */
/* The library used SSL_MAX_RECORD_SIZE, but that was strangely changed to
   SSL_MAX_RECORD_LEN in MatrixSSL 1.2.1. Brain-damaged... */
#endif

#define NEED_CONNLIST (CONFIG_ABOUT & 1) /* "about:activity" needs it */

typedef struct tConnection
{ tConnectAttemptData cad;
#if NEED_CONNLIST
  struct tConnection* next;
#endif
  tConnCbFunc callback;
#if NEED_DISSOLVING
  tConnDissolver dissolver;
#endif
  void* data; /* (usually tResource*) */
#if OPTION_TLS
  tTlsSession tls_session;
#if OPTION_TLS == TLS_MATRIX
  tTlsMatrixCrutch* tmc;
#endif
#endif
#if OPTION_POP || OPTION_TRAP
  const char* authstamp;
#endif
  const tHostPortProtInfo* hppi;
  const void* writedata;
  size_t writedata_todo, writedata_done;
  int fd;
  tConnectionFlags flags;
  tResourceProtocol protocol;
  tResourceError prelire; /* "preliminary" re */
  unsigned char x; /* (meaning depends on protocol) */
} tConnection;

#if (!defined(O_NONBLOCK)) && defined(O_NDELAY)
#define O_NONBLOCK (O_NDELAY)
#endif

static tBoolean make_fd_nonblocking(int fd)
/* tries to make <fd> non-blocking; returns whether it worked */
{ tBoolean retval;
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket)
  { static const tUint32 constant_one = 1;
      /* (lwIP 0.7.2 says "u32_t" - never heard of "int"...) */
    errno = 0; /* silly old lwIP versions didn't set errno on error */
    retval = cond2boolean(lwip_ioctl(fd, FIONBIO, &constant_one) == 0);
  }
  else
#endif
  { const int flags = fcntl(fd, F_GETFL, 0);
    if ( (flags == -1) || (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) )
      retval = falsE;
    else retval = truE;
  }
  return(retval);
}

static int create_socket(int address_family)
/* creates a new TCP socket and sets it to non-blocking mode; returns -2 if the
   fd wouldn't be observable, -1 for OS errors or a valid fd if it worked */
{ int retval;
#if USE_LWIP
  errno = 0; /* silly old lwIP versions didn't set errno on error */
  retval = lwip_socket(address_family, SOCK_STREAM, ipprotocolnumber_tcp);
#else
  retval = socket(address_family, SOCK_STREAM, ipprotocolnumber_tcp);
#endif
  if (retval >= 0)
  { fd_register(&retval, fdkSocket);
    if (!fd_is_observable(retval)) { my_close_sock(retval); retval = -2; }
    else if (!make_fd_nonblocking(retval))
    { my_close_sock(retval); retval = -1; }
    else make_fd_cloexec(retval);
  }
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "create_socket(%d): %d\n", address_family, retval);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

static void set_portnumber(tSockaddr* addr, int address_family,
  tPortnumber portnumber)
{ if (address_family == AF_INET)
    ((struct sockaddr_in*)(addr))->sin_port = portnumber;
#if USE_IPv6
  else if (address_family == AF_INET6)
    ((struct sockaddr_in6*)(addr))->sin6_port = portnumber;
#endif
  /* "else": we don't know the AF, so we don't override default ports. */
}

static int my_do_connect(int fd, const tSockaddr* addr, size_t addrlen)
{ int err;
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket)
  { errno = 0; /* silly old lwIP versions didn't set errno on error */
    err = lwip_connect(fd, (const struct sockaddr*) addr,
      (socklen_t) addrlen);
  }
  else
#endif
  { unsigned char loopcount = 0;
    loop:
    err = connect(fd, (const struct sockaddr*) addr, (socklen_t) addrlen);
    if (err != 0)
    { if ( (errno == EINTR) && (++loopcount < 100) ) goto loop;
      else if ( (errno == EALREADY) && (loopcount > 0) ) errno = EINPROGRESS;
      else if (errno != EINPROGRESS)
      { int e = errno; my_close_sock(fd); errno = e; }
    }
  }
  return(err);
}

static int my_connect(int fd, const tSockaddrEntry* entry,
  tPortnumber portnumber)
{ tSockaddr addr = entry->addr; /* (yes, we copy a structure here) */
  set_portnumber(&addr, entry->address_family, portnumber);
  return(my_do_connect(fd, &addr, entry->addrlen));
}

#define connect_err2error conn_err2error /* (currently the same) */
#define reConnectionFailureDefault (reConnect)
static int conn_err; /* temporary storage for connection-related errors */

static tResourceError conn_err2error(int err)
/* translates error codes; use this when a general connection-related (read,
   write, connect) error occurs */
{ tResourceError re;
  switch (err)
  { case ECONNREFUSED: re = reRefused; break;
    case EADDRNOTAVAIL: re = reNetwork; break; /* CHECKME! */
    case ENETUNREACH: re = reNetwork; break;
    case ETIMEDOUT: re = reTimeout; break;
    case EPIPE: case EAGAIN: re = reServerClosed; break;
#ifdef ECONNRESET
    case ECONNRESET: re = reConnReset; break;
#endif
    default: re = reConnectionFailureDefault; break;
  }
  return(re);
}

static tResourceError conn_get_failre(tConnection* conn)
/* for use in ccekConnectFailed handling */
{ tResourceError re = conn->prelire;
  if (re == reFine) re = connect_err2error(conn_err);
  return(re);
}

static __my_inline void conn_set_prelire(tConnection* conn, tResourceError re)
{ conn->prelire = re;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "prelire: %p, %d\n", conn, re);
  debugmsg(debugstrbuf);
#endif
}

static tBoolean conn_connect(tConnection* conn)
/* tries to connect to one of the IP addresses which are "allowed" for <conn>;
   returns whether it worked */
{ tConnectAttemptData* cad = &(conn->cad);
  tSockaddrEntry **list = cad->sockaddrs, *entry;
  tSockaddrIndex idx, best;
  tSockaddrRating rating, best_rating;
  int fd;

  conn->flags &= ~cnfConnected; conn->fd = -1;
  cad->current = SOCKADDR_INDEX_INVALID;
  if (list == NULL) /* no choice (e.g. FTP data connections) */
  { failed: return(falsE); }

  /* try to find the "best" sockaddr */
  bestloop:
  best = SOCKADDR_INDEX_INVALID; best_rating = SOCKADDR_RATING_NONE;
  for (idx = 0; idx < cad->num; idx++)
  { const tSockaddrPortProtInfo* sppi;
    if (my_bit_test(&(cad->tried), idx)) continue; /* already tried this one */
    entry = list[idx];
    sppi = sockaddr2sppi(entry, cad->portnumber, conn->protocol, falsE);
    rating = ( (sppi != NULL) ? sppi->rating : SOCKADDR_RATING_NEW );
    if (best_rating < rating) { best_rating = rating; best = idx; }
  }
  if (best == SOCKADDR_INDEX_INVALID) goto failed;

  /* try to connect to that sockaddr */
  idx = best;
  my_bit_set(&(cad->tried), idx); cad->current = idx; entry = list[idx];
#if CONFIG_DEBUG
  sockaddr2uistr(entry, strbuf, STRBUF_SIZE / 2);
  sprint_safe(debugstrbuf, "conn_connect(%p): af=%d, *%s*\n", conn,
    entry->address_family, strbuf);
  debugmsg(debugstrbuf);
#endif
  fd = create_socket(entry->address_family);
  if (fd == -2) /* no amount of further looping could help */
  { conn_set_prelire(conn, reTmofd); goto failed; }
  else if (fd == -1) conn_set_prelire(conn, reSocket);
  else if (fd >= 0)
  { int err = my_connect(fd, entry, cad->portnumber);
    if ( (err == 0) || ( (err == -1) && (errno == EINPROGRESS) ) )
    { if (err == 0) conn->flags |= cnfConnected; /*unlikely when non-blocking*/
      conn->fd = fd; return(truE);
    }
    else conn_set_prelire(conn, connect_err2error(errno));
  }
  goto bestloop;
}

#if CONFIG_DEBUG
static void debug_conn_callback(const tConnection* conn, tConnCbEventKind ccek)
{ sprint_safe(debugstrbuf, "conn_callback(): fd=%d, ccek=%d, data=%p, x=%d\n",
    conn->fd, ccek, conn->data, conn->x);
  debugmsg(debugstrbuf);
}
static void conn_bug(const tConnection* conn, tConnCbEventKind ccek)
{ sprint_safe(debugstrbuf, "conn_bug(): fd #%d, ccek=%d\n", conn->fd, ccek);
  debugmsg(debugstrbuf);
}
#else
#define debug_conn_callback(conn, ccek) do { } while (0)
#define conn_bug(conn, ccek) do { } while (0)
#endif

#define conn_callback(conn, ccek) \
  do \
  { debug_conn_callback((conn), (ccek)); \
    ((conn)->callback)((conn), (ccek)); \
  } while (0)

#define conn2resource(conn) \
  ( (conn->flags & cnfDataIsResource) ? ((tResource*) (conn->data)) : NULL )

static tSockaddrEntry* conn2sockaddr(const tConnection* conn)
{ tSockaddrEntry* retval;
  const tConnectAttemptData* cad = &(conn->cad);
  const tSockaddrIndex idx = cad->current;
  if (idx != SOCKADDR_INDEX_INVALID) retval = cad->sockaddrs[idx];
  else retval = NULL;
  return(retval);
}

static tSockaddrPortProtInfo* conn2sppi(const tConnection* conn,
  tBoolean create_if_null)
{ tSockaddrPortProtInfo* retval;
  tSockaddrEntry* entry = conn2sockaddr(conn);
  retval = ( (entry != NULL) ? sockaddr2sppi(entry, conn->cad.portnumber,
    conn->protocol, create_if_null) : NULL );
  return(retval);
}

#if NEED_CONNLIST
static tConnection* connlist_head = NULL;
#endif

static tConnection* conn_create(int fd, tConnCbFunc callback, void* data,
  tBoolean data_is_resource, const tHostPortProtInfo* hppi,
  tResourceProtocol protocol)
{ tConnection* retval = (tConnection*) memory_allocate(sizeof(tConnection),
    mapConnection);
  tConnectAttemptData* cad = &(retval->cad);
#if NEED_CONNLIST
  retval->next = connlist_head; connlist_head = retval;
#endif
  retval->fd = fd; retval->callback = callback; retval->data = data;
  if (data_is_resource) retval->flags |= cnfDataIsResource;
  retval->hppi = hppi; retval->protocol = protocol;
  cad->current = SOCKADDR_INDEX_INVALID;
  if (hppi != NULL)
  { const tCachedHostInformation* hostinfo = hppi->host;
    tSockaddrIndex num = hostinfo->num_sockaddrs;
    if (num > 0)
    { const size_t size = num * sizeof(tSockaddrEntry*);
      tSockaddrEntry** e = cad->sockaddrs = __memory_allocate(size, mapOther);
      my_memcpy(e, hostinfo->sockaddrs, size); cad->num = num;
    }
    cad->portnumber = hppi->portnumber;
  }
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "conn_create(): fd #%d\n", fd);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

static my_inline void conn_change_data(tConnection* conn, void* data,
  tBoolean data_is_resource)
{ conn->data = data;
  if (data_is_resource) conn->flags |= cnfDataIsResource;
  else conn->flags &= ~cnfDataIsResource;
}

static my_inline void conn_change_callback(tConnection* conn,
  tConnCbFunc callback, void* data, tBoolean data_is_resource)
{ conn->callback = callback;
  conn_change_data(conn, data, data_is_resource);
}

static void conn_cleanup_fd(tConnection* conn)
{ int fd = conn->fd;
  if (fd >= 0) { my_close_sopi(fd); conn->fd = -1; }
  conn->flags &= ~(cnfConnected | cnfReading | cnfWriting | cnfSuspended);
}

static void conn_remove(tConnection** _conn)
{ tConnection* conn = *_conn;

#if NEED_DISSOLVING
  { tConnDissolver dissolver = conn->dissolver;
    if (dissolver != NULL)
    { tBoolean did_dissolve_now = ((dissolver)(conn));
#if CONFIG_DEBUG
      sprint_safe(debugstrbuf, "dissolve(%d,%d,%p,%p)\n", conn->fd,
        did_dissolve_now, conn, dissolver);
      debugmsg(debugstrbuf);
#endif
      if (!did_dissolve_now) { *_conn = NULL; /* detach */ return; }
    }
  }
#endif

#if NEED_CONNLIST
  list_extract(&connlist_head, conn, tConnection);
#endif

#if OPTION_TLS
  { tTlsSession session = conn->tls_session;
    if (session != NULL)
    {
#if OPTION_TLS == TLS_GNUTLS
      (void) gnutls_deinit(session);
#elif OPTION_TLS == TLS_OPENSSL
      SSL_free(session);
#elif OPTION_TLS == TLS_MATRIX
      tTlsMatrixCrutch* tmc = conn->tmc;
      matrixSslDeleteSession(session);
      if (tmc != NULL)
      { memory_deallocate(tmc->incoming.buf);
        memory_deallocate(tmc->outgoing.buf);
        memory_deallocate(tmc->temp_in.buf);
        memory_deallocate(tmc);
      }
#endif
    }
  }
#endif

  { const tConnectAttemptData* cad = &(conn->cad);
    tSockaddrEntry** e = cad->sockaddrs;
    __dealloc(e);
  }

#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "conn_remove(): fd #%d\n", conn->fd);
  debugmsg(debugstrbuf);
#endif
  conn_cleanup_fd(conn);
   __dealloc(conn->writedata); memory_deallocate(conn);
  *_conn = NULL;
}

#if NEED_DISSOLVING
static __my_inline void conn_set_dissolver(tConnection* conn,
  tConnDissolver dissolver)
{ conn->dissolver = dissolver;
}
static my_inline void conn_do_dissolve(tConnection* conn)
{ conn_set_dissolver(conn, NULL); conn_remove(&conn);
}
#endif

static one_caller void check_connect(tConnection* conn)
/* checks whether a connect() attempt for a socket worked */
{ int fd = conn->fd, sockerr = 0;
  socklen_t dummy = (socklen_t) sizeof(sockerr);
  tSockaddrPortProtInfo* sppi = conn2sppi(conn, truE);
#if NEED_FD_REGISTER
  (void) fd_register_lookup(&fd);
#endif
#if USE_LWIP
  errno = 0; /* silly old lwIP versions didn't set errno on error */
  if (lwip_getsockopt(fd, SOL_SOCKET, SO_ERROR, &sockerr, &dummy) != 0)
#else
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &sockerr, &dummy) != 0)
#endif
  { conn_err = errno;
    failed:
#if CONFIG_DEBUG
    sprint_safe(debugstrbuf, "check_connect(): failed for fd #%d; %d,%d\n",
      fd, conn_err, sockerr);
    debugmsg(debugstrbuf);
#endif
#if CONFIG_CUSTOM_CONN
    { tResource* resource = conn2resource(conn);
      if ( (resource != NULL) && (doing_custom_conn(resource)) )
        custom_conn_tell_error(resource, connect_err2error(conn_err));
    }
#endif
    if (sppi != NULL) sppi->rating >>= 1;
    conn_cleanup_fd(conn);
    if (conn_connect(conn)) conn_callback(conn, ccekConnectSetup);
    else conn_callback(conn, ccekConnectFailed);
    return;
  }
  if (sockerr != 0) { conn_err = sockerr; goto failed; }
  if (sppi != NULL) sppi->rating = MIN(sppi->rating + 2, SOCKADDR_RATING_MAX);
  conn_set_prelire(conn, reFine); conn->flags |= cnfConnected;
  conn_callback(conn, ccekConnectWorked);
}

static void conn_io_handler(void* data, tFdObservationFlags flags)
{ tConnection* conn = (tConnection*) data;
  if (flags & fdofRead) conn_callback(conn, ccekRead);
  else if (flags & fdofWrite)
  { if (conn->flags & cnfConnected) conn_callback(conn, ccekWrite);
    else check_connect(conn);
  }
}

#define __conn_set_readwrite(c, f) \
  fd_observe((c)->fd, conn_io_handler, (c), (f))

static void conn_set_readwrite(tConnection* conn, tFdObservationFlags flags)
{ __conn_set_readwrite(conn, flags);
  if (flags & fdofRead) conn->flags |= cnfReading;
  else conn->flags &= ~cnfReading;
  if (flags & fdofWrite) conn->flags |= cnfWriting;
  else conn->flags &= ~cnfWriting;
}

#define conn_set_read(conn) conn_set_readwrite(conn, fdofRead)
#define conn_set_write(conn) conn_set_readwrite(conn, fdofWrite)

#if CONFIG_USER_QUERY
static void conn_set_suspend(tConnection* conn, tBoolean do_suspend)
{ tConnectionFlags flags = conn->flags;
  tBoolean is_suspended = cond2boolean(flags & cnfSuspended);
  if (do_suspend != is_suspended) /* must do something */
  { if (do_suspend)
    { __conn_set_readwrite(conn, fdofNone); conn->flags |= cnfSuspended; }
    else /* unsuspend */
    { __conn_set_readwrite(conn, (flags & cnfWriting) ? fdofWrite : fdofRead);
      conn->flags &= ~cnfSuspended;
    }
  }
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "conn_set_suspend(): fd #%d; %d,%d\n", conn->fd,
    do_suspend, is_suspended);
  debugmsg(debugstrbuf);
#endif
}
#endif

static void conn_set_software_id(tConnection* conn, const char* text)
{ tSockaddrPortProtInfo* sppi = conn2sppi(conn, truE);
  if (sppi != NULL)
  { const char *r = my_strchr(text, '\r'), *result = ( (r != NULL) ?
      my_strndup(text, r - text) : my_strdup(text) );
    __dealloc(sppi->software_id); sppi->software_id = result;
  }
}


/** TLS */

#if OPTION_TLS

static tBoolean tls_is_usable = falsE;

#define conn_using_tls(conn) ((conn)->flags & cnfTlsDedication)

tBoolean resource_in_tls(const tResource* resource,
  tBoolean whether_handshaking)
{ tBoolean retval = falsE;
  const tConnection* conn = resource->cconn;
  if (conn != NULL)
  { if (!whether_handshaking) { if (conn_using_tls(conn)) retval = truE; }
    else { if (conn->flags & cnfTlsHandshaking) retval = truE; }
  }
  return(retval);
}

static tTlsError tls_err2te(int err)
/* converts TLS library error codes to tTlsError codes */
{ tTlsError retval;
  switch (err)
  {
#if OPTION_TLS == TLS_GNUTLS
    case GNUTLS_E_EXPIRED: retval = teSessionExpired; break;
    case GNUTLS_E_FATAL_ALERT_RECEIVED: retval = teFatalAlert; break;
    case GNUTLS_E_FILE_ERROR: retval = teFile; break;
    case GNUTLS_E_GOT_APPLICATION_DATA: retval = teData; break;
    case GNUTLS_E_MEMORY_ERROR: retval = teOom; break;
    case GNUTLS_E_NO_CIPHER_SUITES: case GNUTLS_E_UNKNOWN_CIPHER_SUITE:
      retval = teCipher; break;
    case GNUTLS_E_COMPRESSION_FAILED: case GNUTLS_E_DECOMPRESSION_FAILED:
    case GNUTLS_E_NO_COMPRESSION_ALGORITHMS:
    case GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM:
      retval = teCompression; break;
    case GNUTLS_E_DECRYPTION_FAILED: case GNUTLS_E_ENCRYPTION_FAILED:
      retval = teCrypt; break;
    case GNUTLS_E_ILLEGAL_SRP_USERNAME: case GNUTLS_E_SRP_PWD_ERROR:
    case GNUTLS_E_SRP_PWD_PARSING_ERROR:
      retval = teSrp; break;
    case GNUTLS_E_PKCS1_WRONG_PAD: case GNUTLS_E_PK_DECRYPTION_FAILED:
    case GNUTLS_E_PK_ENCRYPTION_FAILED: case GNUTLS_E_PK_SIGN_FAILED:
    case GNUTLS_E_PK_SIG_VERIFY_FAILED: case GNUTLS_E_UNKNOWN_PK_ALGORITHM:
      retval = tePk; break;
    case GNUTLS_E_LARGE_PACKET: case GNUTLS_E_RECORD_LIMIT_REACHED:
    case GNUTLS_E_TOO_MANY_EMPTY_PACKETS: case GNUTLS_E_UNEXPECTED_PACKET:
    case GNUTLS_E_UNEXPECTED_PACKET_LENGTH:
    case GNUTLS_E_UNSUPPORTED_VERSION_PACKET:
      retval = tePacket; break;
    case GNUTLS_E_UNIMPLEMENTED_FEATURE: retval = teUnimplemented; break;
    case GNUTLS_E_ERROR_IN_FINISHED_PACKET:
    case GNUTLS_E_UNEXPECTED_HANDSHAKE_PACKET:
      retval = teHandshake; break;
#elif OPTION_TLS == TLS_OPENSSL /* || .... */
    /* IMPLEMENTME for the other libraries? */
#endif
    default: retval = teUnknown; break;
  }
  return(retval);
}

static void tls_set_error(tConnection* conn, tTlsError te)
{ tResource* resource = conn2resource(conn);
  if (resource != NULL)
  { resource_set_error(resource, reTls);
    if (resource->tls_error == teFine) resource->tls_error = te;
  }
}

#if OPTION_TLS == TLS_GNUTLS

#define __tls_gnutls_intr(err) \
  ( ((err) == GNUTLS_E_INTERRUPTED) || ((err) == GNUTLS_E_AGAIN) )
#define __tls_gnutls_warn(err) ((err) == GNUTLS_E_WARNING_ALERT_RECEIVED)

static
#if DO_GNUTLS_SUFFIX
  gnutls_certificate_client_credentials_t
#else
  gnutls_certificate_client_credentials
#endif
  tls_xcred;

#if CONFIG_DEBUG

static void tls_gnutls_debug(const char* operation, int err)
{ tBoolean is_error = cond2boolean(err < 0);
  const char* errstr;
  char* spfbuf;
  if (!is_error) errstr = strEmpty;
  else { errstr = gnutls_strerror(err); if (errstr == NULL) errstr=strEmpty; }

  my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf, "TLS%d: %s (%d%s%s)\n", OPTION_TLS,
    operation, err, ( (*errstr != '\0') ? ", " : strEmpty ), errstr);
  debugmsg(spfbuf);
  my_spf_cleanup(debugstrbuf, spfbuf);
  /* Don't take the identifier "err" too seriously. The great GnuTLS
     documentation often does not say anything about the meaning of the return
     value of a specific function, so it might not actually be related to error
     handling... :-( */
}

#define tls_gnutls_debug_errvalue tls_gnutls_debug

#else /* #if CONFIG_DEBUG */

#define tls_gnutls_debug(operation, err) do { } while (0)

static void tls_gnutls_debug_errvalue(const char* operation, int err)
/* Call this function only if it is _known_ (e.g. from GnuTLS documentation)
   that <err> really means an error indicator. */
{ if ( (err < 0) && (!__tls_gnutls_intr(err)) && (!__tls_gnutls_warn(err)) )
  { tTlsError te = tls_err2te(err);
    tBoolean show_te = cond2boolean(is_tls_error_expressive(te));
    const char* errstr = gnutls_strerror(err);
    char* spfbuf;
    my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      _("retawq: TLS library error during operation \"%s\": #%d%s%s - %s\n"),
      operation, err, (show_te ? strSpacedDash : strEmpty), (show_te ?
      _(strTlsError[te]) : strEmpty), null2empty(errstr));
    if (may_use_fd2()) my_write_str(fd_stderr, spfbuf);
    debugmsg(spfbuf);
    my_spf_cleanup(strbuf, spfbuf);
  }
}

#endif /* #if CONFIG_DEBUG */

static void tls_gnutls_check_direction(tConnection* conn)
{ if (gnutls_record_get_direction(conn->tls_session) == 1)
    conn_set_write(conn);
  else conn_set_read(conn);
}

#elif OPTION_TLS == TLS_OPENSSL

static SSL_CTX* tls_context = NULL;

#if CONFIG_DEBUG
static int tls_openssl_print_errtext(const char* str, size_t len,
  __sunused void* data __cunused)
{ return(my_write(debugfd, str, len));
}
static void tls_openssl_debug_errstuff(const char* funcname,
  const tConnection* conn, int err, int sslge)
{ char* spfbuf;
  my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf,
    "TLS%d: SSL_%s(): fd=%d, err=%d, sslge=%d, errno=%d%s%s\n", OPTION_TLS,
    funcname, conn->fd, err, sslge, errno, ( (errno != 0) ? strSpacedDash :
    strEmpty ), ( (errno != 0) ? my_strerror(errno) : strEmpty ));
  debugmsg(spfbuf);
  my_spf_cleanup(debugstrbuf, spfbuf);
  ERR_print_errors_cb(tls_openssl_print_errtext, NULL);
    /* (undocumented, but "hopefully not vanishing" - it is OpenSSL's sole
        non-ridiculous error printing interface for file descriptors (like
        debugfd)) */
}
#else
#define tls_openssl_debug_errstuff(funcname, conn, err, sslge) do { } while (0)
#endif

static void tls_openssl_handle_error(tConnection* conn, int sslge)
/* Just one more OpenSSL silliness... */
{ if (sslge == SSL_ERROR_SYSCALL)
  { tResource* resource = conn2resource(conn);
    if (resource != NULL)
    { tResourceError re = conn_err2error(errno);
      if (re != reConnectionFailureDefault)
      { resource_set_error(resource, re); return; }
    }
  }
  tls_set_error(conn, teUnknown);
}

#define tls_openssl_operate(operation, funcname) \
  do \
  { ERR_clear_error(); err = operation; sslge = SSL_get_error(session, err); \
    tls_openssl_debug_errstuff(funcname, conn, err, sslge); \
  } while (0)

static tBoolean tls_openssl_shutdown(tConnection* conn)
/* returns whether it "finished somehow" */
{ tBoolean retval;
  tTlsSession session = conn->tls_session;
  int err, sslge;
  tls_openssl_operate(SSL_shutdown(session), "shutdown");
  if ( (sslge == SSL_ERROR_SYSCALL) && (err == 0) )
  { /* Just one more OpenSSL silliness. SSL_shutdown() documentation says that
       sslge can be wrong if SSL_shutdown() returned 0: "The output of
       SSL_get_error(3) may be misleading, as an erroneous SSL_ERROR_SYSCALL
       may be flagged even though no error occurred." */
    /* An additional silliness: SSL_want() documentation says: "Error
       conditions are not handled and must be treated using SSL_get_error(3)."
       But since SSL_get_error() "may be misleading" - what to do instead?! */
    switch (SSL_want(session))
    { case SSL_READING: sslge = SSL_ERROR_WANT_READ; break;
      case SSL_WRITING: sslge = SSL_ERROR_WANT_WRITE; break;
    }
  }
  switch (sslge)
  { case SSL_ERROR_NONE: retval = truE; break;
    case SSL_ERROR_WANT_READ: conn_set_read(conn); retval = falsE; break;
    case SSL_ERROR_WANT_WRITE: conn_set_write(conn); retval = falsE; break;
    default: tls_openssl_handle_error(conn, sslge); retval = truE; break;
  }
  return(retval);
}

#elif OPTION_TLS == TLS_MATRIX

#if CONFIG_DEBUG
static void tls_matrix_debug_errvalue(const char* what, int err)
{ sprint_safe(debugstrbuf, "TLS%d: %s, %d\n", OPTION_TLS, what, err);
  debugmsg(debugstrbuf);
}
static void tls_matrix_debug_oserr(const char* what, int err, int err2)
{ sprint_safe(debugstrbuf, "TLS%d: %s, %d, %d\n", OPTION_TLS, what, err,
    ( (err == -1) ? err2 : 0 ));
  debugmsg(debugstrbuf);
}
static void tls_matrix_debug_msd(int msd, unsigned char msde,
  unsigned char msdal, unsigned char msdad)
{ sprint_safe(debugstrbuf, "TLS%d: msd, %d, %d, %d, %d\n", OPTION_TLS, msd,
    msde, msdal, msdad);
  debugmsg(debugstrbuf);
}
#else
#define tls_matrix_debug_errvalue(a, b) do { } while (0)
#define tls_matrix_debug_oserr(a, b, c) do { } while (0)
#define tls_matrix_debug_msd(a, b, c, d) do { } while (0)
#endif

static void tmci_buf(/*@out@*/ sslBuf_t* buf)
{ const int size = buf->size = TLS_MCBUFSIZE; /* (yes, "int"...) */
  buf->buf = buf->start = buf->end = __memory_allocate(size, mapOther);
}

static one_caller void tls_matrix_crutch_init(tConnection* conn)
{ tTlsMatrixCrutch* tmc = conn->tmc = (tTlsMatrixCrutch*)
    memory_allocate(sizeof(tTlsMatrixCrutch), mapOther);
  tmci_buf(&(tmc->incoming)); tmci_buf(&(tmc->outgoing));
  tmci_buf(&(tmc->temp_in));
}

#endif /* TLS variants */

static one_caller tBoolean tls_initialize(void)
{ static tBoolean tls_did_init = falsE;
  if (tls_did_init) goto out;
  tls_did_init = truE;

#if OPTION_TLS == TLS_GNUTLS

  { char* spfbuf;
    int err = gnutls_global_init();
    tls_gnutls_debug_errvalue(_(strInitialization), err);
    if (err != 0) goto out;
    err = gnutls_certificate_allocate_credentials(&tls_xcred);
    tls_gnutls_debug_errvalue("allocate credentials", err);
    my_spf(strbuf, STRBUF_SIZE, &spfbuf, strPercsPercs, config.path,
      "gnutls-ca.pem");
    err = gnutls_certificate_set_x509_trust_file(tls_xcred, spfbuf,
      GNUTLS_X509_FMT_PEM);
    tls_gnutls_debug("trustfile", err);
    my_spf_cleanup(strbuf, spfbuf);
    tls_is_usable = truE;
  }

#elif OPTION_TLS == TLS_OPENSSL

  { SSL_METHOD* m;
    char seedfilenamebuf[512];
    const char* seedfilename;
    time_t seedval;
    unsigned short randcount;
    SSL_load_error_strings();  /* Do something before... */
    (void) SSL_library_init(); /* ...initializing; what a sane concept! :-( */

    /* Handle this random rubbish which a sane library would do itself... */
    debugmsg("TLS: random A\n");
    if (RAND_status()) goto rand_done; /* nothing to do, nice */
    debugmsg("TLS: random B\n");
    seedfilename = RAND_file_name(seedfilenamebuf, sizeof(seedfilenamebuf));
    if (seedfilename != NULL)
    { debugmsg("TLS: random C\n");
#if CONFIG_DEBUG
      { char* spfbuf;
        my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf, "TLS%d: seedfilename=%s\n",
          OPTION_TLS, seedfilename);
        debugmsg(spfbuf);
        my_spf_cleanup(debugstrbuf, spfbuf);
      }
#endif
      if (RAND_egd(seedfilename) >= 0) goto rand_finish;
      debugmsg("TLS: random D\n");
      (void) RAND_load_file(seedfilename, -1);
        /* ("-1" is allowed for OpenSSL >= 0.9.5) */
      if (RAND_status()) goto rand_finish;
    }
    debugmsg("TLS: random E\n");
    seedval = my_time() ^ ((~getpid()) * (~getppid())); /* good enough? :-) */
    RAND_seed((const void*) (&seedval), sizeof(seedval));
    if (RAND_status()) goto rand_finish;
    debugmsg("TLS: random F\n");
    srand48(seedval);
    randcount = 10000; /* wild guess, thanks to lack of useful documentation,
      just to avoid infinite loops */
    while (randcount-- > 0)
    { long l = lrand48();
      RAND_seed((const void*) (&l), sizeof(l));
      if (RAND_status()) goto rand_counted;
    }
    debugmsg("TLS: random G\n");
    /* CHECKME: what to do here? Set reRandom? Just hope the best?? */
    rand_counted: {}
#if CONFIG_DEBUG
    sprint_safe(debugstrbuf, "TLS%d: remaining randcount: %d\n", OPTION_TLS,
      randcount);
    debugmsg(debugstrbuf);
#endif
    rand_finish:
    debugmsg("TLS: random H\n");
    if (seedfilename != NULL) RAND_write_file(seedfilename);
    rand_done:
    debugmsg("TLS: random Z\n");

    m = SSLv23_client_method();
    if ( (m == NULL) || ( (tls_context = SSL_CTX_new(m)) == NULL ) )
    { if (may_use_fd2())
        my_write_str(fd_stderr, _("retawq: OpenSSL initialization failed\n"));
      goto out;
    }
    (void) SSL_CTX_set_options(tls_context, SSL_OP_ALL);
      /* (OpenSSL documentation says this is safe; stupid-me doubts it somehow,
          esp. for SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS, but...) */
    (void) SSL_CTX_set_mode(tls_context, SSL_MODE_ENABLE_PARTIAL_WRITE);
    tls_is_usable = truE;
  }

#elif OPTION_TLS == TLS_MATRIX

  { int err = matrixSslOpen();
    tls_matrix_debug_errvalue(_(strInitialization), err);
    if (err == 0) tls_is_usable = truE;
  }

#endif

  out:
  return(tls_is_usable);
}

static one_caller void tls_deinitialize(void)
{
#if OPTION_TLS == TLS_GNUTLS
  if (tls_is_usable) gnutls_global_deinit();
#elif OPTION_TLS == TLS_OPENSSL
  if (tls_context != NULL) SSL_CTX_free(tls_context);
#elif OPTION_TLS == TLS_MATRIX
  if (tls_is_usable) matrixSslClose();
#endif
}

static unsigned char tls_do_handshaking(tConnection* conn)
{ unsigned char retval; /* 0=error, 1=proceeding, 2=done */
  tTlsSession session = conn->tls_session;

#if OPTION_TLS == TLS_GNUTLS

  int err = gnutls_handshake(session);
  tls_gnutls_debug_errvalue("handshake", err);
  if ( (__tls_gnutls_intr(err)) || (__tls_gnutls_warn(err)) )
  { tls_gnutls_check_direction(conn); retval = 1; }
  else if (err < 0) { tls_set_error(conn, tls_err2te(err)); retval = 0; }
  else retval = 2;

#elif OPTION_TLS == TLS_OPENSSL

  int err, sslge;
  tls_openssl_operate(SSL_connect(session), "connect");
  switch (sslge)
  { case SSL_ERROR_NONE: retval = 2; break;
    case SSL_ERROR_WANT_READ: conn_set_read(conn); retval = 1; break;
    case SSL_ERROR_WANT_WRITE: conn_set_write(conn); retval = 1; break;
    default: tls_openssl_handle_error(conn, sslge); retval = 0; break;
  }

#elif OPTION_TLS == TLS_MATRIX


#endif

  if (retval == 1)
  {
#if CONFIG_DEBUG
    if (!(conn->flags & cnfTlsHandshaking))
    { sprint_safe(debugstrbuf, "TLS%d: starting handshake, fd #%d\n",
        OPTION_TLS, conn->fd);
      debugmsg(debugstrbuf);
    }
#endif
    conn->flags |= cnfTlsHandshaking;
  }
  else
  { conn->flags &= ~cnfTlsHandshaking;
    if (retval == 2)
    { if (conn->flags & cnfWantToWrite) conn_set_write(conn);
      else conn_set_read(conn);
    }
  }
  return(retval);
}

static void tls_session_dissolver_callback(tConnection* conn,
  tConnCbEventKind ccek)
{
#if OPTION_TLS == TLS_GNUTLS
  tTlsSession session = conn->tls_session;
  int err;
#endif

  switch (ccek)
  {
#if OPTION_TLS == TLS_GNUTLS
    case ccekRead:
      err = gnutls_record_recv(session, NULL, 0);
      handle_err:
#if CONFIG_DEBUG
      sprint_safe(debugstrbuf, "TLS%d: dis - %d,%d,%d\n", OPTION_TLS, conn->fd,
        ccek, err);
      debugmsg(debugstrbuf);
#endif
      if ( (__tls_gnutls_intr(err)) || (__tls_gnutls_warn(err)) )
        tls_gnutls_check_direction(conn);
      else goto do_dissolve; /* error or done */
      break;
    case ccekWrite:
      err = gnutls_record_send(session, NULL, 0); goto handle_err;
      /*@notreached@*/ break;
#elif OPTION_TLS == TLS_OPENSSL
    case ccekRead: case ccekWrite:
      if (tls_openssl_shutdown(conn)) goto do_dissolve;
      break;
#elif OPTION_TLS == TLS_MATRIX
#endif
    default: conn_bug(conn, ccek); do_dissolve: conn_do_dissolve(conn); break;
  }
}

static tBoolean tls_session_dissolver(tConnection* conn)
/* returns whether it actually dissolved the session right now */
{ tBoolean retval;
#if OPTION_TLS == TLS_GNUTLS
  int err;
  conn->flags &= ~cnfDataIsResource;
  err = gnutls_alert_send(conn->tls_session, GNUTLS_AL_WARNING,
    GNUTLS_A_CLOSE_NOTIFY);
  tls_gnutls_debug("alert_send", err);
  if ( (__tls_gnutls_intr(err)) || (__tls_gnutls_warn(err)) )
  { tls_gnutls_check_direction(conn); retval = falsE; }
  else retval = truE; /* done (unlikely since non-blocking) or error */
#elif OPTION_TLS == TLS_OPENSSL
  conn->flags &= ~cnfDataIsResource;
  retval = tls_openssl_shutdown(conn);
#elif OPTION_TLS == TLS_MATRIX
#endif
  if (!retval)
  { conn_set_dissolver(conn, NULL); conn->flags |= cnfDissolving;
    conn_change_callback(conn, tls_session_dissolver_callback, NULL, falsE);
  }
  return(retval);
}

static tBoolean tls_session_init(tConnection* conn, const char* hostname)
{ tBoolean retval = falsE;
  tTlsSession session;
  tTlsError te = teFine;
#if OPTION_TLS == TLS_GNUTLS
  static const int cert_type_priority[] =
  { GNUTLS_CRT_X509, GNUTLS_CRT_OPENPGP, 0 };
  int err;
#elif OPTION_TLS == TLS_MATRIX
  int err;
#endif

  if (!tls_initialize())
  {
#if (OPTION_TLS == TLS_OPENSSL) || (OPTION_TLS == TLS_MATRIX)
    init_failed:
#endif
    te = teInit; goto out;
  }

#if OPTION_TLS == TLS_GNUTLS

  err = gnutls_init(&session, GNUTLS_CLIENT);
  tls_gnutls_debug_errvalue("initialize session", err);
  if (err != 0) { te = tls_err2te(err); goto out; }
  conn->tls_session = session;
  err = gnutls_set_default_priority(session);
  tls_gnutls_debug("set default priority", err);
  err = gnutls_certificate_type_set_priority(session, cert_type_priority);
  tls_gnutls_debug("set certificate type priority", err);
  err = gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, tls_xcred);
  tls_gnutls_debug("set credentials", err);
  gnutls_transport_set_ptr(session, (my_gnutls_transport_ptr) conn->fd);
  if (hostname != NULL)
  { err = gnutls_server_name_set(session, GNUTLS_NAME_DNS, hostname,
      strlen(hostname));
    tls_gnutls_debug("nameset", err);
  }
  /* FIXME: RFC2818, 3.1! */

#elif OPTION_TLS == TLS_OPENSSL

  session = conn->tls_session = SSL_new(tls_context);
  if (session == NULL) goto init_failed;
  (void) SSL_set_fd(session, conn->fd);

#elif OPTION_TLS == TLS_MATRIX

  err = matrixSslNewSession(&session, NULL, NULL, 0);
  tls_matrix_debug_errvalue("session init", err);
  if (err != 0) goto init_failed;
  conn->tls_session = session;
  tls_matrix_crutch_init(conn);
  err = matrixSslEncodeClientHello(session, &(conn->tmc->outgoing), 0);
  tls_matrix_debug_errvalue("client hello", err);
  if (err != 0) goto init_failed;

#endif

  /* try to start the handshaking */
  if (tls_do_handshaking(conn) == 0) goto out;

  /* 't worked */
  conn->flags |= cnfTlsDedication;
  conn_set_dissolver(conn, tls_session_dissolver);
  retval = truE;

  out:
  if (te != teFine) tls_set_error(conn, te);
  return(retval);
}

static ssize_t tls_record_read(tConnection* conn, /*@out@*/ void* buf,
  size_t size, /*@out@*/ tBoolean* _was_interrupted)
{ ssize_t retval;
  tTlsSession session = conn->tls_session;

#if OPTION_TLS == TLS_GNUTLS

  *_was_interrupted = falsE;
  retval = gnutls_record_recv(session, buf, size);
  tls_gnutls_debug_errvalue(_("read"), retval);
  if ( (__tls_gnutls_intr(retval)) || (__tls_gnutls_warn(retval)) )
  { tls_gnutls_check_direction(conn);
    zeroy: retval = 0; *_was_interrupted = truE;
  }
  else if (retval == GNUTLS_E_REHANDSHAKE)
  { if (tls_do_handshaking(conn) != 0) goto zeroy;
  }
  else if (retval < 0) tls_set_error(conn, tls_err2te(retval));

#elif OPTION_TLS == TLS_OPENSSL

  int err, sslge;
  *_was_interrupted = falsE;
  tls_openssl_operate(SSL_read(session, buf, size), "read");
  switch (sslge)
  { case SSL_ERROR_NONE: retval = err; break;
    case SSL_ERROR_WANT_READ:
      conn_set_read(conn);
      zeroy: retval = 0; *_was_interrupted = truE;
      break;
    case SSL_ERROR_WANT_WRITE: conn_set_write(conn); goto zeroy; break;
    default: tls_openssl_handle_error(conn, sslge); retval = 0; break;
  }

#elif OPTION_TLS == TLS_MATRIX


#endif

  return(retval);
}

static ssize_t tls_record_write(tConnection* conn, const void* buf,
  size_t size, /*@out@*/ tBoolean* _was_interrupted)
{ ssize_t retval;
  tTlsSession session = conn->tls_session;

#if OPTION_TLS == TLS_GNUTLS

  retval = gnutls_record_send(session, buf, size);
  tls_gnutls_debug_errvalue(_("write"), retval);
  if ( (__tls_gnutls_intr(retval)) || (__tls_gnutls_warn(retval)) )
  { tls_gnutls_check_direction(conn); *_was_interrupted = truE; retval = 0; }
  else
  { *_was_interrupted = falsE;
    if (retval < 0) tls_set_error(conn, tls_err2te(retval));
  }

#elif OPTION_TLS == TLS_OPENSSL

  int err, sslge;
  *_was_interrupted = falsE;
  tls_openssl_operate(SSL_write(session, buf, size), "write");
  switch (sslge)
  { case SSL_ERROR_NONE: retval = err; break;
    case SSL_ERROR_WANT_READ:
      conn_set_read(conn);
      zeroy: retval = 0; *_was_interrupted = truE;
      break;
    case SSL_ERROR_WANT_WRITE: conn_set_write(conn); goto zeroy; break;
    default: tls_openssl_handle_error(conn, sslge); retval = 0; break;
  }

#elif OPTION_TLS == TLS_MATRIX


#endif

  return(retval);
}

#define tls_record_write_str(conn, str, wi) \
  tls_record_write(conn, str, strlen(str), wi)

static one_caller tBoolean tls_version_is_old(const char* version)
/* returns whether the TLS/libgcrypt library version is known to be "old" */
{ static const int minpart[3] = /* "most recent" version number */
#if OPTION_TLS == TLS_GNUTLS
    { 1, 0, 24 }; /* GnuTLS */
#elif OPTION_TLS == TLS_OPENSSL
    { 0, 9, 7 }; /* OpenSSL */
#define minpart_ch ('e')
#elif OPTION_TLS == TLS_MATRIX
    { 1, 2, 2 }; /* MatrixSSL */
#elif OPTION_TLS == TLS_BUILTIN
    { 1, 2, 1 }; /* libgcrypt */
#endif
  tBoolean retval = falsE;
  const char* temp = version;
  unsigned char count;
  int part[3];

  /* try to parse the version string; since the format of this string might
     change in the future etc., let's be careful */
  for (count = 0; count <= 2; count++)
  { if (!my_isdigit(*temp)) goto out;
    my_atoi(temp, &(part[count]), &temp, 999);
    if (count < 2)
    { if (*temp != '.') goto out;
      else temp++;
    }
  }

  /* understood the version string, now check the version */
  for (count = 0; count <= 2; count++)
  { int p = part[count], m = minpart[count];
    if (p != m)
    { if (p < m) retval = truE;
      goto out;
    }
  }
#if OPTION_TLS == TLS_OPENSSL
  { char ch = *temp;
    if ( (my_islower(ch)) && (*(temp + 1) == ' ') && (ch < minpart_ch) )
      retval = truE;
  }
#undef minpart_ch
#endif

  out:
  return(retval);
}

static one_caller const char* tls_version_warning(const char* version)
/* returns an HTML warning text if the TLS/libgcrypt library version is known
   to be "old" */
{ const char* retval = (tls_version_is_old(version) ?
    _(" <b>Warning</b>: this version is out of date!") : strEmpty);
  return(retval);
}

#endif /* #if OPTION_TLS */


/** Communication with main.c */

static void resource_finalize(tResource* resource)
{ resource->flags |= rfFinal; stop_save_as(resource);
}

static void push_to_main_res(tResource* resource, unsigned char flags)
{ if (!(flags & 1)) resource_finalize(resource);
  dhm_notify(resource, dhmnfDataChange);
  /* IMPLEMENTME: sometimes just "dhmnfMetadataChange"! */
}

static void push_to_main_req(tResourceRequest* request, unsigned char flags)
{ if (!(flags & 1))
  { tResource* resource = request->resource;
    if (resource != NULL) resource_finalize(resource);
  }
  dhm_notify(request, dhmnfDataChange);
  /* IMPLEMENTME: sometimes just "dhmnfMetadataChange"! */
}

#if CONFIG_USER_QUERY

static void resource_suspend(const tResource* resource)
{ tConnection* conn = resource->cconn;
  if (conn != NULL) conn_set_suspend(conn, truE);
}

static void resource_unsuspend(const tResource* resource)
{ tConnection* conn = resource->cconn;
  if (conn != NULL) conn_set_suspend(conn, falsE);
}

static __my_inline void user_query_deallocate(const tUserQuery* query)
{ memory_deallocate(query); /* (_currently_ nothing further to do) */
}

static tBoolean resource_ask_anything(tResource* resource,
  tUserQueryCallback callback, tMissingInformationFlags mif,
  unsigned char flags)
/* flags: "&1": prior login attempt failed */
{ tBoolean retval = falsE;
  tHostPortProtInfo* hppi;
  if ( (!is_promptable)
#if CONFIG_CONSOLE
       || (program_mode == pmConsole)
#endif
     )
    goto out; /* no way of prompting a user; but IMPLEMENTME for console! */
  if (flags & 1) mif |= mifPriorLoginAttemptFailed;
  hppi = ( (mif & mifProxyrelated) ? resource->actual_hppi :
    resource->textual_hppi );
  if (hppi->flags & hppifUserQueryRunning) /* already a query running */
  { goto out; /* IMPLEMENTME: "wait" instead! */
  }
  else /* start a query */
  { tUserQuery* query = (tUserQuery*) memory_allocate(sizeof(tUserQuery),
      mapUserQuery);
    query->callback = callback; query->resource = resource; query->hppi = hppi;
    query->mif = mif; query->hostname = hppi->host->hostname;
    query->portnumber = hppi->portnumber;
    user_query_queue(query);
    if (query->mif & mifQueryFailed) /* main.c knows in advance it won't go */
    { user_query_deallocate(query); goto out; }
    resource_suspend(resource);
    hppi->flags |= hppifUserQueryRunning;
    retval = truE;
  }
  out:
  return(retval);
}

static void resource_ask_finish(tUserQuery* query)
{ tHostPortProtInfo* hppi = query->hppi;
  /* IMPLEMENTME: store login information in hppi! */
  hppi->flags &= ~hppifUserQueryRunning;
  /* IMPLEMENTME: kick any resources which were "waiting" on this hppi! */
  user_query_deallocate(query);
}

#endif /* #if CONFIG_USER_QUERY */


/** Helper functions II */

tPortnumber rp2portnumber(tResourceProtocol rp)
/* returns the default portnumber for the given protocol */
{ tPortnumber retval;
  switch (rp)
  { case rpHttp: retval = portnumber_http; break;
    case __rpFtp: case __rpFtps: retval = portnumber_ftp; break;
    case __rpFinger: retval = portnumber_finger; break;
    case rpCvs: retval = portnumber_cvs; break;
    case __rpGopher: retval = portnumber_gopher; break;
    case __rpNntp: retval = portnumber_nntp; break;
    case __rpPop: case __rpPops: retval = portnumber_pop3; break;
    case __rpHttps: retval = portnumber_https; break;
    default: retval = 0; break; /* might happen (for unknown schemes) */
  }
  return(retval);
}

void rp2scheme(tResourceProtocol rp, char* dest)
/* intended for user interface texts */
{ const char* str = NULL;
  tBoolean use_uc = cond2boolean(__has_rpflag(rp, rpfUiToupper)),
    use_alt = cond2boolean(__has_rpflag(rp, rpfUiAlternate));
  if (use_alt)
  { if (rp == __rpNntp) str = strNntp;
    else if (rp == __rpPop) str = strPop3;
  }
  if (str == NULL) str = rp_data[rp].scheme;
  if (use_uc) my_strcpy_toupper(dest, str);
  else strcpy(dest, str);
}

#if CONFIG_BLOAT & BLOAT_SSC
const char* ssc2info(tResourceProtocol rp, tServerStatusCode ssc)
/* tries to convert a server status code to a nice, human-readable string; this
   produces mostly very general texts right now - IMPLEMENTME further! */
{ const char* retval = strEmpty; /* default */
  static const char strIntermediary[] = N_("intermediary"),
    strServerError[] = N_("server error"), strOkay[] = N_("okay"),
    strAuthentReq[] = N_("authentication required");
#if OPTION_NEWS || CONFIG_FTP
  static const char strServerReady[] = N_("server ready");
#endif
#if CONFIG_FTP
  static const char strAccountRequired[] = N_("account required");
#endif

  if ( (is_httplike(rp))
#if OPTION_LOCAL_CGI
       || (rp == rpLocalCgi)
#endif
     )
  { switch (ssc / 100)
    { case 1: retval = _(strIntermediary); break;
      case 2: retval = _(strOkay); break;
      case 3: retval = _(strRedirection); break;
      case 4:
        if (ssc == 401) retval = _(strAuthentReq);
        else if (ssc == 407) retval = _(strProxyAuthentReq);
        else retval = _("request error");
        break;
      case 5: retval = _(strServerError); break;
    }
  }
#if CONFIG_FTP
  else if (is_ftplike(rp))
  { switch (ssc / 100)
    { case 1: retval = _(strIntermediary); break;
      case 2:
        if (ssc == 200) retval = _(strOkay);
        else if (ssc == 220) retval = _(strServerReady);
        else if (ssc == 221) retval = _("server closed");
        else if (ssc == 226) retval = _("transfer successful");
        else if ( (ssc == 227) || (ssc == 229) )
          retval = _("entering passive mode");
        else if ( (ssc == 230)
#if OPTION_TLS
                  || ( (rp == rpFtps) && (ssc == 232) )
#endif
                )
        { retval = _("logged in"); }
        break;
      case 3:
        if (ssc == 331) retval = _("password required");
        else if (ssc == 332) retval = _(strAccountRequired);
        else if (ssc == 350) retval = _(strIntermediary);
        break;
      case 4:
        if (ssc == 421) retval = _("service not available");
        else retval = _("transient server problem");
        break;
      case 5:
        if (ssc == 530) retval = _(strResourceError[reLogin]);
        else if (ssc == 532) retval = _(strAccountRequired);
        else if (ssc == 550) retval = _("file/directory not found");
        else retval = _(strServerError);
        break;
      case 6: retval = _("protected reply"); break;
    }
  }
#endif
#if OPTION_NEWS
  else if (rp == rpNntp)
  { switch (ssc / 100)
    { case 2:
        if ( (ssc == 200) || (ssc == 201) ) retval = _(strServerReady);
        else if (ssc == 211) retval = _("group selected");
        else if ( (ssc == 215) || (ssc == 231) )
          retval = _("news groups follow");
        else if ( (ssc >= 220) && (ssc <= 222) ) retval = _("content follows");
        break;
      case 4:
        if (ssc == 411) retval = _("no such news group");
        else if ( ( (ssc >= 421) && (ssc <= 423) ) || (ssc == 430) )
          retval = _("no such article");
        else if (ssc == 480) retval = _(strAuthentReq);
        break;
    }
  }
#endif
  return(retval);
}
#endif /* BLOAT_SSC */

static tBoolean is_filesuffix(const char* filename, const char* suff)
/* returns whether the <filename> has the suffix <suff>; the latter must be
   given in lowercase! */
{ size_t filenamelen = strlen(filename), sufflen = strlen(suff);
  const char* temp;
  if (filenamelen < sufflen + 1 /* dot */ + 1 /* at least one other char */)
    return(falsE);
  temp = filename + filenamelen - sufflen;
  if ( (*(temp - 1)) != '.' ) return(falsE);
  if (!streqcase(temp, suff)) return(falsE);
  return(truE);
}

static tResourceContentKind filesuffix2rck(const char* name)
/* calculates a resource content kind based on the suffix of <name> */
{ if ( (is_filesuffix(name, strHtml)) || (is_filesuffix(name, "htm")) ||
       (is_filesuffix(name, strShtml)) || (is_filesuffix(name, "phtml")) )
    return(rckHtml);
  return(rckUnknown);
}

static tBoolean calc_parentpath(const char* path, char* staticbuf, size_t size)
{ tBoolean retval = falsE;
  size_t pathlen = strlen(path);
  if (pathlen > 1)
  { const char* p = path + pathlen - 1;
    if (*p == chDirsep) p--;
    while (p >= path)
    { if (*p == chDirsep)
      { size_t len = p - path + 1;
        if (len + 3 < size)
        { my_memcpy(staticbuf, path, len); staticbuf[len] = '\0';
          retval = truE;
        }
        break;
      }
      p--;
    }
  }
  return(retval);
}

const char* htmlify(const char* src)
/* converts special HTML characters in <src> */
{ enum { convnum = 3 };
  static const struct
  { const char* replacement;
    unsigned short len; /* == strlen(replacement) */
    char ch;
  } conv[convnum] =
  { /* { "&quot;", 6, '"'  }, */ /* #34 */
       { "&amp;",  5, '&'  },    /* #38 */
    /* { "&#39;",  5, '\'' }, */ /* #39 */
       { "&lt;",   4, '<'  },    /* #60 */
       { "&gt;",   4, '>'  }     /* #62 */
  };
  size_t destlen;
  const char* srctmp;
  char ch, *retval, *desttmp;
  unsigned short count;
  tBoolean need_conversion = falsE;

  /* Calculate the length of the resulting string: */
  destlen = 1;
  srctmp = src;
  while ( (ch = *srctmp++) != '\0' )
  { for (count = 0; count < convnum; count++)
    { if (ch == conv[count].ch)
      { destlen += conv[count].len;
        need_conversion = truE;
        goto next_char_1;
      }
    }
    destlen++;
    next_char_1: {}
  }
  if (!need_conversion) return(src); /* the most likely case */

  /* Build the resulting string: */
  retval = desttmp = __memory_allocate(destlen, mapString);
  srctmp = src;
  while ( (ch = *srctmp++) != '\0' )
  { for (count = 0; count < convnum; count++)
    { if (ch == conv[count].ch)
      { const char* tmp = conv[count].replacement;
        while ( (ch = *tmp++) != '\0' ) *desttmp++ = ch;
        goto next_char_2;
      }
    }
    *desttmp++ = ch; /* no replacement */
    next_char_2: {}
  }
  *desttmp = '\0';
  return(retval);
}


/** Connections */

#if OPTION_TLS

static ssize_t __conn_read(tConnection* conn, /*@out@*/ void* buf,
  size_t bufsize, /*@out@*/ tBoolean* _was_interrupted)
{ ssize_t retval;
  if (conn_using_tls(conn))
  { retval = tls_record_read(conn, buf, bufsize, _was_interrupted);
    if (retval < 0) tls_set_error(conn, tls_err2te(retval));
  }
  else
  { retval = my_read_sopi(conn->fd, buf, bufsize); *_was_interrupted = falsE; }
  return(retval);
}

#define conn_read(resultvar, conn, buf, bufsize, action_if_interrupted) \
  do \
  { tBoolean was_interrupted; \
    resultvar = __conn_read(conn, buf, bufsize, &was_interrupted); \
    if (was_interrupted) { action_if_interrupted; } \
  } while (0)

#else

#define conn_read(resultvar, conn, buf, bufsize, action_if_interrupted) \
  resultvar = my_read_sopi(conn->fd, buf, bufsize)

#endif

static void conn_set_writedata(tConnection* conn, const void* data,
  size_t size)
/* sets a new message to be sent over the network for this connection */
{ __dealloc(conn->writedata); conn->writedata = data;
  conn->writedata_todo = size; conn->writedata_done = 0;
  conn->flags |= cnfWantToWrite;
#if CONFIG_DEBUG
  conn->flags &= ~cnfDebugNodata;
#endif
}

static __my_inline void conn_set_writestr(tConnection* conn, const char* str)
{ conn_set_writedata(conn, str, strlen(str));
}

static unsigned char conn_write_writedata(tConnection* conn)
/* tries to write as much of the current message as possible; return value:
   0=error, 1=proceeding, 2=done */
{ const char *data = conn->writedata, *src;
  size_t todo, done, writelen;
  ssize_t write_err;
  int fd;

  if (data == NULL) /* "should not happen" */
  { failed: conn->flags &= ~cnfWantToWrite; return(0); }
  todo = conn->writedata_todo; done = conn->writedata_done;

  src = data + done; writelen = todo - done; fd = conn->fd;
#if OPTION_TLS
  if (conn_using_tls(conn))
  { tBoolean was_interrupted;
    write_err = tls_record_write(conn, src, writelen, &was_interrupted);
    if (was_interrupted) return(1);
    if (write_err < 0) tls_set_error(conn, tls_err2te(write_err));
  }
  else
#endif
  { write_err = __my_write/*_sopi*/(fd, src, writelen); }

  if (write_err <= 0) goto failed;
  done += write_err; conn->writedata_done = done;
#if CONFIG_DEBUG
  { sprint_safe(debugstrbuf, "cww: %d wrote %d/%d:\n", fd, done, todo);
    debugmsg(debugstrbuf);
    if (!(conn->flags & cnfDebugNodata))
    { __debugmsg(src, write_err);
      if (src[write_err - 1] != '\n') debugmsg(strNewline);
    }
  }
#endif
  if (done >= todo)
  { conn->flags &= ~cnfWantToWrite; dealloc(conn->writedata); return(2); }
  else return(1);
}

#if NEED_QUITCMD_DISSOLVING

static void quitcmd_dissolver_callback(tConnection* conn,
  tConnCbEventKind ccek)
/* I like spaghetti. Hey, this function is almost readable... :-) */
{ switch (ccek)
  {
#if OPTION_TLS
    case ccekRead:
      if (!conn_using_tls(conn)) goto handle_bug;
      /* "else": */ /*@fallthrough@*/
#endif
    case ccekWrite:
      switch (conn_write_writedata(conn))
      { case 0: do_dissolve: conn_do_dissolve(conn); break;
        case 1: break;
        case 2:
#if OPTION_TLS
          if (conn_using_tls(conn))
          { /* We handled the "high-level" quitcmd, now we have to handle the
               "low-level" TLS close_notify alert. */
            conn_set_dissolver(conn, tls_session_dissolver);
            conn_remove(&conn);
          }
          else
#endif
          { goto do_dissolve; /*@notreached@*/ }
          break;
      }
      break;
    default:
#if OPTION_TLS
      handle_bug:
#endif
      conn_bug(conn, ccek); goto do_dissolve; /*@notreached@*/ break;
  }
}

static tBoolean quitcmd_dissolver(tConnection* conn)
/* returns whether it actually dissolved the session right now */
{ tBoolean retval;
  if ( (conn->writedata_done <= 0) || (!(conn->flags & cnfWantToWrite)) )
  { /* The dissolving attempt doesn't happen in the middle of a write (which
       could happen e.g. if we dissolve due to a write() call failure); so we
       can safely try to send a nice "quit" command to the server. */
    static const char strNetcmdQuit[] = "QUIT\r\n";
    conn_set_dissolver(conn, NULL); conn->flags |= cnfDissolving;
    conn_change_callback(conn, quitcmd_dissolver_callback, NULL, falsE);
    conn_set_writestr(conn, my_strdup(strNetcmdQuit));
    conn_set_write(conn);
    retval = falsE;
  }
#if OPTION_TLS
  else if (conn_using_tls(conn))
  { /* We can't send the "high-level" quitcmd, but maybe we can at least send
       the "low-level" TLS close_notify alert. */
    retval = tls_session_dissolver(conn);
  }
#endif
  else retval = truE; /* can't do anything */
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "quitcmd_dissolver(): %d,%d\n", conn->fd, retval);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

#endif /* #if NEED_QUITCMD_DISSOLVING */


/** Low-level resource handling */

#if 0
static size_t ramcachesize_used = 0, ramcachesize_freeable = 0;
#if CONFIG_DISK_CACHE
static size_t diskcachesize_used = 0;
#endif
#endif

#define HASHTABLESIZE_RC (HASHTABLESIZE)
static tResource* rc_head[HASHTABLESIZE_RC];

static tHashIndex resource_uri2hashindex(const char* _uri)
/* calculates a hash table index from the given URI; we take at most the
   first 50 characters of the URI into account - this "should be enough" for
   good hashing and we need not do "%" after each partial operation (because
   the maximum possible value, 50 * 255, fits into a tHashIndex variable). */
{ tHashIndex retval = 0;
  const unsigned char* uri = (const unsigned char*) _uri;
  unsigned char count = 50;
  while (count-- > 0)
  { const unsigned char ch = *uri++;
    if (ch == '\0') break;
    retval += ((tHashIndex) ch);
  }
  return(retval % HASHTABLESIZE_RC);
}

static void attach_resource_to_request(tResourceRequest* request,
  tResource* resource)
{ dhm_attach(request->resource, resource);
  request->state = rrsAttachedResource; request->flags |= rrfResourceChanged;
  dhm_notify(request, dhmnfAttachery);
}

static void resource_stop(tResource* resource)
{ tBoolean did = falsE;
  if (resource->cconn != NULL) { conn_remove(&(resource->cconn)); did = truE; }
  if (resource->dconn != NULL) { conn_remove(&(resource->dconn)); did = truE; }
  if (resource->save_as != NULL) { stop_save_as(resource); did = truE; }
  if ( (did) && (resource->state != rsError) ) resource->state = rsStopped;
}

#if USE_RPSD

typedef void (*tRpsdRemover)(struct tRpsdGeneric*);
typedef struct tRpsdGeneric
{ tRpsdRemover remover; /* must be first in any specific rpsd struct */
} tRpsdGeneric;

static void rpsd_remove(tResource* resource)
{ tRpsdGeneric* rpsd = resource->rpsd;
  if (rpsd != NULL)
  { tRpsdRemover remover = rpsd->remover;
    if (remover != NULL) (remover)(rpsd);
    memory_deallocate(rpsd); resource->rpsd = NULL;
  }
}

#endif

static one_caller void resource_deallocate(tResource* resource)
{ const char* uri = resource->uri_data->uri;
  const tHashIndex i = resource_uri2hashindex(uri);
#if CONFIG_DEBUG
  { char* spfbuf;
    my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf, "resource_deallocate(%p,%s)\n",
      resource, uri);
    debugmsg(spfbuf);
    my_spf_cleanup(debugstrbuf, spfbuf);
  }
#endif
  dhm_notify(resource, dhmnfRemoval);

  /* Remove from the RAM cache */

  if (rc_head[i] == resource) rc_head[i] = resource->next;
  if (resource->prev != NULL) resource->prev->next = resource->next;
  if (resource->next != NULL) resource->next->prev = resource->prev;

  /* Stop any ongoing I/O for this resource */

  resource_stop(resource);

  /* Deallocate the resource */

  debugmsg("deallocating resource\n");
  sinking_data_deallocate(&(resource->sinking_data));
  uri_detach(resource->uri_data); cantent_put(resource->cantent);
#if USE_RPSD
  rpsd_remove(resource);
#endif
  memory_deallocate(resource);
}

static void resource_dhm_control(void* _resource, __sunused void* data
  __cunused, tDhmControlCode dcc)
{ tResource* resource = (tResource*) _resource;
  switch (dcc)
  { case dhmccRefcount0:
      resource_deallocate(resource);
      /* IMPLEMENTME: only deallocate it if the "RAM quota" is exceeded! */
      break;
  }
}

static /* __sallocator -- not an "only" reference */ tResource* __callocator
  resource_create(tResourceRequest* request, /*const*/ tContentblock* content,
  size_t nominal_contentlength, tResourceContentKind kind,tResourceState state)
{ const tUriData* uri_data = request->uri_data;
  const char* uri = uri_data->uri;
  const tResourceProtocol protocol = uri_data->rp;
  const tHashIndex i = resource_uri2hashindex(uri);
  tResource* retval = memory_allocate(sizeof(tResource), mapResource);
  tCantent* cantent = cantent_create();
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "created resource at %p\n", retval);
  debugmsg(debugstrbuf);
#endif
  dhm_init(retval, resource_dhm_control, "resource");
  uri_attach(retval->uri_data, request->uri_data);
  cantent_attach(retval->cantent, cantent); cantent->kind = kind;
  attach_resource_to_request(request, retval);
  if (rc_head[i] != NULL)
  { retval->next = rc_head[i]; rc_head[i]->prev = retval; }
  rc_head[i] = retval;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  if (request->flags & rrfDownload)
    retval->flags |= rfDownload | rfActivityWatched;
#endif
  retval->protocol = protocol; retval->state = state;
  retval->sinking_data = request->sinking_data; request->sinking_data = NULL;
  cantent_set_firstcontent(cantent, content);
  retval->nominal_contentlength = nominal_contentlength;
  return(retval);
}

static tResource* resource_lookup(tResourceRequest* request)
/* tries to find a resource with the given URI (from <request>) in the internal
   resource cache and attaches it to <request> if found; make sure to call this
   only after calculating the _final_ URI (e.g. after appending a "/" to the
   name of a local directory if necessary)! */
{ const char *uri, *post1;
  tHashIndex i;
  tResource* resource;
  tResourceProtocol protocol;
  tResourceRequestFlags rrf;

  if ( (request->action != rraLoad)
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
       || (request->flags & rrfDownload)
#endif
     )
  { /* We're not "allowed" to look into the cache for this request, e.g.
       because the user wants to _re_-load the resource. */
    goto out;
  }
  if ( (request->flags & rrfIsRedirection) &&
       (is_httplike(request->uri_data->rp)) )
    goto out; /* cf. comment about Apache httpd in http_setup_reqstr() */

  /* Look into the RAM cache */

  uri = request->uri_data->uri; i = resource_uri2hashindex(uri);
  resource = rc_head[i];
  if (resource == NULL) goto try_disk; /* (optimizing for most likely case) */
  protocol = request->uri_data->rp; rrf = request->flags;
  post1 = null2empty(request->uri_data->post);

  while (resource != NULL)
  { if (resource->protocol != protocol) goto do_next; /* different protocols */
    if ( ((rrf & rrfPost) != 0) != ((resource->flags & rfPost) != 0) )
    { goto do_next; } /* different method */
    if (strcmp(resource->uri_data->uri, uri)) goto do_next; /* different URI */
    if (strcmp(post1, null2empty(resource->uri_data->post))) goto do_next;
    /* CHECKME: also require identical authorization? */
    attach_resource_to_request(request, resource);
#if CONFIG_DEBUG
    { char* spfbuf;
      my_spf(debugstrbuf, STRBUF_SIZE, &spfbuf,
        "found resource in RAM cache (%p,%s)\n", resource, uri);
      debugmsg(spfbuf);
      my_spf_cleanup(debugstrbuf, spfbuf);
    }
#endif
    return(resource);
    do_next: resource = resource->next;
  }

  /* Look into the disk cache */

  try_disk: {}

  /* Nothing found... */

  out:
  return(NULL);
}

#if CONFIG_EXTRA & EXTRA_DOWNLOAD
static char* download_buf = NULL;
static size_t download_buf_size;
#endif

static void resource_provide_room(tResource* resource, /*@out@*/ char** dest,
  /*@out@*/ size_t* size, size_t desired_size, unsigned char flags)
/* <desired_size> and <flags> are only relevant if resource->lastcontent isn't
   used: "&1": desired_size is exact; "&2": it's parameter for ocbs();
   otherwise it's minimum */
{ tCantent* cantent = resource->cantent;
  tContentblock* lastcontent;
  size_t s;
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  const tBoolean is_download = cond2boolean(resource->flags & rfDownload);
#endif

  if (
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
       (!is_download) &&
#endif
       ( (lastcontent = cantent->lastcontent) != NULL ) )
  { const size_t usable = lastcontent->usable, used = lastcontent->used;
    if (used < usable) /* there's some free room left */
    { *dest = lastcontent->data + used; *size = usable - used; return; }
  }

  if ( (flags & 1) && (desired_size > 0) )
  { /* ("desired_size > 0" "should" always be true if "flags & 1"...) */
    s = desired_size;
  }
  else if (flags & 2) s = optimal_contentblocksize(desired_size);
  else
  { size_t ocbs = optimal_contentblocksize(0);
    s = MAX(desired_size, ocbs);
  }

#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  if (is_download)
  { if (download_buf == NULL) download_buf_size = 4096;
    if ( (download_buf_size < s) && (download_buf_size < HIGHEST_OCBS) )
    { while ( (download_buf_size < s) && (download_buf_size < HIGHEST_OCBS) )
        download_buf_size <<= 1;
      dealloc(download_buf);
    }
    if (download_buf == NULL)
      download_buf = __memory_allocate(download_buf_size, mapPermanent);
    *dest = download_buf; *size = download_buf_size;
  }
  else
#endif
  { const tContentblock* block = cantent_append_new_contentblock(cantent, s);
    *dest = block->data; *size = block->usable;
  }
}

static void resource_record(tResource* resource, const char* data, size_t size)
{
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  if (resource->flags & rfDownload)
  { const int fd = resource->sinking_data->download_fd;
    const ssize_t err = my_write(fd, data, size);
    if (err < 0) resource_set_error(resource, conn_err2error(errno));
    /* FIXME: handle <err> further, stop the downloading, notify user for
       custom connections! */
    if ( (data == download_buf) && (size >= download_buf_size) )
    { /* looks like the server was fast enough to fill our buffer completely,
         so a larger buffer might increase throughput */
      if (download_buf_size < HIGHEST_OCBS)
      { download_buf_size <<= 1; memory_deallocate(download_buf);
        download_buf = __memory_allocate(download_buf_size, mapPermanent);
      }
    }
  }
  else
#endif
  { resource->cantent->lastcontent->used += size;
    do_save_as(resource, data, size);
  }
  resource->bytecount += size;
  if (resource->flags & rfActivityWatched) got_activity = truE;
}

static void resource_collect(tResource* resource, const char* src, size_t size)
{
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  if (resource->flags & rfDownload) resource_record(resource, src, size);
  else /* might have to record it in smaller pieces */
#endif
  { size_t available_size, copy_size;
    tBoolean did_loop = falsE;
    char* dest;
    loop:
    resource_provide_room(resource, &dest, &available_size, size, 0);
    copy_size = MIN(size, available_size);
    my_memcpy(dest, src, copy_size); src += copy_size; size -= copy_size;
    resource_record(resource, dest, copy_size);
    if ( (size > 0) && (!did_loop) ) { did_loop = truE; goto loop; }
#if CONFIG_DEBUG
    if (size > 0) debugmsg("BUG: resource_collect()\n"); /* "can't happen" */
#endif
  }
}

static __my_inline void resource_collect_str(tResource* resource,
  const char* str)
{ resource_collect(resource, str, strlen(str));
}

static void resource_collect_title2(tResource* resource, const char* h,
  const char* b)
/* builds an HTML document intro with title strings for head and body parts */
{ char buf[STRBUF_SIZE];
  char* spfbuf;
  my_spf(buf, STRBUF_SIZE, &spfbuf, strHtmlPageTitle, h, b);
  resource_collect_str(resource, spfbuf);
  my_spf_cleanup(buf, spfbuf);
}

static __my_inline void resource_collect_title(tResource* resource,
  const char* title)
{ resource_collect_title2(resource, title, title);
}


/* Some stuff for HTTP/about resources */

#if OPTION_COOKIES
#include "cookie.c"
#else
#define cookie_collect(resource) (strEmpty)
#define cookie_collect_cleanup(str) do { } while (0)
#endif

/* begin-autogenerated */
my_enum1 enum
{ hhpsDontCare = 0, hhpsConnection = 1, hhpsContentLength = 2,
  hhpsContentType = 3, hhpsLocation = 4, hhpsServer = 5, hhpsSetCookie2 = 6,
  hhpsSetCookie = 7, hhpsStatus = 8, hhpsTransferEncoding = 9,
  hhpsWwwAuthenticate = 10
} my_enum2(unsigned char) tHttpHeaderParsingState;
#define MAX_HHPS (10)

static const char* const strHhps[MAX_HHPS + 1] =
{ strA /*don't care*/, "connection:", "content-length:", "content-type:",
  "location:", "server:", "set-cookie2:", "set-cookie:", "status:",
  "transfer-encoding:", "www-authenticate:"
};

static const unsigned char hhpslen[MAX_HHPS + 1] =
{ 1, 11, 15, 13, 9, 7, 12, 11, 7, 18, 17 };

#if CONFIG_ABOUT & 4
static one_caller void prepare_about_ctconfig(void)
{ sprint_safe(strbuf, "OPTION_TEXTMODEMOUSE = %d\n<br>OPTION_I18N = %d\n<br>OPTION_CED = %d\n<br>OPTION_COOKIES = %d\n<br>OPTION_NEWS = %d\n<br>OPTION_LOCAL_CGI = %d\n<br>OPTION_EXECEXT = %d\n<br>OPTION_TLS = %d\n<br>OPTION_IPV6 = %d\n<br>OPTION_THREADING = %d\n<br>OPTION_BIRTCFG = %d", OPTION_TEXTMODEMOUSE, OPTION_I18N, OPTION_CED, OPTION_COOKIES, OPTION_NEWS, OPTION_LOCAL_CGI, OPTION_EXECEXT, OPTION_TLS, OPTION_IPV6, OPTION_THREADING, OPTION_BIRTCFG);
}
#endif
/* end-autogenerated */


/** "about:" resources */

#if USE_S2U

tBoolean resource_ui_conn_ip(const tResource* resource, char* buf, size_t size)
/* produces a user interface string containing an IP address related to the
   <resource> if appropriate */
{ tBoolean retval = falsE;
  const tConnection* conn;
  const tSockaddrEntry* entry;
  if ( (resource->state == rsConnecting) &&
       ( (conn = resource->cconn) != NULL ) &&
       ( (entry = conn2sockaddr(conn)) != NULL ) )
  { sockaddr2uistr(entry, buf, size);
    if (*buf != '\0') retval = truE;
  }
  /* IMPLEMENTME: append portnumber; check ->dconn for FTP-like protocols? */
  return(retval);
}

#endif

static one_caller void fetch_about(tResourceRequest* request)
{ const char* path;
  if (resource_lookup(request) != NULL) return; /* found in cache, done */
  path = request->uri_data->path;

  if (!strcmp(path, strRetawq))
  { tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckHtml, rsComplete);
    char* spfbuf;

    /* introductory text */

#if CONFIG_TG == TG_CONSOLE
#define WHAT ", console"
#elif CONFIG_TG == TG_NCURSES
#define WHAT ", ncurses"
#elif CONFIG_TG == TG_XCURSES
#define WHAT ", xcurses"
#elif CONFIG_TG == TG_BICURSES
#define WHAT ", bicurses"
#elif TGC_IS_CURSES
#define WHAT ", curses"
#elif CONFIG_TG == TG_X
#define WHAT ", X Window System"
#elif CONFIG_TG == TG_GTK
#define WHAT ", GTK"
#else
#define WHAT (strEmpty)
#endif

    my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<html><head><title>About retawq</title></head>\n<body>\n<h3 align=\"center\"><b>retawq %s (%s mode%s)</b></h3>\n<p>The web browser <b>retawq</b> is released <b>without any warranty</b>. The project home page is <a href=\"http://retawq.sourceforge.net/\">retawq.sourceforge.net</a>.</p>\n"), RETAWQ_VERSION, _(strTG), WHAT); /* what the h[ae]ck... :-) */
    resource_collect_str(resource, spfbuf);
    my_spf_cleanup(strbuf, spfbuf);

#undef WHAT

    /* support text */

#if OPTION_I18N || CONFIG_JAVASCRIPT || CONFIG_CSS || OPTION_TLS
    { static const char strCommaSpace[] = ", ";
      tBoolean is_first = truE;
      resource_collect_str(resource,
        _("<p>This program contains support for: "));
#define ADDSUPP(str) \
  do \
  { if (is_first) is_first = falsE; \
    else resource_collect_str(resource, strCommaSpace); \
    resource_collect_str(resource, str); \
  } while (0)
#if OPTION_I18N
      ADDSUPP("i18n");
#endif
#if CONFIG_JAVASCRIPT
      ADDSUPP("Javascript");
#endif
#if CONFIG_CSS
      ADDSUPP("CSS");
#endif
#if OPTION_TLS
      ADDSUPP(strTLS);
#endif
#undef ADDSUPP
      resource_collect_str(resource, ".</p>\n");
    }
#endif

    /* UI library text */

    spfbuf = NULL;

#if CONFIG_TG == TG_X

    if (xws_display != NULL)
    { my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://www.x.org/\">X Window System</a> library version %d.%d (vendor: %s, release %d) on display \"%s\".</p>\n"), ProtocolVersion(xws_display), ProtocolRevision(xws_display), ServerVendor(xws_display), VendorRelease(xws_display), DisplayString(xws_display));
    }

#elif CONFIG_TG == TG_GTK

    my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://www.gtk.org/\">GTK</a> library version %d.%d.%d on display \"%s\".</p>\n"), gtk_major_version, gtk_minor_version, gtk_micro_version, gdk_get_display()); /* gdk_display_name */

#elif (CONFIG_TG == TG_NCURSES) && defined(NCURSES_VERSION)

/* work-around for ncurses silliness... */
#if (defined(NCURSES_VERSION_MAJOR)) && (NCURSES_VERSION_MAJOR >= 5)
#define my_curses_version curses_version()
#else
#define my_curses_version NCURSES_VERSION
#endif

    my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://directory.fsf.org/ncurses.html\">ncurses</a> library version \"%s\".</p>\n"), my_curses_version);

#endif

    if (spfbuf != NULL)
    { resource_collect_str(resource, spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
    }

    /* TLS/libgcrypt library text */

#if OPTION_TLS == TLS_GNUTLS
    { const char* v = gnutls_check_version(NULL);
      if (v == NULL) v = _(strUnknown);
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://www.gnutls.org/\">GnuTLS</a> library version %s.%s</p>\n"), v, tls_version_warning(v));
      resource_collect_str(resource, spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
    }
#elif OPTION_TLS == TLS_OPENSSL
    { const char *v = SSLeay_version(SSLEAY_VERSION), *v2 = null2empty(v),
        *v3 = ((strneqcase(v2, "openssl ", 8)) ? v2 + 8 : v2);
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://www.openssl.org/\">OpenSSL</a> library version \"%s\".%s</p>\n"), v2, tls_version_warning(v3));
      resource_collect_str(resource, spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
    }
#elif OPTION_TLS == TLS_MATRIX
    { /* IMPLEMENTME: version number?! */
      resource_collect_str(resource, _("<p>Using <a href=\"http://www.matrixssl.org/\">MatrixSSL</a>.</p>\n"));
    }
#elif OPTION_TLS == TLS_BUILTIN
    { const char *v = gcry_check_version(NULL), *v2 = null2empty(v);
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>Currently using <a href=\"http://www.gnupg.org/\">libgcrypt</a> library version %s.%s</p>\n"), v2, tls_version_warning(v2));
      resource_collect_str(resource, spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
    }
#endif

    /* other texts */

#if OFWAX
    resource_collect_str(resource, "<p>This is a modified version for use with <a href=\"http://www.modest-proposals.com/Hacklin.htm\">Xwoaf</a>.</p>\n");
#endif
    if (initial_messages != NULL)
    { resource_collect_str(resource, _("<p>Start messages:"));
      resource_collect_str(resource, initial_messages);
      resource_collect_str(resource, "</p>\n");
    }
    resource_collect_str(resource, strEndBodyHtml);
  }
#if CONFIG_ABOUT & 1
  else if (!strcmp(path, "activity")) /* connection activity list */
  { tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckHtml, rsComplete);
    const tConnection* conn = connlist_head;
    tBoolean is_first = truE;
    char schemebuf[MAXSCHEMESTRSIZE];
    resource->flags |= rfActivityUnwatchable;
    resource_collect_title(resource, _("Activity"));
    while (conn != NULL)
    { const tHostPortProtInfo* hppi = conn->hppi;
      const tResource* res;
      const char* info = NULL;
      char* spfbuf;
      tBoolean must_cleanup_info;
      /* IMPLEMENTME: reasons for not actually listing a connection? */
      /* list this connection */
      is_first = must_cleanup_info = falsE;
#if NEED_DISSOLVING
      if (conn->flags & cnfDissolving)
      { info = unconstify_or_("disconnecting");
        /* technically wrong notion, but users will understand it better than
           "dissolving" or "shutting down" :-) */
        goto finish;
      }
#endif
      res = conn2resource(conn);
      if (res != NULL)
      { size_t bytecount = res->bytecount;
        char bytecountbuf[200];
        if (bytecount < 2) *bytecountbuf = '\0';
        else
        { sprint_safe(bytecountbuf, "%s%d %s", strSpacedDash, bytecount,
            _(strBytes));
        }
        /* IMPLEMENTME: show "saving to FILENAME" for res->save_as chain! */
        my_spf(strbuf2, STRBUF_SIZE, &spfbuf, strPercsPercs,
          res->uri_data->uri, bytecountbuf);
        info = my_spf_use(spfbuf); must_cleanup_info = truE;
      }

      if (hppi != NULL)
      { /* IMPLEMENTME: show hostinfo! */
      }

#if OPTION_TLS
      { tTlsSession session = conn->tls_session;
        if (session != NULL) { /* IMPLEMENTME: show TLS info (cipher etc.) */ }
      }
#endif

      finish:
      rp2scheme(conn->protocol, schemebuf);
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("<p>fd #%d, scheme %s%s%s</p>"),
        conn->fd, schemebuf, ( (info != NULL) ? strSpacedDash : strEmpty ),
        null2empty(info));
      resource_collect_str(resource, spfbuf);
      if (must_cleanup_info) my_spf_cleanup(strbuf2, info);
      my_spf_cleanup(strbuf, spfbuf);
      conn = conn->next;
    }
    if (is_first) resource_collect_str(resource, _("<p>none</p>"));
    resource_collect_str(resource, strNewlineEndBodyHtml);
  }
#endif /* #if CONFIG_ABOUT & 1 */
#if CONFIG_ABOUT & 2
  else if (!strcmp(path, "hostinfo")) /* host information cache excerpt */
  { /* This existed mostly for debugging purposes, but... */
    static const char strNewlineEndp[] = "\n</p>";
    tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckHtml, rsComplete);
    tBoolean did_something1 = falsE, did_something2 = falsE;
    char* spfbuf;
    tHashIndex i;
    tSockaddrIndex si;
    char schemebuf[MAXSCHEMESTRSIZE];

    resource_collect_title(resource,
      _("Contents of the host information cache"));

    for (i = 0; i < HASHTABLESIZE_CHI; i++)
    { const tCachedHostInformation* hostinfo = chi_head[i];
      if (hostinfo == NULL) continue;
      did_something1 = truE;
      while (hostinfo != NULL)
      { const tHostPortProtInfo* hppi = hostinfo->hppi;
        my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("\n<p>name: \"%s\"; flags: %d"),
          hostinfo->hostname, hostinfo->flags);
        resource_collect_str(resource, spfbuf);
        my_spf_cleanup(strbuf, spfbuf);
#if USE_S2U
        for (si = 0; si < hostinfo->num_sockaddrs; si++)
        { sockaddr2uistr(hostinfo->sockaddrs[si], strbuf, STRBUF_SIZE);
          if (*strbuf != '\0')
          { resource_collect_str(resource, _("; IP address: "));
            resource_collect_str(resource, strbuf);
          }
        }
#endif
        while (hppi != NULL)
        { tResourceProtocol protocol = hppi->protocol;
          rp2scheme(protocol, schemebuf);
          my_spf(strbuf, STRBUF_SIZE, &spfbuf,
            _("\n<br>port %d: scheme %s; flags: %d"),
            ntohs(hppi->portnumber), schemebuf, hppi->flags);
          resource_collect_str(resource, spfbuf);
          my_spf_cleanup(strbuf, spfbuf);
          hppi = hppi->next;
        }
#if OPTION_COOKIES
        if (hostinfo->cookies != NULL)
        { const char* buf = cookie_reviewlist(hostinfo);
          if (buf != NULL)
          { resource_collect_str(resource, buf); memory_deallocate(buf); }
        }
#endif
        resource_collect_str(resource, strNewlineEndp);
        hostinfo = hostinfo->next;
      }
    }

#if USE_S2U
    for (i = 0; i < HASHTABLESIZE_SOCKADDRS; i++)
    { tSockaddrEntry* entry = sockaddr_entry_head[i];
      if (entry == NULL) continue;
      if ( (did_something1) && (!did_something2) )
      { resource_collect_str(resource,
          "\n<p><hr width=\"60%\" align=\"center\"><p>");
      }
      did_something2 = truE;
      while (entry != NULL)
      { sockaddr2uistr(entry, strbuf, STRBUF_SIZE);
        if (*strbuf != '\0')
        { tSockaddrPortProtInfo* sppi;
          resource_collect_str(resource, _("\n<p>IP address: "));
          resource_collect_str(resource, strbuf);
          sppi = entry->sppi;
          while (sppi != NULL)
          { const char *swid = sppi->software_id, *swid2 = ( (swid != NULL) ?
              htmlify(swid) : NULL );
            const tBoolean do_swid = cond2boolean(swid2 != NULL);
            tSockaddrPortProtInfoFlags sppif = sppi->sppif;
            rp2scheme(sppi->protocol, schemebuf);
            my_spf(strbuf, STRBUF_SIZE, &spfbuf,
              _("\n<br>port %d: scheme %s; flags: %d; rating: %d%s%s%s%s"),
              ntohs(sppi->portnumber), schemebuf, sppif, sppi->rating,
              ( (sppif & sppifCannotHttp11) ? _("; not HTTP/1.1") : strEmpty ),
              (do_swid ? _("; server: \"") : strEmpty), (do_swid ? swid2 :
              strEmpty), (do_swid ? strDoubleQuote : strEmpty));
            if (do_swid) htmlify_cleanup(swid, swid2);
            resource_collect_str(resource, spfbuf);
            my_spf_cleanup(strbuf, spfbuf);
            sppi = sppi->next;
          }
          resource_collect_str(resource, strNewlineEndp);
        }
        entry = entry->next;
      }
    }
#endif
    if ( (!did_something1) && (!did_something2) )
      resource_collect_str(resource, _("<p>No information known yet.</p>\n"));
    resource_collect_str(resource, strNewlineEndBodyHtml);
  }
#endif /* #if CONFIG_ABOUT & 2 */
#if CONFIG_ABOUT & 4
  else if (!strcmp(path, "ctconfig")) /* compile-time configuration */
  { tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckHtml, rsComplete);
    resource_collect_title(resource,
      _("retawq Compile-Time Configuration Options"));
    prepare_about_ctconfig(); resource_collect_str(resource, strbuf);
    resource_collect_str(resource, strNewlineEndBodyHtml);
  }
#endif /* #if CONFIG_ABOUT & 4 */
#if CONFIG_ABOUT & 8
  else if (!strcmp(path, strHelp)) /* go to documentation */
  { tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckHtml, rsComplete);
    resource_collect_title(resource, _("Help"));
#define HELP1 PATH_INSTALL_DOC "/index.html"
#define HELP2 "http://retawq.sourceforge.net/docu/"
    resource_collect_str(resource, "<p><a href=\"local:" HELP1 "\">" HELP1
      "</a>\n<br><a href=\"" HELP2 "\">" HELP2 "</a></p>");
    resource_collect_str(resource, strNewlineEndBodyHtml);
#undef HELP1
#undef HELP2
  }
#endif /* #if CONFIG_ABOUT & 8 */
#if (CONFIG_RTCONFIG) && (OPTION_BIRTCFG)
  else if (!strcmp(path, "birtcfg")) /* show built-in run-time configuration */
  { /* This one doesn't have a CONFIG_ABOUT bit. If a birtcfg exists, users
       shall be able to see what it looks like. A (bi)rtcfg can contain
       settings which cause security/privacy problems, and users surely want to
       know. */
    tResource* resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckText, rsComplete);
    resource_collect_str(resource, strBirtcfg);
  }
#endif /* #if (CONFIG_RTCONFIG) && (OPTION_BIRTCFG) */
  else
  { resource_request_set_error(request, reUri);
  }
}


/** "local:" resources */

typedef struct
{
#if CONFIG_LOCALDIR > 1
  struct stat statbuf;
#endif
  const char *key, *value;
#if CONFIG_LOCALDIR > 1
  tBoolean statbuf_is_good;
#endif
} tDirSorterElement;

#if CONFIG_LOCALDIR > 1

static const char strQuerySort[] = "?sort=";
static const char* current_localdirsort;

static one_caller char* local_query2sorting(const char* query)
{ char* retval = NULL;
  enum { buflen = 20 };
  static char buf[buflen];
  if (query != NULL)
  { size_t len = strlen(strQuerySort) - 1, slen;
    if (my_strstr(query, strQuerySort + 1))
    { const char *sorting = query + len, *end, *end_max;
      if (*sorting == '\0') goto out;
      end = my_strchr(sorting, '&');
      if (end == NULL) end = sorting + strlen(sorting);
      end_max = sorting + buflen - 5;
      if (end > end_max) end = end_max;
      slen = end - sorting;
      my_memcpy(buf, sorting, slen);
      buf[slen] = '\0';
      check_localdirsort(buf);
      if (*buf != '\0') retval = buf;
    }
  }
  out:
  return(retval);
}

static int mode2num(mode_t mode)
/* converts a file type (file/directory/...) to a number for sorting */
{ int retval;
  if (S_ISDIR(mode)) retval = 1;
  else if (S_ISREG(mode)) retval = 2;
  else retval = 3;
  return(retval);
}

static one_caller /*@observer@*/ /*@null@*/ const char*
  path2format(const char* path)
{ const char* retval = NULL;
  const tConfigLocaldirformat* ldf = config.ldf;
  while (ldf != NULL)
  { const char* pattern = ldf->path_pattern;
    size_t len;
    if ( (pattern == NULL) || (*pattern == '\0') ) goto nxt;
    len = strlen(pattern);
    if ( ( (pattern[len - 1] == '*') && (!strncmp(pattern, path, len - 1)) ) ||
         (!strcmp(pattern, path)) )
    { retval = ldf->format;
      if ( (retval != NULL) && (*retval == '\0') ) retval = NULL;
      break;
    }
    nxt:
    ldf = ldf->next;
  }
  return(retval);
}

#endif /* #if CONFIG_LOCALDIR > 1 */

static int directory_sorter(const void* _a, const void* _b)
{ const tDirSorterElement *a = (const tDirSorterElement*) _a,
    *b = (const tDirSorterElement*) _b;
#if CONFIG_LOCALDIR > 1
  const struct stat *as, *bs;
  const char* sorting;
  char sortcrit;

  if (*current_localdirsort == '\0') goto out; /* the most likely case */
  if ( (!(a->statbuf_is_good)) || (!(b->statbuf_is_good)) ) goto out;
  as = &(a->statbuf); bs = &(b->statbuf);
  sorting = current_localdirsort;
  while ( (sortcrit = *sorting++) != '\0' )
  { int val;
    tBoolean reverse;
    if (my_islower(sortcrit)) reverse = falsE;
    else { sortcrit = my_tolower(sortcrit); reverse = truE; }
    switch (sortcrit)
    { case 'g': val = my_numcmp(as->st_gid, bs->st_gid); break;
      case 'i': val = my_strcasecmp(a->key, b->key); break;
      case 'm': val = my_numcmp(as->st_mtime, bs->st_mtime); break;
      case 'n': val = strcmp(a->key, b->key); break;
      case 's': val = my_numcmp(as->st_size, bs->st_size); break;
      case 't': val = mode2num(as->st_mode) - mode2num(bs->st_mode); break;
      case 'u': val = my_numcmp(as->st_uid, bs->st_uid); break;
      default: val = 0; break; /* "should not happen" */
    }
    if (val != 0) /* found an "applicable" sorting criterium */
    { if (reverse) val = -val;
      return(val);
    }
  }
  return(0); /* no criterium applied */
  out: {}
#endif
  return(strcmp(a->key, b->key)); /* just sort by name */
}

#if CONFIG_LOCALDIR > 0
static my_inline char inodemode2str(mode_t mode)
{ if (S_ISREG(mode)) return(config.char_file);
  else if (S_ISDIR(mode)) return(config.char_dir);
  else return('?');
}
static char dirperms_buf[10];
#define __dirperms(pr, pw, px) \
  *p++ = ( (mode & pr) ? ('r') : ('-') ); \
  *p++ = ( (mode & pw) ? ('w') : ('-') ); \
  *p++ = ( (mode & px) ? ('x') : ('-') );
static one_caller void calc_dirperms(mode_t mode)
{ char* p = dirperms_buf;
  __dirperms(S_IRUSR, S_IWUSR, S_IXUSR)
  __dirperms(S_IRGRP, S_IWGRP, S_IXGRP)
  __dirperms(S_IROTH, S_IWOTH, S_IXOTH)
  *p = '\0';
}
#endif /* #if CONFIG_LOCALDIR > 0 */

static /*@observer@*/ const char strParentDirectory[] = N_("Parent directory");

static one_caller tBoolean fetch_local_directory(tResourceRequest* request)
/* returns whether it worked */
{ const char *path = request->uri_data->path, *htmlpath, *spfbuf2;
#if CONFIG_LOCALDIR > 1
  const char* format;
#endif
  char *spfbuf, *temp;
  const struct dirent* entry;
  unsigned int element_count, element_maxcount, i;
  /*@relnull@*/ tDirSorterElement* sorter_base;
  tResource* resource;
  DIR* dir = opendir(path);
  if (dir == NULL) return(falsE);

  /* Read the directory contents */

#if CONFIG_LOCALDIR > 1
  format = path2format(path);
#endif
  sorter_base = NULL;
  element_count = element_maxcount = 0;
  while ( (entry = readdir(dir)) != NULL )
  { const char *name = entry->d_name, *key, *htmlkey;
    if (name == NULL) continue; /* "should not happen", library bug */
    if (*name == '.')
    { char c = name[1];
      if ( (c == '\0') || ( (c == '.') && (name[2] == '\0') ) )
      { continue; } /* we aren't interested in "." and ".." */
    }

    if (element_maxcount <= element_count)
    { element_maxcount += 20;
      sorter_base = memory_reallocate(sorter_base, element_maxcount *
        sizeof(tDirSorterElement), mapOther);
    }
    key = my_strdup(name); htmlkey = htmlify(key);
    sorter_base[element_count].key = key;

#if CONFIG_LOCALDIR > 0
    /* The user wants more information about the contents of the directory. */
    { int err;
      struct stat statbuf;
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, strPercsPercs, path, key);
      err = my_stat(spfbuf, &statbuf);
      my_spf_cleanup(strbuf, spfbuf);
      if (err != 0)
      { /* very unlikely... */
#if CONFIG_LOCALDIR > 1
        sorter_base[element_count].statbuf_is_good = falsE;
#endif
        goto just_the_name;
      }
      else
      { mode_t mode = statbuf.st_mode;
        const char* slash = (S_ISDIR(mode) ? strSlash : strEmpty);
        if (!S_ISREG(mode)) spfbuf2 = strEmpty;
        else
        { const size_t bytes = (size_t) statbuf.st_size;
          my_spf(strbuf2, STRBUF_SIZE, &temp, strBracedNumstr,
            localized_size(bytes), bytebytes(bytes));
          spfbuf2 = my_spf_use(temp);
        }
        calc_dirperms(mode);
        my_spf(NULL, 0, &spfbuf,
          "<br>%c %s <a href=\"local:%s%s%s\">%s%s</a>%s\n",
          inodemode2str(mode), dirperms_buf, path, key, slash, htmlkey,
          slash, spfbuf2);
        if (spfbuf2 != strEmpty) my_spf_cleanup(strbuf2, spfbuf2);
#if CONFIG_LOCALDIR > 1
        sorter_base[element_count].statbuf = statbuf;
        sorter_base[element_count].statbuf_is_good = truE;
#endif
      }
    }
    if (falsE) /* ugly (-: */
#endif /* #if CONFIG_LOCALDIR > 0 */
    { just_the_name:
      my_spf(NULL, 0, &spfbuf, "<br><a href=\"local:%s%s\">%s</a>\n",
        path, key, htmlkey);
    }
    sorter_base[element_count].value = my_spf_use(spfbuf);

    element_count++;
    htmlify_cleanup(key, htmlkey);
  }
  (void) closedir(dir);

  /* Put the directory contents into an HTML page (sorted) */

  resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH, rckHtml,
    rsComplete);

  htmlpath = htmlify(path);
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("Contents of local directory \"%s\""),
    htmlpath);
  htmlify_cleanup(path, htmlpath);
  resource_collect_title(resource, spfbuf);
  my_spf_cleanup(strbuf, spfbuf);

  if (calc_parentpath(path, strbuf2, STRBUF_SIZE))
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      "<p><a href=\"local:%s\">%s</a></p>\n", strbuf2, _(strParentDirectory));
    resource_collect_str(resource, spfbuf);
    my_spf_cleanup(strbuf, spfbuf);
  }

  if (element_count <= 0)
  { resource_collect_str(resource, _("This directory is empty."));
  }
  else
  {
#if CONFIG_LOCALDIR > 1
    const char* clds = local_query2sorting(request->uri_data->query);
    if (clds == NULL) /* no special sorting for this URI given */
    { tConfigLocaldirsort* lds = config.lds;
      while (lds != NULL)
      { const char* pattern = lds->path_pattern;
        size_t len;
        if ( (pattern == NULL) || (*pattern == '\0') ) goto nxt;
        len = strlen(pattern);
        if ( ( (pattern[len - 1] == '*') && (!strncmp(pattern,path,len-1)) ) ||
             (!strcmp(pattern, path)) )
        { clds = lds->sorting; break; }
        nxt:
        lds = lds->next;
      }
    }
    if (clds == NULL) clds = strEmpty; /* sort by name (default) */
    else if (*clds == '_') clds = NULL; /* disable all sorting */
    if (clds != NULL)
    { current_localdirsort = clds;
#else
    {
#endif
      qsort(sorter_base, element_count, sizeof(tDirSorterElement),
        directory_sorter);
    }
    for (i = 0; i < element_count; i++)
    { resource_collect_str(resource, sorter_base[i].value);
      memory_deallocate(sorter_base[i].key);
      memory_deallocate(sorter_base[i].value);
    }
  }

  resource_collect_str(resource, strEndBodyHtml);
  __dealloc(sorter_base);
  return(truE);
}

static one_caller void build_local_uri(tResourceRequest* request)
{ tUriData* uri_data = request->uri_data;
  const char* path = uri_data->path;
  char* spfbuf;
#if CONFIG_LOCALDIR > 1
  const char* query = uri_data->query;
  if (query != NULL)
  { my_spf(NULL, 0, &spfbuf, "local:%s%s%s", path, strQuerySort, query);
  }
  else
#endif
  { my_spf(NULL, 0, &spfbuf, "local:%s", path);
  }
  memory_deallocate(uri_data->uri); uri_data->uri = my_spf_use(spfbuf);
}

static one_caller void fetch_local(tResourceRequest* request)
{ tUriData* uri_data = request->uri_data;
  char* path = uri_data->path;
  struct stat statbuf;
  mode_t mode;

  /* Check whether the thing really exists and whether it's a regular file or
     a directory */

  if (my_stat(path, &statbuf) != 0)
  { /* The thing doesn't exist (any longer) or is otherwise inaccessible, but
       maybe we have its old contents in cache. */
    try_cache:
    if (resource_lookup(request) != NULL) return; /* found in cache, done */
    bad: resource_request_set_error(request, reFile); return;
  }

  mode = statbuf.st_mode;
  if (S_ISDIR(mode)) /* it's a directory */
  { const size_t len = strlen(path);
    if ( ( (len > 0) && (path[len - 1] != chDirsep) ) || (len == 0) )
    { uri_data->path = path = memory_reallocate(path, len + 1 + 1, mapString);
      strcat(path, strSlash); build_local_uri(request);
    }
    if (resource_lookup(request) != NULL) return; /* found in cache, done */
    if (!fetch_local_directory(request)) goto bad;
  }
  else if (S_ISREG(mode)) /* it's a regular file */
  { tResource* resource;
    tContentblock* content;
    void* buf;
    size_t size;
    tBoolean need_unmap SHUT_UP_COMPILER(falsE);
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
    if (request->flags & rrfDownload) /* can't "download" local files */
    { resource_request_set_error(request, reUri); return; }
#endif
    if (resource_lookup(request) != NULL) return; /* found in cache, done */
    switch (my_mmap_file_readonly(path, &buf, &size))
    { case 0: goto bad; /*@notreached@*/ break;
      case 1:
        buf = memory_allocate(1,mapOther); size = 0; need_unmap = falsE; break;
      case 2: need_unmap = truE; break;
    }
    content = contentblock_create(size, 0);
    content->used = size; content->data = buf;
    resource = resource_create(request, content, size, filesuffix2rck(path),
      rsComplete);
    resource->bytecount = size;
    if (need_unmap) resource->cantent->caf |= cafNeedUnmap;
  }
  else
  { /* The thing isn't a regular file or directory now, but maybe there was
       one at this position earlier, which we might have in cache. */
    goto try_cache;
  }
}


/** Local CGI resources */

#if OPTION_LOCAL_CGI

#if HAVE_SETENV
#define __my_setenv(var, val) (void) setenv(var, val, 1)
#define my_setenv(var, val) __my_setenv(var, val)
#elif HAVE_PUTENV
#define __my_setenv(var, val) (void) putenv(var "=" val)  /* fast and */
static void my_setenv(const char* var, const char* val)   /* slow variant */
{ char* spfbuf;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, "%s=%s", var, val);
  putenv(spfbuf);
  my_spf_cleanup(strbuf, spfbuf);
}
#else
#error "Cannot use local CGI - your libc has neither setenv() nor putenv(). Disable the compile-time configuration option OPTION_LOCAL_CGI!"
/* IMPLEMENTME? This is only executed in child processes after fork(), so there
   are no race conditions possible if we change environ manually... Or simply
   setup a minimal, CGI-specific environment and use execle(). */
#endif /* #if HAVE_SETENV/HAVE_PUTENV */

static one_caller uid_t my_geteuid(void)
{ static tBoolean done = falsE;
  static uid_t euid;
  if (!done) { euid = geteuid(); done = truE; } /* (calculate it only once) */
  return(euid);
}

static one_caller gid_t my_getegid(void)
{ static tBoolean done = falsE;
  static gid_t egid;
  if (!done) { egid = getegid(); done = truE; } /* (calculate it only once) */
  return(egid);
}

/* prototypes */
static void http_read(tResource*);
static void http_rpsd_prepare(tResource*);

static void local_cgi_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  if (conn->x == 0) /* reading the script's reply */
  { switch (ccek)
    { case ccekRead: http_read(resource); break;
      default: is_buggy: conn_bug(conn, ccek); break;
    }
  }
  else /* writing the post data */
  { switch (ccek)
    { case ccekWrite:
        if (conn_write_writedata(conn) != 1)
        { /* error or done - just stop writing, don't stop reading */
          conn_remove(&(resource->dconn));
        }
        break;
      default: goto is_buggy; /*@notreached@*/ break;
    }
  }
}

static one_caller tBoolean start_local_cgi(tResourceRequest* request)
/* starts a local CGI script if allowed/possible; returns want-it-back info */
{ const tConfigLocalCgi* clc = config.local_cgi;
  tResourceError error;
  tBoolean is_post;
  const char *path, *post SHUT_UP_COMPILER(NULL);
  size_t pathlen, postlen SHUT_UP_COMPILER(0);
  struct stat statbuf;
  mode_t mode;
  int fd_pair[2], fd_post[2];
  pid_t pid;

  /* First of all, check whether the configuration allows it */

  if (clc == NULL) goto forbidden; /* (optimizing for the most likely case) */
  path = request->uri_data->path; pathlen = strlen(path);
  while (clc != NULL)
  { const char* pattern = clc->path_pattern;
    size_t len;
    if ( (!strcmp(path, pattern)) ||
         ( ( (len = strlen(pattern)) > 0 ) && (pattern[len - 1] == '*') &&
           (pathlen >= len) && (!strncmp(path, pattern, len - 1)) ) )
    { if (clc->flags & clcfAllowed) goto allowed;
      break;
    }
    clc = clc->next;
  }
  forbidden:
  error = reConfigForbids;
  failed:
  resource_request_set_error(request, error);
  return(falsE);

  allowed:

  /* Check cache */

  if (resource_lookup(request) != NULL) return(falsE); /*found in cache, done*/

  /* Check whether the thing actually exists, is a regular file, executable */

  if (my_stat(path, &statbuf) != 0) { bad_file: error = reFile; goto failed; }
  mode = statbuf.st_mode;
  if (!S_ISREG(mode)) goto bad_file;
  /* simple pre-check without all the get...id() calls: */
  if (!(mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
  { cant_exec: error = reExec; goto failed; }
  /* more thorough check: */
  if (! (( (mode & S_IXOTH) ||
         ( (mode & S_IXUSR) && (statbuf.st_uid == my_geteuid()) ) ||
         ( (mode & S_IXGRP) && (statbuf.st_gid == my_getegid()) ) )) )
    goto cant_exec;

  /* Execute the script */

  if (my_pipe(fd_pair) != 0) { pipe_failed: error = rePipe; goto failed; }
  if (!fd_is_observable(fd_pair[0]))
  { tmofd: my_close_pipe(fd_pair[0]); my_close_pipe(fd_pair[1]);
    error = reTmofd; goto failed;
  }
  is_post = cond2boolean(request->flags & rrfPost);
  if (is_post)
  { if (my_pipe(fd_post) != 0)
    { my_close_pipe(fd_pair[0]); my_close_pipe(fd_pair[1]); goto pipe_failed; }
    if (!fd_is_observable(fd_post[1]))
    { my_close_pipe(fd_post[0]); my_close_pipe(fd_post[1]); goto tmofd; }
    post = null2empty(request->uri_data->post);
    postlen = strlen(post);
  }

  pid = fork();
  if (pid < 0) /* failed */
  { error = reFork;
    my_close_pipe(fd_pair[0]); my_close_pipe(fd_pair[1]);
    if (is_post) { my_close_pipe(fd_post[0]); my_close_pipe(fd_post[1]); }
    goto failed;
  }
  else if (pid == 0) /* child process */
  { char *spfbuf, *spfbuf2;
    const char *query, *temp;
    reset_signals();
    query = request->uri_data->query;
    my_close_unregistried(0); my_close_pipe(fd_pair[0]);
    if (!may_use_fd2()) my_close_unregistried(2);
    if (fd_pair[1] != 1)
    { if (!my_dup2(fd_pair[1], 1))
      { dup2_failed:
        my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("retawq: local CGI error: can't setup file descriptors (error #%d, %s)\n"), errno, my_strerror(errno));
        if (may_use_fd2()) my_write_str(fd_stderr, spfbuf);
        debugmsg(spfbuf);
        goto exit_child;
      }
      my_close_pipe(fd_pair[1]);
    }
    if (is_post)
    { my_close_pipe(fd_post[1]);
      if (fd_post[0] != 0)
      { if (!my_dup2(fd_post[0], 0)) goto dup2_failed;
        my_close_pipe(fd_post[0]);
      }
      sprint_safe(strbuf, strPercd, postlen);
      my_setenv("CONTENT_LENGTH", strbuf);
      my_setenv("CONTENT_TYPE", strAxwfu);
    }
    __my_setenv("GATEWAY_INTERFACE", "CGI/1.1");
      my_setenv("HTTP_USER_AGENT", config.user_agent);
    if ( (query != NULL) && (*query != '\0') )
      my_setenv("QUERY_STRING", query);
      my_setenv("REMOTE_HOST", strLocalhost);
    __my_setenv("REMOTE_ADDR", "127.0.0.1");
      my_setenv("REQUEST_METHOD", is_post ? strPost : strGet);
      my_setenv("SCRIPT_NAME", path);
      my_setenv("SERVER_NAME", strLocalhost);
    __my_setenv("SERVER_PORT", "80"); /* (just a dummy value) */
    __my_setenv("SERVER_PROTOCOL", "HTTP/1.0"); /* CHECKME! */
      my_setenv("SERVER_SOFTWARE", strSoftwareId);

    temp = my_strrchr(path, chDirsep);
    if (temp != NULL) /* "should" be true */
    { size_t len = temp - path + 1, size = len + 1;
      char* ptr = ( (size <= STRBUF_SIZE) ? strbuf :
        __memory_allocate(size, mapString) );
      my_memcpy(ptr, path, len); ptr[len] = '\0';
      (void) chdir(ptr);
      if (ptr != strbuf) memory_deallocate(ptr);
    }
    (void) execl(path, path, NULL);

    /* If we get here, execl() failed, so we send a little error text to the
       resource handler and exit the child process: */
    my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      _("Local CGI error: can't execute \"%s\" (error #%d, %s)%s"), path,errno,
      my_strerror(errno), ( (errno == ENOENT) ? _("\n\n(Note: an error like \"file not found\" might mean \"script interpreter not found\" here.)") : strEmpty ));
    my_spf(strbuf2, STRBUF_SIZE, &spfbuf2,
     "Status: 500\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
     strlen(spfbuf), spfbuf);
    my_write_str_unregistried(1, spfbuf2);
    exit_child:
    _exit(1);
  }
  else /* parent process */
  { tResource* resource;
    tConnection* conn;
    make_fd_cloexec(fd_pair[0]); my_close_pipe(fd_pair[1]);
    resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
      rckUnknown, rsReading);
    http_rpsd_prepare(resource);
    conn = resource->cconn = conn_create(fd_pair[0], local_cgi_callback,
      resource, truE, NULL, resource->protocol);
    conn->flags |= cnfConnected; conn_set_read(conn);
    if (is_post)
    { my_close_pipe(fd_post[0]);
      if (postlen <= 0) { my_close_pipe(fd_post[1]); } /* nothing to post */
      else
      { make_fd_cloexec(fd_post[1]);
        (void) make_fd_nonblocking(fd_post[1]);
          /* (just trying; we won't let it all fail if only this fails...) */
        conn = resource->dconn = conn_create(fd_post[1], local_cgi_callback,
          resource, truE, NULL, resource->protocol);
        conn->flags |= cnfConnected; conn->x = 1; conn_set_write(conn);
        conn_set_writestr(conn, my_strdup(post));
      }
      resource->flags |= rfPost;
    }
  }
  return(truE);
}

#endif /* #if OPTION_LOCAL_CGI */


/** Execution of external programs: shell */

#if OPTION_EXECEXT & EXECEXT_SHELL

static one_caller void execext_shell_read(tConnection* conn)
{ tResource* resource = (tResource*) (conn->data); /* always != NULL here */
  char* dest;
  size_t size;
  int err;
  resource_provide_room(resource, &dest, &size, resource->bytecount, 2);
  err = my_read_pipe(conn->fd, dest, size);
  if (err <= 0) /* error or EOF */
  { conn_remove(&(resource->cconn));
    if (resource->dconn != NULL) conn_remove(&(resource->dconn));
    if (err == 0) resource->state = rsComplete;
    else resource_set_error(resource, reConnect); /* CHECKME: re...? */
    push_to_main_res(resource, 0);
  }
  else
  { resource_record(resource, dest, err);
#if CONFIG_CUSTOM_CONN
    if (program_mode == pmConsole)
    { /* CHECKME: do this without resource_provide_room()/resource_record()?
         Seems superfluous... */
      __custom_conn_print(resource, dest, err, ccpkNetresp);
    }
#endif
    push_to_main_res(resource, 1);
  }
}

static void execext_shell_callback(tConnection* conn, tConnCbEventKind ccek)
{ if (conn->x == 0) /* reading the command's output */
  { switch (ccek)
    { case ccekRead: execext_shell_read(conn); break;
      default: is_buggy: conn_bug(conn, ccek); break;
    }
  }
  else /* writing the command's input */
  { switch (ccek)
    { case ccekWrite:
        if (conn_write_writedata(conn) != 1)
        { /* error or done - just stop writing, don't stop reading */
          tResource* resource = conn2resource(conn);
          if (resource != NULL) conn_remove(&(resource->dconn));
          else conn_remove(&conn);
        }
        break;
      default: goto is_buggy; /*@notreached@*/ break;
    }
  }
}

void resource_start_execext_shell(tExecextShellData* data)
{ tResourceRequest* request = data->request;
  const char *command = data->command, *writedata = data->writedata;
  tExecextShellFlags esf = data->esf;
  tResourceError error;
  const tBoolean do_read = cond2boolean(esf & (esfReadStdout | esfReadStderr)),
    do_write = cond2boolean(writedata != NULL);
  int fd_read[2], fd_write[2]; /* "read"/"write" from parent's point of view */
  unsigned char _cleanup = 0; /* what to cleanup on failure */
  pid_t pid;

  if (do_write)
  { if (my_pipe(fd_write) != 0)
    { pipe_failed: error = rePipe;
      failed:
      if (request != NULL)
      { resource_request_set_error(request, error);
        push_to_main_req(request, 0);
      }
      if (_cleanup & 1)
      { my_close_pipe(fd_write[0]); my_close_pipe(fd_write[1]); }
      if (_cleanup & 2)
      { my_close_pipe(fd_read[0]); my_close_pipe(fd_read[1]); }
      return;
    }
    _cleanup |= 1;
    if (!fd_is_observable(fd_write[1])) { tmofd: error = reTmofd; goto failed;}
  }
  if (do_read)
  { if (my_pipe(fd_read) != 0) goto pipe_failed;
    _cleanup |= 2;
    if (!fd_is_observable(fd_read[0])) goto tmofd;
  }

  pid = fork();
  if (pid < 0) { error = reFork; goto failed; }
  else if (pid == 0) /* child process */
  { const char *path, *errstr;
    char *spfbuf, *spfbuf2, *param[20];
    unsigned char count;
    int err;
    reset_signals(); count = 0;
    if (config.flags & cfExecextShellCustom)
    { const tExecextParam *list = config.execext_shell;
      while (list != NULL) { param[count++] = list->param; list = list->next; }
    }
    else
    { param[count++] = unconstify("/bin/sh");
      param[count++] = unconstify("-c");
    }
    param[count++] = unconstify(command); param[count] = NULL;
    if (do_read) my_close_pipe(fd_read[0]);
    if (esf & esfReadStdout)
    { if (fd_read[1] != 1)
      { if (!my_dup2(fd_read[1], 1))
        { dup2_failed: err = errno; errstr = _("can't setup file descriptors");
          goto child_error;
        }
      }
    }
    else my_close_unregistried(1);
    if (esf & esfReadStderr)
    { if ( (fd_read[1] != 2) && (!my_dup2(fd_read[1], 2)) ) goto dup2_failed; }
    else my_close_unregistried(2);
    if (do_write)
    { my_close_pipe(fd_write[1]);
      if ( (fd_write[0] != 0) && (!my_dup2(fd_write[0], 0)) ) goto dup2_failed;
    }
    else my_close_unregistried(0);
    path = param[0];
    (void) execv(path, param);

    /* If we get here, execv() failed, so we send a little error text to the
       resource handler and exit the child process: */
    err = errno;
    my_spf(strbuf2, STRBUF_SIZE, &spfbuf2, _("can't execute \"%s\""), path);
    errstr = spfbuf2;
    child_error:
    my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      _("retawq: execext-shell error: %s (error #%d, %s)"), errstr, err,
      my_strerror(err));
    if (esf & esfReadStderr) my_write_str(fd_read[1], spfbuf);
    else if (may_use_fd2()) my_write_str_unregistried(2, spfbuf); /* CHECKME!*/
    debugmsg(spfbuf);
    _exit(1);
  }
  else /* parent process */
  { tResource* resource = NULL;
    tConnection *rconn = NULL, *wconn = NULL;
    if (do_read)
    { my_close_pipe(fd_read[1]); make_fd_cloexec(fd_read[0]);
      (void) make_fd_nonblocking(fd_read[0]); /* (just trying) */
      resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH,
        ( (esf & esfEnforceHtml) ? rckHtml : rckUnknown ), rsReading);
      rconn = conn_create(fd_read[0], execext_shell_callback, resource, truE,
        NULL, rpExecextShell);
      rconn->flags |= cnfConnected; conn_set_read(rconn);
    }
    if (do_write)
    { my_close_pipe(fd_write[0]); make_fd_cloexec(fd_write[1]);
      (void) make_fd_nonblocking(fd_write[1]); /* (just trying) */
      wconn = conn_create(fd_write[1], execext_shell_callback, resource,
        cond2boolean(resource != NULL), NULL, rpExecextShell);
      wconn->flags |= cnfConnected; wconn->x = 1; conn_set_write(wconn);
      conn_set_writedata(wconn, writedata, data->writedata_size);
      data->writedata = NULL; /* detach */
#if CONFIG_DEBUG
      wconn->flags |= cnfDebugNodata;
#endif
    }
    if (resource != NULL) { resource->cconn = rconn; resource->dconn = wconn; }
  }
}

#endif /* #if OPTION_EXECEXT & EXECEXT_SHELL */


/** Non-local resources: HTTP */

#if CONFIG_HTTP & HTTP_11
my_enum1 enum
{ cpsUnknown = 0, cpsData = 1, cpsSize = 2, cpsCrlf1 = 3, cpsSkipSizeline = 4,
  cpsCr2 = 5, cpsCrlf2 = 6, cpsSkipTrailerA = 7, cpsSkipTrailerB = 8,
  cpsSkipTrailerC = 9, cpsSkipTrailerD = 10
} my_enum2(unsigned char) tChunkParsingState; /* for HTTP/1.1 chunk encoding */
#endif

my_enum1 enum
{ hrfNone = 0
#if CONFIG_HTTP & HTTP_11
  , hrfChunked = 0x01
#endif
#if CONFIG_HTTP & HTTP_AUTH_ANY
  , hrfUseNewAuth = 0x02
#endif
} my_enum2(unsigned char) tHttpRpsdFlags;

typedef struct
{ tRpsdRemover remover;
  char* header;
#if CONFIG_HTTP & HTTP_AUTH_ANY
  const char* challenge; /* from an HTTP response header */
  const char* new_auth; /* for the subsequent HTTP request header */
#endif
  size_t usable, used; /* for the header */
#if CONFIG_HTTP & HTTP_11
  size_t chunksize_left;
  tChunkParsingState cps;
#endif
  tHttpRpsdFlags flags;
} tHttpRpsd;

static void http_rpsd_remove(tRpsdGeneric* _rpsd)
{ tHttpRpsd* rpsd = (tHttpRpsd*) _rpsd;
  __dealloc(rpsd->header);
#if CONFIG_HTTP & HTTP_AUTH_ANY
  __dealloc(rpsd->challenge); __dealloc(rpsd->new_auth);
#endif
}

static void http_rpsd_prepare(tResource* resource)
{ resource->rpsd = memory_allocate(sizeof(tHttpRpsd), mapRpsd);
  resource->rpsd->remover = http_rpsd_remove;
}

#define __IS_HTTP_WHITESPACE(ch) ((ch) == ' ') /* (enough after sanitizing) */
#define IS_HTTP_WHITESPACE(ch) ( ((ch) == ' ') || ((ch) == '\t') )

static void skip_http_whitespace(const char** _ptr)
/* skips an HTTP header whitespace sequence at the current position */
{ const char* ptr = *_ptr;
  while (__IS_HTTP_WHITESPACE(*ptr)) ptr++;
  *_ptr = ptr;
}

#define http_close(res) conn_remove(&((res)->cconn))

#if CONFIG_HTTP & HTTP_11

#define http_cl(res) ((tHttpRpsd*)((res)->rpsd))->chunksize_left
#define http_cps(res) ((tHttpRpsd*)((res)->rpsd))->cps

static void parse_http_chunks(tResource* resource, const char* src,
  size_t count)
/* Welcome to the World's Worst Chunk-O-Mania! (Now you know the _real_
   meaning of "WWW" and ".com":-) A pseudo-code algorithm for this is available
   in RFC2616, 19.4.6, but of course we do it differently... */
{ size_t chunksize = http_cl(resource), usable_left, usedsize, add;
  char *deststart, *dest SHUT_UP_COMPILER(NULL);
  tChunkParsingState cps = http_cps(resource);
  tBoolean do_push, want_it_back;

  usable_left = usedsize = 0; do_push = falsE; want_it_back = truE;
  while (count-- > 0)
  { const char ch = *src++;
    switch (cps)
    { case cpsData: /* the most likely case */
        if (chunksize > 0) /* it's a "real data" character, store it */
        { if (usable_left <= 0) /* must get more room */
          { if (usedsize > 0)
            { resource_record(resource, deststart, usedsize); usedsize = 0; }
            resource_provide_room(resource, &deststart, &usable_left,
              resource->bytecount, 2);
            dest = deststart;
          }
          *dest++ = ch; usedsize++; usable_left--; chunksize--; do_push = truE;
        }
        else if (ch == '\r') cps = cpsCrlf2;
        else
        { bad_response:
          resource_set_error(resource, reResponse);
          goto stopnpush;
        }
        break;
      case cpsSize: /* parse chunk-size value */
        if (my_isdigit(ch))
        { add = (size_t) (ch - '0');
          calc:
          chunksize = 16 * chunksize + add;
          if (chunksize > 100 * 1024 * 1024) goto bad_response;
        }
        else if ( (ch >= 'a') && (ch <= 'f') )
        { add = (size_t) (ch - 'a' + 10); goto calc; }
        else if ( (ch >= 'A') && (ch <= 'F') )
        { add = (size_t) (ch - 'A' + 10); goto calc; }
        else if (ch == '\r') cps = cpsCrlf1;
        else
        { /* The server sent optional additional information (called
             "chunk-extension" in RFC2616, 3.6.1) we don't care about. */
          cps = cpsSkipSizeline;
        }
        break;
      case cpsCrlf1: /* reached end of chunk-size line */
        if (ch == '\n')
        { if (chunksize == 0) cps = cpsSkipTrailerC;
          else cps = cpsData;
#if CONFIG_DEBUG
          sprint_safe(debugstrbuf, "expecting HTTP chunk of size %d\n",
            chunksize);
          debugmsg(debugstrbuf);
#endif
        }
        else goto bad_response;
        break;
      case cpsSkipSizeline:
        if (ch == '\r') cps = cpsCrlf1;
        /* "else": skip further characters */
        break;
      case cpsCr2: /* reached end of chunk */
        if (ch == '\r') cps = cpsCrlf2;
        else goto bad_response;
        break;
      case cpsCrlf2: /* reached end of chunk */
        if (ch == '\n') cps = cpsSize;
        else goto bad_response;
        break;
      case cpsSkipTrailerA:
        if (ch == '\r') cps = cpsSkipTrailerB;
        break;
      case cpsSkipTrailerB:
        if (ch == '\n') cps = cpsSkipTrailerC;
        else cps = cpsSkipTrailerA;
        break;
      case cpsSkipTrailerC:
        if (ch == '\r') cps = cpsSkipTrailerD;
        else cps = cpsSkipTrailerA;
        break;
      case cpsSkipTrailerD:
        if (ch == '\n') /* finished receiving this resource */
        { debugmsg("finished chunked document\n");
          resource->state = rsComplete;
          stopnpush: http_close(resource);
#if OPTION_LOCAL_CGI
          if (resource->dconn != NULL) conn_remove(&(resource->dconn));
#endif
          do_push = truE; want_it_back = falsE; goto out;
        }
        else cps = cpsSkipTrailerA;
        break;
    }
  }
  out:
  if (usedsize > 0) resource_record(resource, deststart, usedsize);
  http_cps(resource) = cps; http_cl(resource) = chunksize;
  if (do_push) push_to_main_res(resource, boolean2bool(want_it_back));
}

#endif /* #if CONFIG_HTTP & HTTP_11 */

static one_caller tMbsIndex do_lookup_hhps(const char* str)
{ my_binary_search(0, MAX_HHPS, strneqcase3(str, strHhps[idx], hhpslen[idx]),
    return(idx))
  /* (case-insensitivity of header field names: RFC2616, 4.2) */
}

static one_caller tHttpHeaderParsingState lookup_hhps(const char** _ptr)
{ const char* ptr = *_ptr;
  tMbsIndex idx = do_lookup_hhps(ptr);
  if (idx < 0) idx = 0; /* hhpsDontCare */
  else if (idx > 0) { *_ptr += hhpslen[idx]; skip_http_whitespace(_ptr); }
  return((tHttpHeaderParsingState) idx);
}

typedef struct
{ char *current, *n, *r;
} tHttpHeaderContext;

static void http_header_cleanup(tHttpHeaderContext* context)
{ if (context->n != NULL) { *(context->n) = '\n'; context->n = NULL; }
  if (context->r != NULL) { *(context->r) = '\r'; context->r = NULL; }
}

static const char* http_header_next(tHttpHeaderContext* context)
/* returns a pointer to the next header line, NULL for ending; RFC2616, 19.3 */
{ char *retval = NULL, *current = context->current, *n;
  http_header_cleanup(context); /* cleanup for the previous step */
  if ( (current == NULL) || (*current == '\0') ) goto out;
  loop:
  n = my_strchr(current, '\n');
  if (n != NULL)
  { char *r, *minpos;
    if ( (n > current) && (*(n - 1) == '\r') ) r = minpos = n - 1;
    else { r = NULL; minpos = n; }
    if (minpos <= current) goto out; /* empty header line, end of header */
    if (IS_HTTP_WHITESPACE(*(n + 1))) /* multi-line field (RFC2616, 4.2) */
    { *n = ' '; if (r != NULL) *r = ' ';
      goto loop;
    }
    context->n = n; *n = '\0';
    if (r != NULL) { context->r = r; *r = '\0'; }
  }
  retval = current; context->current = ( (n != NULL) ? (n + 1) : NULL );
  { unsigned char u, *x = (unsigned char*) retval;
    while ( (u = *x) != '\0' )
    { if (u == 9) *x = ' '; /* replace tab with space, to simplify */
      else if (is_control_char(u)) *x = 'X'; /* found forbidden character */
      x++;
    }
  }
  out:
  return(retval);
}

static one_caller tBoolean finish_http_header(tResource* resource,char* header)
/* returns whether it worked */
{ tCantent* cantent = resource->cantent;
  const char* temp;
  int http_major, http_minor, http_status, value;
  tHttpHeaderContext header_context;
  tHttpHeaderParsingState hhps;
  tBoolean cannot_http11 = truE, ignore_contentlength = falsE,
    can_keep_alive = falsE;
#if CONFIG_HTTP & HTTP_11
  tBoolean is_chunked = falsE;
#endif
#if OPTION_COOKIES
  const tBoolean accept_cookies = cookie_accept_any(resource);
#endif
#if OPTION_LOCAL_CGI
  const tBoolean is_local_cgi = cond2boolean(resource->protocol == rpLocalCgi);
#endif

  /* Parse the HTTP header (RFC2616); at first we check the HTTP version
     (RFC2145) and status code. */

  my_memclr_var(header_context); header_context.current = header;
#if OPTION_LOCAL_CGI
  if (is_local_cgi)
  { resource->server_status_code = 200; /* assume the script worked nicely */
    goto header_loop; /* local CGI scripts don't send an "HTTP/.." line */
  }
#endif
  temp = http_header_next(&header_context);
  if ( (temp == NULL) || (!strneqcase(temp, "http/", 5)) )
  { bad: resource_set_error(resource, reResponse);
    http_header_cleanup(&header_context); return(falsE);
  }
  my_atoi(temp + 5, &http_major, &temp, 99);
  if (http_major > 1) goto bad; /* beyond our capabilities; blame server :-) */
  if (*temp != '.') goto bad;
  my_atoi(temp + 1, &http_minor, &temp, 99); skip_http_whitespace(&temp);
  my_atoi(temp, &http_status, &temp, 999);
  if ( (http_status < 100) || (http_status > 599) ) goto bad;
  resource->server_status_code = (tServerStatusCode) http_status;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "HTTP header: %d,%d,%d\n", http_major, http_minor,
    http_status);
  debugmsg(debugstrbuf);
#endif

  if ( (http_major < 1) || ( (http_major == 1) && (http_minor < 1) ) )
  { tSockaddrPortProtInfo* sppi = conn2sppi(resource->cconn, truE);
    if (sppi != NULL) sppi->sppif |= sppifCannotHttp11;
    cannot_http11 = truE;
  }
  else
  { cannot_http11 = falsE;
    can_keep_alive = truE; /* (RFC2616, 8.1.2.1 par. 4) */
  }

  header_loop:
  temp = http_header_next(&header_context);
  if (temp == NULL) goto finish_header; /* reached end of header */
  hhps = lookup_hhps(&temp);

  switch (hhps)
  { case hhpsConnection:
      if (strneqcase(temp, "keep-alive", 10)) can_keep_alive = truE;
      else if (strneqcase(temp, strClose, 5)) can_keep_alive = falsE;
      break;
    case hhpsContentLength:
      if (!ignore_contentlength)
      { my_atoi(temp, &value, NULL, 1000 * 1024 * 1024);
        resource->nominal_contentlength = value;
      }
      break;
    case hhpsContentType:
      { /* (case-insensitivity of media types: RFC2616, 3.7) */
        tResourceContentKind kind;
        if (strneqcase(temp, "text/html", 9)) kind = rckHtml;
        else if (strneqcase(temp, "text/plain", 10)) kind = rckPlainText;
        else kind = rckUnknown;
        cantent->kind = kind;
      }
      break;
    case hhpsLocation:
      __dealloc(cantent->redirection);
      cantent->redirection = ( (*temp != '\0') ? my_strdup(temp) : NULL );
      break;
    case hhpsServer: conn_set_software_id(resource->cconn, temp); break;
#if OPTION_COOKIES
    case hhpsSetCookie: case hhpsSetCookie2:
      if (accept_cookies)
        cookie_handle_text(resource, temp, cond2boolean(hhps==hhpsSetCookie2));
      break;
#endif
#if OPTION_LOCAL_CGI
    case hhpsStatus:
      if (is_local_cgi)
      { int i;
        my_atoi(temp, &i, &temp, 999);
        if ( (i < 100) || (i > 599) ) goto bad;
        resource->server_status_code = (tServerStatusCode) i;
      }
      break;
#endif
    case hhpsTransferEncoding:
      ignore_contentlength = truE; /* RFC2616, 4.4(.3|end) plus HTTP errata */
      if ( (!cannot_http11) && (strneqcase(temp, "chunked", 7)) )
      {
#if CONFIG_HTTP & HTTP_11
        is_chunked = truE;
#else
        goto bad; /* server bug (we sent an HTTP/1.0 request) */
#endif
      }
      break;
#if CONFIG_HTTP & HTTP_AUTH_ANY
    case hhpsWwwAuthenticate:
      { tHttpRpsd* rpsd = (tHttpRpsd*) resource->rpsd;
        __dealloc(rpsd->challenge);
        rpsd->challenge = ( (*temp != '\0') ? my_strdup(temp) : NULL );
        debugmsg("HTTP challenge: *"); debugmsg(null2empty(rpsd->challenge));
        debugmsg("*\n");
      }
      break;
#endif
  }
  goto header_loop;

  finish_header:
  if (ignore_contentlength)
    resource->nominal_contentlength = UNKNOWN_CONTENTLENGTH;
  if (resource->cantent->kind == rckUnknown)
    resource->cantent->kind = filesuffix2rck(resource->uri_data->path);
#if CONFIG_HTTP & HTTP_11
  if (is_chunked)
  { ((tHttpRpsd*)(resource->rpsd))->flags |= hrfChunked;
    http_cl(resource) = 0; http_cps(resource) = cpsSize;
  }
#endif
  if ( (resource->nominal_contentlength == UNKNOWN_CONTENTLENGTH) ||
       (!can_keep_alive) )
    resource->cconn->flags |= cnfDontReuse;
  debugmsg("finished HTTP header:\n"); debugmsg(header); debugmsg(strNewline);
  return(truE);
}

#if CONFIG_HTTP & (HTTP_AUTH_BASIC | HTTP_PROXYAUTH)
static const char* http_build_basic_authorization(const char* prefix,
  const char* username, const char* password)
{ char *spfbuf, *spfbuf_enc;
  const char* enc;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf_enc, strPercsColonPercs, username,
    password);
  enc = base64_encode(spfbuf_enc); my_spf_cleanup(strbuf, spfbuf_enc);
  my_spf(NULL, 0, &spfbuf, "%sAuthorization: Basic %s\r\n", prefix, enc);
  memory_deallocate(enc);
  return(my_spf_use(spfbuf));
}
#endif

static one_caller const char* http_calc_version(const tResource* resource)
{ const char* version = NULL;
  const tConfigProtocolVersion* v = config.http_version;

  if (v != NULL)
  { const char* hostname = resource2textual_host(resource)->hostname;
    tPortnumber resport = resource->uri_data->portnumber;
    while (v != NULL)
    { tPortnumber hp = v->hosts_portnumber;
      if ( (hp == 0) || (hp == resport) )
      { const char* pattern = v->hosts_pattern;
        if ( (pattern != NULL) && (my_pattern_matcher(pattern, hostname)) )
        { version = v->protstr; break; } /* (might be NULL, no problem) */
      }
      v = v->next;
    }
  }

  if (version == NULL)
  {
#if (0) && (CONFIG_HTTP & HTTP_11)
    /* This code might be re-enabled at some point in the future - when an
       acceptable number of HTTP server implementations can handle HTTP/1.1
       correctly... */
    const tSockaddrPortProtInfo* sppi = conn2sppi(resource->cconn, falsE);
    const tBoolean cannot_http11 =
      cond2boolean( (sppi != NULL) && (sppi->sppif & sppifCannotHttp11) );
    version = (cannot_http11 ? str1dot0 : str1dot1);
#else
    version = str1dot0;
#endif
  }
  return(version);
}

static one_caller void http_setup_reqstr(tResource* resource)
{ const tUriData* uri_data = resource->uri_data;
  const tConfigProxy* proxy = resource->proxy;
  tResourceFlags rf = resource->flags;
  const tPortnumber portnumber = ntohs(resource->uri_data->portnumber);
    /* (in host byte order!) */
  const tBoolean use_post = cond2boolean(rf & rfPost);
  const char *uri = uri_data->uri, *path = uri_data->path, *spfbuf_uri,
    *query = uri_data->query, *post = (use_post ? null2empty(uri_data->post) :
    strEmpty), *hostname = resource2textual_host(resource)->hostname,
    *cookies = cookie_collect(resource), *spfbuf_proxyauth = strEmpty,
    *http_version = http_calc_version(resource);
  const tBoolean use_query = cond2boolean((query!=NULL) && (*query != '\0')),
    use_proxy = cond2boolean((proxy != NULL) && (proxy->proxy_hostname!=NULL));
  char portbuf[100], *spfbuf;

  if (use_post)
  { sprint_safe(strbuf2, "Content-Type: %s\r\nContent-Length: %d\r\n",
      strAxwfu, strlen(post));
    resource->flags |= rfPost; rf = resource->flags;
  }

#if CONFIG_HTTP & HTTP_PROXYAUTH
  if (use_proxy)
  { const char *auth_user = proxy->auth_user, *auth_pass = proxy->auth_pass;
    if ( (auth_user != NULL ) && (auth_pass != NULL ) )
    { spfbuf_proxyauth = http_build_basic_authorization("Proxy-", auth_user,
        auth_pass);
    }
  }
#endif

  if (use_proxy) spfbuf_uri = uri; /* RFC2616, 5.1.2 */
  else if (!use_query) spfbuf_uri = path; /* most likely case */
  else /* RFC2616 (3.2.2) plus HTTP/1.1 erratum "URI includes query" */
  { my_spf(NULL, 0, &spfbuf, "%s?%s", path, query);
    spfbuf_uri = my_spf_use(spfbuf);
  }

  /* The following might seem to contradict RFC2616 (3.2.2, 3.2.3, 14.23,
     14.30), but some servers using Apache httpd would try to cause infinite
     redirection to the original URI if we used the portnumber without such a
     condition. (For example, the word "equivalent" in 3.2.3 means "basically
     the same" to me, and 14.30 says that a server may only redirect to
     something "other" than the Request-URI, but here "equivalent" seems to
     imply "other" instead of "basically the same".) */
  if (!(uri_data->udf & udfGotExplicitPortnumber)) *portbuf = '\0';
  else sprint_safe(portbuf, ":%d", portnumber);

  my_spf(NULL, 0, &spfbuf,
/*A*/ "%s %s HTTP/%s\r\n"
/*B*/ "Host: %s%s\r\n"
/*C*/ "User-Agent: %s\r\n"
/*D*/ "%s" /* proxy authentication if configured */
/*E*/ "%s" /* possibly authentication (non-proxy) */
/*F*/ "%s" /* cache-control stuff if desired */
/*G*/ "%s" /* cookies if any */
/*H*/ "%s" /* content-type/-length for POST method */
      "Accept: */*\r\n"
/*I*/ "Accept-Language: %s\r\n"
      "Accept-Charset: iso-8859-1\r\n"
/*J*/ "\r\n%s", /* end of header; POST data if any */

/*A*/ (use_post ? strPost : strGet), spfbuf_uri, http_version,
/*B*/ hostname, portbuf,
/*C*/ config.user_agent,
/*D*/ spfbuf_proxyauth,
/*E*/ strEmpty,
/*F*/ ( (rf & rfIsEnforced) ? /* (RFC2616, 14.32 and 14.9) */
       "Pragma: no-cache\r\nCache-Control: no-cache\r\n" : strEmpty ),
/*G*/ cookies,
/*H*/ (use_post ? strbuf2 : strEmpty),
/*I*/ config.languages,
/*J*/ post);

#if OPTION_TLS
  if ( (resource->protocol == rpHttps) && (proxy != NULL) )
  { char* spfbuf2;
    my_spf(NULL, 0, &spfbuf2,
      "CONNECT %s:%d HTTP/%s\r\nHost: %s:%d\r\n%s\r\n%s", /* RFC2817, 5.2 */
      hostname, portnumber, http_version, hostname, portnumber,
      spfbuf_proxyauth, spfbuf);
    memory_deallocate(spfbuf);
    spfbuf = my_spf_use(spfbuf2);
  }
#endif

  conn_set_writestr(resource->cconn, my_spf_use(spfbuf));
  my_spf_cleanup(strEmpty, spfbuf_proxyauth);
  if ( (spfbuf_uri != uri) && (spfbuf_uri != path) )
    memory_deallocate(spfbuf_uri);
  cookie_collect_cleanup(cookies);
}

#if CONFIG_HTTP & HTTP_AUTH_ANY

static one_caller tBoolean http_might_retry_with_auth(const tResource*
  resource)
{ tBoolean retval = falsE;
  tHttpRpsd* rpsd = (tHttpRpsd*)(resource->rpsd);
  const tUriData* uri_data;
  const char *c = rpsd->challenge, *u, *p;
  if (c == NULL) goto out;
  uri_data = resource->uri_data;
  u = uri_data->username; p = uri_data->password;
#if CONFIG_HTTP & HTTP_AUTH_BASIC
  if ( (strneqcase(c, "basic", 5)) && (__IS_HTTP_WHITESPACE(c[5])) )
  { if ( (u != NULL) && (p != NULL) )
    { rpsd->new_auth = http_build_basic_authorization(strEmpty, u, p);
      retval = truE;
    }
    goto out;
  }
#endif
#if CONFIG_HTTP & HTTP_AUTH_DIGEST
  if ( (strneqcase(c, "digest", 6)) && (__IS_HTTP_WHITESPACE(c[6])) )
  { /* IMPLEMENTME! */
    goto out;
  }
#endif
  out:
  return(retval);
}

#endif /* #if CONFIG_HTTP & HTTP_AUTH_ANY */

#if CONFIG_DEBUG
static void http_read_debug(const tResource* resource, const char* what,
  int err, const char* buf)
{ tBoolean is_chunked;
#if CONFIG_HTTP & HTTP_11
  is_chunked = cond2boolean(((tHttpRpsd*)(resource->rpsd))->flags &hrfChunked);
#else
  is_chunked = falsE;
#endif
  sprint_safe(debugstrbuf, "http_read(%p, %s): %d,%d,%d\n", resource, what,
    err, errno, is_chunked);
  debugmsg(debugstrbuf);
  if (err > 0)
  { __debugmsg(buf, err);
    if (buf[err - 1] != '\n') debugmsg(strNewline);
  }
}
#else
#define http_read_debug(a, b, c, d) do { } while (0)
#endif

static void http_read(tResource* resource)
/* This function is used for rpHttp, rpHttps and rpLocalCgi. */
{ tConnection* conn = resource->cconn;
  int err;
  tBoolean want_it_back = truE;
#if CONFIG_HTTP & HTTP_11
  tBoolean is_chunked;
#endif
  tServerStatusCode ssc;
  tTlHeaderState ths;
  char* dest;
  size_t size, ncl;

  ths = resource->tlheaderstate;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "http_read(%p): %d\n", resource, ths);
  debugmsg(debugstrbuf);
#endif
  if (ths >= 0) /* reading the header */
  { tHttpRpsd* rpsd = (tHttpRpsd*) resource->rpsd;
    char *header = rpsd->header, *temp;
    size_t usable = rpsd->usable, used = rpsd->used, count;
    if (usable <= used)
    { if (usable >= HIGHEST_OCBS / 2) /* big header - attack? */
      { resource_set_error(resource, reResponse); goto do_stop_reading; }
      else if (usable <= 0) usable = 1024;
      else usable <<= 1;
      rpsd->usable = usable;
      rpsd->header = header = memory_reallocate(header, usable, mapOther);
    }
    temp = header + used;
    conn_read(err, conn, temp, usable - used, return);
    http_read_debug(resource, "header", err, temp);
    if (err <= 0)
    { stop_reading:
      if (err == 0) resource->state = rsComplete;
      else resource_set_error(resource, reServerClosed);
      do_stop_reading: want_it_back = falsE; http_close(resource);
#if OPTION_LOCAL_CGI
      if (resource->dconn != NULL) conn_remove(&(resource->dconn));
#endif
      goto finish;
    }
    rpsd->used += err; count = err;
    while (count-- > 0)
    { const char ch = *temp++;
      switch (ths)
      { case 0: if (ch == '\r') { ths = 1; } break;
        case 1:
          if (ch == '\n') ths = 2;
          else if (ch == '\r') { /* ths = 1; */ }
          else ths = 0;
          break;
        case 2: ths = ( (ch == '\r') ? 3 : 0 ); break;
        case 3:
          if (ch == '\n') /* found the end of the HTTP header */
          { *(temp - 2) = '\0'; /* terminate the tough text trickily :-) */
            if (!finish_http_header(resource, header)) goto do_stop_reading;
            ssc = resource->server_status_code;
            if ( (ssc >= 100) && (ssc <= 199) ) /* informational header */
            { rpsd->used -= (temp - header);
              if (count > 0) { my_memcpy(header, temp, count); temp = header; }
              ths = 0; goto next_char;
            }
            resource->state = rsReading; ths = -1; /* done with headerisms */
            if (count > 0)
            {
#if CONFIG_HTTP & HTTP_11
              if (((tHttpRpsd*)(resource->rpsd))->flags & hrfChunked)
              { parse_http_chunks(resource, temp, count);
                resource->tlheaderstate = ths;
                return;
              }
              else
#endif
              { resource_collect(resource, temp, count); }
            }
            goto header_done;
          }
          else if (ch == '\r') ths = 1;
          else ths = 0;
          break;
      }
      next_char: {}
    }
    header_done: resource->tlheaderstate = ths; goto prefinish;
  }

#if CONFIG_HTTP & HTTP_11
  is_chunked = cond2boolean(((tHttpRpsd*)(resource->rpsd))->flags &
    hrfChunked);
  if ( (is_chunked) && (http_cps(resource) != cpsData) )
  { /* only read a few bytes, they "should" mean the next chunk-size */
    char temp[15];
    conn_read(err, conn, temp, sizeof(temp), return);
    http_read_debug(resource, "chunky", err, temp);
    if (err > 0) { parse_http_chunks(resource, temp, err); return; }
    else goto stop_reading; /* error or EOF */
  }
#endif

  { size_t desired_size, bytecount = resource->bytecount;
    unsigned char rprflags;
    ncl = resource->nominal_contentlength;
    if ( (ncl != UNKNOWN_CONTENTLENGTH) && (bytecount < ncl) )
    { /* We know how much to expect, so we try (more or less) to slurp it all
         at once in order to avoid unnecessary read() calls. */
      ncl -= bytecount; /* subtract what was already read */
      if (ncl > HIGHEST_OCBS) ncl = HIGHEST_OCBS; /* (just a "sane" limit) */
      desired_size = ncl; rprflags = 0;
    }
    else { desired_size = bytecount; rprflags = 2; }
    resource_provide_room(resource, &dest, &size, desired_size, rprflags);
  }

#if CONFIG_HTTP & HTTP_11
  if (is_chunked)
  { size_t max = http_cl(resource);
    if (size > max) size = max; /* read at most until end of chunk */
  }
#endif
  conn_read(err, conn, dest, size, return);
  http_read_debug(resource, "data", err, dest);
  if (err <= 0) goto stop_reading;
  resource_record(resource, dest, err);
#if CONFIG_HTTP & HTTP_11
  if (is_chunked)
  { http_cl(resource) -= err;
    if (http_cl(resource) == 0) http_cps(resource) = cpsCr2;
  }
#endif

  prefinish:
  ncl = resource->nominal_contentlength;
  if ( (ncl != UNKNOWN_CONTENTLENGTH) && (resource->bytecount >= ncl) )
  { /* The server sent us all the announced data, so we're done: */
    err = 0; goto stop_reading;
  }
  finish:
#if CONFIG_HTTP & HTTP_AUTH_ANY
  if ( (!want_it_back) && (resource->server_status_code == 401) && (0) &&
       (!(((tHttpRpsd*)(resource->rpsd))->flags & hrfUseNewAuth)) &&
#if OPTION_LOCAL_CGI
       (resource->protocol != rpLocalCgi) &&
#endif
       (http_might_retry_with_auth(resource))
     )
  { ((tHttpRpsd*)(resource->rpsd))->flags |= hrfUseNewAuth;
    if (resource->cconn != NULL) conn_remove(&(resource->cconn));
    want_it_back = truE; resource->state = rsConnecting;
  }
#endif
  if ( (!want_it_back) || (resource->state == rsReading) )
    push_to_main_res(resource, boolean2bool(want_it_back));
}

static void http_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  switch (ccek)
  { case ccekConnectSetup:
      conn_set_write(conn);
      if (conn->flags & cnfConnected) goto do_setup;
      else push_to_main_res(resource, 1); /* say "Connecting to..." */
      break;
    case ccekConnectWorked:
      do_setup: http_rpsd_prepare(resource); http_setup_reqstr(resource);
      resource->state = rsMsgExchange;
#if OPTION_TLS
      if ( (resource->protocol == rpHttps) && (resource->proxy == NULL) )
      { tBoolean got_session = tls_session_init(conn,
          resource2textual_host(resource)->hostname);
        if (!got_session)
        { resource_set_error(resource, reTls); goto stopnpush; }
      }
#endif
      push_to_main_res(resource, 1);
      break;
    case ccekConnectFailed:
      resource_set_error(resource, conn_get_failre(conn));
      stopnpush: http_close(resource); push_to_main_res(resource, 0); break;
    case ccekRead:
#if OPTION_TLS
      if (conn->flags & cnfTlsHandshaking)
      { tdh: if (tls_do_handshaking(conn) == 0) goto stopnpush;
        break;
      }
#endif
      http_read(resource); break;
    case ccekWrite:
#if OPTION_TLS
      if (conn->flags & cnfTlsHandshaking) goto tdh;
#endif
      switch (conn_write_writedata(conn))
      { case 0:
          resource_set_error(resource, reServerClosed); goto stopnpush;
          /*@notreached@*/ break;
        case 1: break; /* keep writing */
        case 2: conn_set_read(conn); break; /* all written, start reading */
      }
      break;
    default: conn_bug(conn, ccek); break;
  }
}


/** Non-local resources: FTP */

#if CONFIG_FTP

my_enum1 enum
{ frfNone = 0, frfLastLine = 0x01, frfSuffsnip = 0x02, frfDir = 0x04,
  frfTryingEpsv = 0x08
#if OPTION_TLS
  , frfFtpsDataClear = 0x10
#endif
} my_enum2(unsigned char) tFtpRpsdFlags;

typedef struct
{ tRpsdRemover remover;
  const char* reply_snippet; /* e.g. part of EPSV reply */
#if CONFIG_FTPDIR > 0
  char* text; /* buffer for FTP directory listing beautification */
  size_t usable, used;
#endif
#if OPTION_TLS
  tFtpTlsMethod ftm;
#define ftps_ftm(resource) (((tFtpRpsd*)((resource)->rpsd))->ftm)
#endif
  tFtpRpsdFlags flags;
} tFtpRpsd;

#define ftp_frf(resource) (((tFtpRpsd*)((resource)->rpsd))->flags)

static void ftp_rpsd_remove(tRpsdGeneric* _rpsd)
{ tFtpRpsd* rpsd = (tFtpRpsd*) _rpsd;
  __dealloc(rpsd->reply_snippet);
#if CONFIG_FTPDIR > 0
  __dealloc(rpsd->text);
#endif
}

#if CONFIG_FTPDIR > 0

static one_caller void ftp_data_handle_dirline(tResource* resource, char* text)
/* adds one line to an FTP directory listing */
{ const char *filename, *htmlfilename, *info, *htmlinfo, *path;
  char *spfbuf, *ws;
  tBoolean found_ws, need_pathslash;
  size_t pathlen;

  if (strneqcase(text, "total", 5)) /* game... */
  { /* Some very funny servers send a "total <number>" line (probably from a
       dumb "ls -l" shell command) at the beginning of the directory listing,
       and the number shouldn't become a link. */
    const char* temp = text + 5;
    if ( (*temp == ' ') || (*temp == '\t') ) /* ...set... */
    { while ( (*temp == ' ') || (*temp == '\t') ) temp++;
      if (my_isdigit(*temp))
      { while (my_isdigit(*temp)) temp++;
        if (*temp == '\0') /* ...and match */
        { my_spf(strbuf, STRBUF_SIZE, &spfbuf, "<br>%s\n", text);
          goto do_collect;
        }
      }
    }
  }

  found_ws = falsE; ws = text + strlen(text) - 1;
  while (ws >= text)
  { char ch = *ws;
    if ( (ch == ' ') || (ch == '\t') ) { found_ws = truE; break; }
    ws--;
  }

  if (found_ws)
  { /* The server sent us some text before the actual file/dir name; we don't
       care what it is (probably some access rights like "rw-r--r--"), we just
       make sure it goes _before_ the link: */
    *ws++ = '\0'; filename = ws; info = text;
  }
  else { filename = text; info = strEmpty; }

  htmlfilename = htmlify(filename); htmlinfo = htmlify(info);
  path = resource->uri_data->path; pathlen = strlen(path);
  need_pathslash = cond2boolean((pathlen <= 0) || (path[pathlen - 1] !=
    chDirsep));
  my_spf(strbuf, STRBUF_SIZE, &spfbuf,
    "<br>%s%s<a href=\"%s://%s:%d%s%s%s\">%s</a>\n", htmlinfo,
    ( (*htmlinfo != '\0') ? strSpace : strEmpty ),
    rp_data[resource->protocol].scheme,
    resource2textual_host(resource)->hostname,
    ntohs(resource->uri_data->portnumber), path, (need_pathslash ? strSlash :
    strEmpty), filename, htmlfilename);
  htmlify_cleanup(filename, htmlfilename); htmlify_cleanup(info, htmlinfo);
  do_collect:
  resource_collect_str(resource, spfbuf);
  my_spf_cleanup(strbuf, spfbuf);
}

#endif /* #if CONFIG_FTPDIR > 0 */

#if CONFIG_DEBUG
static void debug_ftp_data_llread(tConnection* conn, int result,
  size_t bufsize, const char* debugstr)
{ const int e = errno;
  sprint_safe(debugstrbuf, "ftp_data_llread(%s): fd=%d,result=%d,bufsize=%d,errno=%d\n", debugstr, conn->fd, result, bufsize, e);
  debugmsg(debugstrbuf); errno = e;
}
#else
#define debug_ftp_data_llread(conn, result, bufsize, debugstr) do { } while (0)
#endif

#define ftp_data_llread(result, conn, buf, bufsize, debugstr) /* low-level */ \
  do \
  { conn_read(result, conn, buf, bufsize, return(3)); \
    debug_ftp_data_llread(conn, result, bufsize, debugstr); \
  } while (0)

static one_caller void ftp_data_dirstart(tResource* resource)
{ const tUriData* uri_data = resource->uri_data;
  const char* path = uri_data->path;
  char* spfbuf;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("Contents of FTP directory \"%s\""),
    path);
#if CONFIG_FTPDIR > 0
  resource->cantent->kind = rckHtml;
  resource_collect_title(resource, spfbuf);
#else
  resource_collect_str(resource, spfbuf);
  resource_collect_str(resource, "\n\n");
#endif
  my_spf_cleanup(strbuf, spfbuf);
#if CONFIG_FTPDIR > 0
  if (calc_parentpath(path, strbuf2, STRBUF_SIZE))
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      "<p><a href=\"%s://%s:%d%s\">%s</a></p>\n",
      rp_data[resource->protocol].scheme,
      resource2textual_host(resource)->hostname,
      ntohs(uri_data->portnumber), strbuf2, _(strParentDirectory));
    resource_collect_str(resource, spfbuf);
    my_spf_cleanup(strbuf, spfbuf);
  }
#endif
}

static one_caller unsigned char ftp_data_read(tResource* resource)
/* return value: 0=error, 1=EOF, 2=proceed, 3=interrupted (TLS only),
   4=do nothing (custom connections only) */
{ tFtpRpsd* rpsd = (tFtpRpsd*) resource->rpsd;
  tConnection* conn = resource->dconn;
  int err;
#if CONFIG_CUSTOM_CONN
  const tBoolean is_cc = cond2boolean(doing_custom_conn(resource));
  const tResourceFlags resflags = resource->flags;
  if ( (is_cc) && (resflags & rfCustomConnBd1) ) /* e.g. "ls" */
  { char ccdirbuf[4096];
    ftp_data_llread(err, conn, ccdirbuf, sizeof(ccdirbuf), "ccdir");
    if (err > 0) __custom_conn_print(resource, ccdirbuf, err, ccpkNetresp);
    else
    { finish_cc_data:
      resource->flags |= rfCustomConnBd2;
      if (err == -1) custom_conn_tell_error(resource, conn_err2error(errno));
      conn_remove(&(resource->dconn));
      if ( (resource->flags & (rfCustomConnBd2 | rfCustomConnBd3)) ==
           (rfCustomConnBd2 | rfCustomConnBd3) )
        custom_conn_unbusify(resource);
      return(4);
    }
  }
  else if ( (is_cc) && (resflags & rfDownload) ) /* e.g. "get" */
  { char* dest;
    size_t size;
    resource_provide_room(resource, &dest, &size, resource->bytecount, 2);
    ftp_data_llread(err, conn, dest, size, "ccget");
    if (err <= 0) goto finish_cc_data;
    resource_record(resource, dest, err); custom_conn_tell_hash(resource);
  }
  else
#endif
#if CONFIG_FTPDIR > 0
  if (rpsd->flags & frfDir)
  { char *text, *temp, *remainder, *end;
    size_t usable, used;
    tBoolean near_r;

    /* read */
    text = rpsd->text; usable = rpsd->usable; used = rpsd->used;
    if (usable <= used)
    { usable += 4096; rpsd->usable = usable;
      text = rpsd->text = memory_reallocate(text, usable, mapOther);
    }
    ftp_data_llread(err, conn, text + used, usable - used, "dir");
    if (err <= 0) /* error or EOF */
    { resource_collect_str(resource, strEndBodyHtml); goto stop_reading; }
    used += err; rpsd->used = used;

    /* look for CRLF */
    temp = remainder = text; end = text + used - 1; near_r = falsE;
    while (temp <= end)
    { const char ch = *temp++;
      if (ch == '\r') near_r = truE;
      else
      { if ( (ch == '\n') && (near_r) ) /* found a CRLF */
        { *(temp - 2) = '\0'; /* overwrites the "\r" */
          ftp_data_handle_dirline(resource, remainder);
          remainder = temp;
        }
        near_r = falsE;
      }
    }

    /* finish */
    if (remainder > text) /* did something */
    { if (remainder > end) rpsd->used = 0; /* did everything */
      else
      { const char* src = remainder;
        char* dest = text;
        while (src <= end) *dest++ = *src++;
        rpsd->used = rpsd->used - (remainder - text);
      }
    }
  }
  else /* isn't a directory */
#endif /* #if CONFIG_FTPDIR > 0 */
  { char* dest;
    size_t size;
    resource_provide_room(resource, &dest, &size, resource->bytecount, 2);
    ftp_data_llread(err, conn, dest, size, "file");
    if (err <= 0)
    { stop_reading:
      if (err == 0) { resource->state = rsComplete; return(1); }
      else { resource_set_error(resource, conn_err2error(errno)); return(0); }
    }
    else resource_record(resource, dest, err);
  }
  return(2); /* all fine */
}

static void ftp_data_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  switch (ccek)
  { case ccekConnectSetup:
      /* IMPLEMENTME: tell user "connecting to...", esp. for custom conns! */
      if (conn->flags & cnfConnected) goto do_setup;
      else { conn_set_write(conn); push_to_main_res(resource, 1); }
      break;
    case ccekConnectWorked:
      do_setup:
      if (ftp_frf(resource) & frfDir) ftp_data_dirstart(resource);
#if OPTION_TLS
      if ( (conn_using_tls(resource->cconn)) &&
           (!(ftp_frf(resource) & frfFtpsDataClear)) )
      { /* We want to protect the data connection whenever the control
           connection is protected and the user didn't explicitly specify the
           "dataclear" method. We can get here under two circumstances:
           1. rpFtps; 2. rpFtp on a custom connection with a manual "auth". */
        tBoolean got_session = tls_session_init(conn,
          resource2textual_host(resource)->hostname);
        if (!got_session)
        { resource_set_error(resource, reTls); goto stopnpush; }
      }
      else
#endif
      { conn_set_read(conn); }
      resource->state = rsReading; push_to_main_res(resource, 1);
      break;
    case ccekConnectFailed:
      resource_set_error(resource, conn_get_failre(conn));
      stopnpush:
      conn_remove(&(resource->dconn));
      conn_remove(&(resource->cconn)); /* CHECKME! */
      push_to_main_res(resource, 0);
      break;
#if OPTION_TLS
    case ccekWrite:
      if (!conn_using_tls(conn)) goto is_buggy;
      /* "else": */ /*@fallthrough@*/
#endif
    case ccekRead:
#if OPTION_TLS
      if (conn->flags & cnfTlsHandshaking)
      { if (tls_do_handshaking(conn) == 0) goto stopnpush;
        break;
      }
#endif
      switch (ftp_data_read(resource))
      { case 1:
          resource->server_status_code = 226; /* CHECKME: that's dirty! */
          /*@fallthrough@*/
        case 0: goto stopnpush; /*@notreached@*/ break;
        case 2: push_to_main_res(resource, 1); break;
        /* case 3: TLS operation was interrupted, just proceed */
        /* case 4: custom connection operation, nothing to do here */
      }
      break;
    default:
#if OPTION_TLS
      is_buggy:
#endif
      conn_bug(conn, ccek); break;
  }
}

static const char* ftp_prepare_snippet(const tResource* resource)
{ const char *retval = NULL, /* assume failure */
    *ptr = ((tFtpRpsd*)(resource->rpsd))->reply_snippet;
  unsigned char count;
  if (ptr == NULL) goto out;
  for (count = 0; count <= 2; count++)
  { const char ch = *ptr++;
    if (!my_isdigit(ch)) goto out; /* "can't happen" */
  }
  if ( (*ptr != ' ') && (*ptr != '-') ) goto out; /* "can't happen" */
  ptr++; retval = ptr;
  out:
  return(retval);
}

static one_caller tBoolean ftp_setup_dconn(tResource* resource,
  /*@out@*/ tResourceError* error)
/* returns whether it worked */
{ const char *ptr = ftp_prepare_snippet(resource), *ptr2, *path;
  char ch;
  tConnection* conn;
  tBoolean is_epsv = cond2boolean(ftp_frf(resource) & frfTryingEpsv),
    is_connected;
  tSockaddr addr;
  size_t addrlen;
  int data_fd, err, address_family;

  if (ptr == NULL) { bad_ftp: *error = reResponse; failed: return(falsE); }
  ptr2 = my_strchr(ptr, '(');
  if (ptr2 != NULL)
  { ptr2++;
    if ( (*ptr2 != '\0') && ( (is_epsv || my_isdigit(*ptr2)) ) )
    { ptr = ptr2; goto snipfin; }
  }
  if (is_epsv) goto bad_ftp; /* invalid server reply format */
  while ( (ch = *ptr++) != '\0' )
  { if (my_isdigit(ch)) { ptr--; goto snipfin; } }
  goto bad_ftp;

  snipfin:
  if (is_epsv)
  { const tSockaddrEntry* sockaddr_entry;
    const char dc = *ptr++; /* "delimiting character" */
    int _portnumber;
    if ( (*ptr != dc) || (*(ptr + 1) != dc) ) goto bad_ftp;
    ptr += 2; if (!my_isdigit(*ptr)) goto bad_ftp;
    my_atoi(ptr, &_portnumber, &ptr, 99999);
#if CONFIG_DEBUG
    sprint_safe(debugstrbuf, "ftp_setup_dconn(): port %d\n", _portnumber);
    debugmsg(debugstrbuf);
#endif
    if ( (_portnumber < 0) || (_portnumber > 65535) || (*ptr != dc) )
      goto bad_ftp;
    sockaddr_entry = conn2sockaddr(resource->cconn);
    if (sockaddr_entry == NULL)
    { *error = reHandshake; /* CHECKME: it's not exactly a _handshake_ bug! */
      goto failed;
    }
    addr = sockaddr_entry->addr;
    address_family = sockaddr_entry->address_family;
    set_portnumber(&addr, address_family, htons((tPortnumber) _portnumber));
    addrlen = sockaddr_entry->addrlen;
  }
  else /* used PASV */
  { int a, b, c, d, p, q;
    unsigned int hostlong;
    unsigned short int hostshort;
    PREPARE_SSCANF_FORMAT(format, 30, "%d,%d,%d,%d,%d,%d")
    if (sscanf(ptr, format, &a, &b, &c, &d, &p, &q) != 6) goto bad_ftp;
    if ( (a < 0) || (a > 255) || (b < 0) || (b > 255) || (c < 0) || (c > 255)
      || (d < 0) || (d > 255) || (p < 0) || (p > 255) || (q < 0) || (q > 255) )
    { goto bad_ftp; }
    hostlong = (a << 24) | (b << 16) | (c << 8) | (d);
    hostshort = (p << 8) | (q);
    my_memclr_var(addr);
    ((struct sockaddr_in*)&addr)->sin_family = address_family = AF_INET;
    ((struct sockaddr_in*)&addr)->sin_addr.s_addr = htonl(hostlong);
    set_portnumber(&addr, AF_INET, htons(hostshort));
    addrlen = sizeof(struct sockaddr_in);
  }

  data_fd = create_socket(address_family);
  if (data_fd < 0)
  { *error = ( (data_fd == -2) ? reTmofd : reSocket ); goto failed; }

  err = my_do_connect(data_fd, &addr, addrlen);
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "FTP data connect: %d,%d,%d,%d\n", data_fd, err,
    errno, address_family);
  debugmsg(debugstrbuf);
#endif
  if (err == 0) is_connected = truE; /* unlikely when non-blocking */
  else if (errno == EINPROGRESS) is_connected = falsE;
  else
  { *error = connect_err2error(errno); my_close_sock(data_fd); goto failed; }

  if ((path = resource->uri_data->path) != NULL)
    resource->cantent->kind = filesuffix2rck(path);
  if (resource->dconn != NULL) /* "should not happen" */
    conn_remove(&(resource->dconn));
  conn = resource->dconn = conn_create(data_fd, ftp_data_callback, resource,
    truE, NULL, resource->protocol);
  if (is_connected) conn->flags |= cnfConnected;
  conn_callback(conn, ccekConnectSetup);
  return(truE);
}

/* File Transfer Protocol server status code equality check; RFC1123 says: "A
   User-FTP SHOULD generally use only the highest-order digit of a 3-digit
   reply code for making a procedural decision, to prevent difficulties when a
   Server-FTP uses non-standard reply codes." CHECKME: should we really? */
#if 1
#define FTPSSCEQ(value1, value2) ((value1) == (value2))
#else
#define FTPSSCEQ(value1, value2) (((value1) / 100) == ((value2) / 100))
#endif

#if CONFIG_USER_QUERY

static __my_inline tBoolean resource_ask_username(tResource* resource,
  tUserQueryCallback callback, unsigned char flags)
{ return(resource_ask_anything(resource, callback, mifUsername, flags));
}

static __my_inline tBoolean resource_ask_password(tResource* resource,
  tUserQueryCallback callback, unsigned char flags)
{ return(resource_ask_anything(resource, callback, mifPassword, flags));
}

static void ftp_user_callback(tUserQuery*); /* prototype */

#endif

#if OPTION_TLS

static tFtpTlsMethod calc_ftp_tls_method(const tResource* resource)
{ tFtpTlsMethod retval;
  const tConfigFtpTlsMethod* cftm;
  const tBoolean is_ftps = cond2boolean(resource->protocol == rpFtps);
#if CONFIG_CUSTOM_CONN
  if ( (doing_custom_conn(resource)) && (is_ftps) )
  { retval = ftmTls; goto out; }
#endif
  retval = ftmAutodetect; cftm = config.ftp_tls_method;
  if (cftm != NULL)
  { const char* hostname = resource2textual_host(resource)->hostname;
    const tPortnumber resport = resource->uri_data->portnumber;
    while (cftm != NULL)
    { tPortnumber hp = cftm->hosts_portnumber;
      if ( ( (hp == 0) || (hp == resport) ) &&
           (my_pattern_matcher(cftm->hosts_pattern, hostname)) )
      { retval = cftm->ftm; break; }
      cftm = cftm->next;
    }
  }
  if ( (retval == ftmAutodetect) && (is_ftps) &&
       (resource->textual_hppi->portnumber == htons(990)) )
    retval = ftmTls; /* (earliest autodetection:-) */
  out:
  return(retval);
}

static tBoolean ftp_control_tls_session_init(tConnection* conn,
  tResource* resource)
{ const tBoolean retval = tls_session_init(conn,
    resource2textual_host(resource)->hostname);
  conn_set_dissolver(conn, quitcmd_dissolver); /* first quit, then close :-) */
  return(retval);
}

#if CONFIG_USER_QUERY
static void ftps_dataclear_callback(tUserQuery*); /* prototype */
#endif

#endif /* #if OPTION_TLS */

static void ftp_control_do_start_command(tResource* resource, const char* str)
{ tConnection* conn = resource->cconn;
#if CONFIG_DEBUG
  debugmsg("starting new FTP command: "); debugmsg(str);
#endif
#if CONFIG_CUSTOM_CONN
  if (doing_custom_conn(resource)) custom_conn_print(resource, str,ccpkNetcmd);
#endif
  conn_set_writestr(conn, str); conn_set_write(conn);
}

#define FCSCR(x) do { retval = (x); goto out; } while (0)

static unsigned char ftp_control_start_command(tResource* resource,
  /*@out@*/ tResourceError* _ftp_error)
{ unsigned char retval; /* 0=error, 1=suspended (for user query), 2=started */
  const tSockaddrPortProtInfo* sppi;
  const char *path, *temp;
  char* spfbuf;
  size_t pathlen;
  switch (resource->handshake)
  { case rchFtpUser:
      if ( (temp = resource->uri_data->username) != NULL )
        my_spf(NULL, 0, &spfbuf, strNetcmdUser, temp);
#if CONFIG_USER_QUERY
      else if (resource_ask_username(resource, ftp_user_callback, 0))
      { susp: FCSCR(1); }
#endif
      else { login_failure: *_ftp_error = reLogin; failed: FCSCR(0); }
      break;
    case rchFtpPassword:
      if ( (temp = resource->uri_data->password) != NULL )
        my_spf(NULL, 0, &spfbuf, strNetcmdPass, temp);
#if CONFIG_USER_QUERY
      else if (resource_ask_password(resource, ftp_user_callback, 0))
        goto susp;
#endif
      else goto login_failure;
      break;
    case rchFtpPasv:
      /* We use PASV/EPSV instead of PORT/EPRT due to RFC1579; but IMPLEMENTME:
         fall back to PORT/EPRT when PASV/EPSV fails? - We always try the more
         modern EPSV command (RFC2428) first unless we definitely know that the
         server doesn't support it. */
      sppi = conn2sppi(resource->cconn, falsE);
      if ( (sppi == NULL) || (!(sppi->sppif & sppifFtpCannotEpsv)) )
      { ftp_frf(resource) |= frfTryingEpsv;
        spfbuf = my_strdup("EPSV\r\n"); /* CHECKME: "EPSV ALL"? */
      }
      else spfbuf = my_strdup("PASV\r\n");
      break;
    case rchFtpRequest:
      path = resource->uri_data->path; pathlen = strlen(path);
      if ( (pathlen > 0) && (path[pathlen - 1] == '/') )
        ftp_frf(resource) |= frfDir;
      if (!strncmp(path, "/~/", 3)) path += 3;
      my_spf(NULL, 0, &spfbuf, "%s%s%s\r\n", ( (ftp_frf(resource) & frfDir)
        ? strList : strRetr ), ( (*path != '\0') ? strSpace : strEmpty ),
        path); /* IMPLEMENTME: try "MLSD" instead of "LIST" first? */
      break;
#if OPTION_TLS
    case rchFtpTlsAuthTls: spfbuf = my_strdup("AUTH TLS\r\n"); break;
    case rchFtpTlsAuthSsl: spfbuf = my_strdup("AUTH SSL\r\n"); break;
    case rchFtpTlsPbsz: spfbuf = my_strdup("PBSZ 0\r\n"); break;
    case rchFtpTlsProt:
      { const tBoolean data_clear =
          cond2boolean(ftps_ftm(resource) == ftmAuthTlsDataclear);
        my_spf(NULL, 0, &spfbuf, "PROT %c\r\n", (data_clear ? 'C' : 'P'));
        if (data_clear) ftp_frf(resource) |= frfFtpsDataClear;
      }
      break;
#endif
    default: *_ftp_error = reHandshake; goto failed; /*@notreached@*/ break;
  }
  ftp_control_do_start_command(resource, my_spf_use(spfbuf));
  FCSCR(2);
  out:
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "FTP-csc: retval=%d\n", retval);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

#undef FCSCR

my_enum1 enum
{ fhdfNone = 0
#if OPTION_TLS
  , fhdfTlsFirst = 0x01
#endif
} my_enum2(unsigned char) tFtpHandshakeDataFlags;

static struct
{ tResourceCommandHandshake next_rch;
  tResourceError re;
  tFtpHandshakeDataFlags flags;
} ftp_handshake_data;

#define FCHR(x) do { retval = (x); goto out; } while (0)

static one_caller unsigned char ftp_control_handshake(tResource* resource)
/* return value: 0=error, 1=command sequence finished, 2=suspended (for user
   query), 3=possibly prepared next */
{ unsigned char retval;
  const tServerStatusCode status = resource->server_status_code;
  tResourceCommandHandshake next = rchUnknown;
  tConnection* conn = resource->cconn;
  switch (resource->handshake)
  { case rchConnected:
      if (FTPSSCEQ(status, 220)) /* "server ready", "awaiting input" */
      { const char* ptr = ftp_prepare_snippet(resource);
        if (ptr != NULL) conn_set_software_id(conn, ptr);
        conn_set_dissolver(conn, quitcmd_dissolver); next = rchFtpUser;
#if OPTION_TLS
        if (resource->protocol == rpFtps)
        { switch (ftps_ftm(resource))
          { case ftmAutodetect: case ftmAuthTls: case ftmAuthTlsDataclear:
              next = rchFtpTlsAuthTls; break;
            case ftmAuthSsl: prepare_auth_ssl: next = rchFtpTlsAuthSsl; break;
          }
        }
#endif
      }
      break;
    case rchFtpUser:
      if ( (FTPSSCEQ(status, 230))
#if OPTION_TLS
          || ( (resource->protocol == rpFtps) && (FTPSSCEQ(status, 232)) )
#endif
         ) /* "no password/account required"; skip "PASS" command */
      { prepare_epsv_pasv: next = rchFtpPasv; }
      else if (status == 331) next = rchFtpPassword; /* "password required" */
      else if ( (status == 332) || (status == 532) ) /* "account required" */
        next = rchFtpPassword; /* (see 332/532 comment below) */
      else
      {
#if CONFIG_USER_QUERY
        if ( (status == 530) &&
             (resource_ask_username(resource, ftp_user_callback, 1)) )
        { drp_susp: dealloc(resource->uri_data->password); susp: FCHR(2); }
#endif
        login_failed: ftp_handshake_data.re = reLogin; failed: FCHR(0);
      }
      break;
    case rchFtpPassword:
      if (status == 230) /* "user logged in" */
      { const tUriData* u = resource->uri_data;
        hppi_login_set(resource->textual_hppi, u->username, u->password);
        goto prepare_epsv_pasv;
      }
      else if (status == 202)  /* "command was superfluous" */
        goto prepare_epsv_pasv;
      else if ( (status == 332) || (status == 532) ) /* "account required" */
      { /* We don't actually send account information (simply because neither
           account configuration nor FTP upload has been implemented yet),
           but we also don't cause a "login failed" error message. 332 and
           532 often mean "account might be required later, e.g. for upload"
           and not "account is required _now_, or all further operations will
           be refused". So we simply proceed and hope the best. :-) */
        goto prepare_epsv_pasv;
      }
      else
      {
#if CONFIG_USER_QUERY
        if ( ( (status == 530) /* || (status == 421) */ ) &&
             (resource_ask_password(resource, ftp_user_callback, 1)) )
        { /* (Yes, some buggy servers send 421 instead of the correct 530 to
              indicate "login failed"; IMPLEMENTME: must sometimes create a new
              connection and resend the login commands in this case!) */
          goto drp_susp;
        }
#endif
        goto login_failed;
      }
      break;
    case rchFtpPasv:
      if (ftp_frf(resource) & frfTryingEpsv)
      { if (FTPSSCEQ(status, 229)) /* "entering passive mode" (EPSV) */
        { epsv_pasv_worked:
          if (!ftp_setup_dconn(resource, &(ftp_handshake_data.re)))goto failed;
          next = rchFtpRequest;
        }
        else /* "EPSV" failed */
        { tSockaddrPortProtInfo* sppi = conn2sppi(conn, truE);
          if (sppi != NULL) sppi->sppif |= sppifFtpCannotEpsv;
          ftp_frf(resource) &= ~frfTryingEpsv;
          goto prepare_epsv_pasv; /* try "PASV" */
        }
      }
      else if (FTPSSCEQ(status, 227)) /* "entering passive mode" (PASV) */
      { goto epsv_pasv_worked; }
      break;
    case rchFtpRequest:
      if (status == 226)
      { /* We leave the control connection open until the data transfer
           "actually" finished, not until the server thinks it's finished. */
        FCHR(1);
      }
      else if ( (status / 10 == 55) && (!doing_custom_conn(resource)) )
      { if (!(ftp_frf(resource) & frfDir))
        { /* The FTP server says something like "file not found", so we hope
             the thing is available as a _directory_ and try to get that: */
          if (resource->dconn != NULL) conn_remove(&(resource->dconn));
          ftp_frf(resource) |= frfDir; debugmsg("retrying as FTP dir\n");
          goto prepare_epsv_pasv;
        }
        /* IMPLEMENTME: else { ....error = re....; goto failed; } */
      }
      break;
#if OPTION_TLS
    case rchFtpTlsAuthTls:
      if (FTPSSCEQ(status, 234))
      { next = rchFtpTlsPbsz; tls1: ftp_handshake_data.flags |= fhdfTlsFirst; }
      else if ( (status / 100 == 5) && (ftps_ftm(resource) == ftmAutodetect) )
      { /* The server didn't like "AUTH TLS", but maybe it likes "AUTH SSL". */
        goto prepare_auth_ssl;
      }
      break;
    case rchFtpTlsAuthSsl:
      if ( (FTPSSCEQ(status, 234)) || (FTPSSCEQ(status, 334)) )
      { /* (The 334 is technically wrong, but some buggy servers use(d) it.) */
        next = rchFtpUser; goto tls1;
      }
      break;
    case rchFtpTlsPbsz:
      if (FTPSSCEQ(status, 200)) next = rchFtpTlsProt;
      break;
    case rchFtpTlsProt:
      if (FTPSSCEQ(status, 200)) next = rchFtpUser;
#if CONFIG_USER_QUERY
      else if ( ( (status == 534) || (status == 504) || (status == 431) ) &&
        (ftps_ftm(resource) == ftmAutodetect) &&
        (resource_ask_anything(resource, ftps_dataclear_callback,
          mifFtpsDataclear, 0)) )
      { goto susp; }
#endif
      break;
#endif /* TLS */
    default: /* "should not happen" */
      ftp_handshake_data.re = reHandshake; goto failed; /*@notreached@*/ break;
  }
  ftp_handshake_data.next_rch = next;
  FCHR(3);
  out:
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "FTP-hs: retval=%d, next=%d\n", retval, next);
  debugmsg(debugstrbuf);
#endif
  return(retval);
}

#undef FCHR

static one_caller unsigned char
  ftp_control_handshake_proceed(tResource* resource)
/* returns whether the control connection may persist (1) or not (0) or, as a
   special case, that a TLS handshake shall be started (2) */
{ unsigned char retval = 1; /* most likely result */
  unsigned char handshake_result;
  tResourceCommandHandshake next;
  tResourceError re;
#if CONFIG_CUSTOM_CONN
  const tBoolean is_cc = cond2boolean(doing_custom_conn(resource));
  if ( (is_cc) && (resource->handshake == rchFtpCustom) )
  { if (resource->flags & (rfCustomConnBd1 | rfCustomConnBd4))
    { resource->flags |= rfCustomConnBd3;
      if ( (resource->server_status_code != 226) && (resource->dconn != NULL) )
      { conn_remove(&(resource->dconn)); resource->flags |= rfCustomConnBd2; }
      else if (resource->dconn == NULL) resource->flags |= rfCustomConnBd2;
      if ( (resource->flags & (rfCustomConnBd2 | rfCustomConnBd3)) ==
           (rfCustomConnBd2 | rfCustomConnBd3) )
      { /* unbusify */ }
      else goto out; /* e.g. still receiving a directory listing */
    }
    /* The command was absolutely custom, so we don't even have to _try_ to
       calculate an automatic next handshake. Just tell the user we're done: */
    do_unbusify: custom_conn_unbusify(resource); goto out;
  }
#endif
  my_memclr_var(ftp_handshake_data);
  handshake_result = ftp_control_handshake(resource);
#if CONFIG_CUSTOM_CONN
  if (is_cc)
  { if (handshake_result == 0)
    { resource->flags |= rfCustomConnStopSequencer;
      custom_conn_tell_error(resource, ftp_handshake_data.re);
    }
    else if (handshake_result == 3)
    { if (resource->handshake == ftp_handshake_data.next_rch)
      { /* The next handshake would be the same as the current one. This means
           that the server didn't understand a specific command and we want to
           try a different command which will have the same or a similar
           effect. The most important case for this is where EPSV failed and we
           want to try PASV now. Whatever, this means that we are still "busy"
           and want to proceed automatically, so we don't want to unbusify: */
        switch (ftp_control_start_command(resource, &re))
        { case 0: goto failed; /*@notreached@*/ break;
          case 1: /* would be a bug here... */
            re = reHandshake; goto failed; /*@notreached@*/ break;
          case 2: break; /* all fine */
        }
        goto out;
      }
#if OPTION_TLS
      if (ftp_handshake_data.flags & fhdfTlsFirst)
      { if ( (resource->handshake == rchFtpTlsAuthSsl) &&
             (ftp_handshake_data.next_rch == rchFtpUser) )
        { /* we did rchFtpTlsAuthSsl and want to perform just the TLS
             handshake, not send an automated rchFtpUser for this _custom_
             connection... */
          resource->cconn->flags &= ~cnfWantToWrite; retval = 2; goto out;
        }
        else goto want_tls;
      }
      else if (ftp_handshake_data.next_rch == rchFtpTlsProt) goto start_prot;
#endif
    }
    else if (handshake_result == 2) goto out;
    goto do_unbusify;
  }
#endif
  switch (handshake_result)
  { case 0:
      re = ftp_handshake_data.re;
      failed: resource_set_error(resource, re); retval = 0; break;
    case 1: case 2: break;
    case 3:
#if OPTION_TLS
      if (ftp_handshake_data.flags & fhdfTlsFirst) { want_tls: retval = 2; }
      start_prot:
#endif
      next = ftp_handshake_data.next_rch;
      if (next == rchUnknown) { re = reResponse; goto failed; }
      resource->handshake = next;
      switch (ftp_control_start_command(resource, &re))
      { case 0: goto failed; /*@notreached@*/ break;
        case 1: break; /* suspended */
        case 2: push_to_main_res(resource, 1); break; /* started */
      }
      break;
  }
  out:
  return(retval);
}

static unsigned char ftp_control_read(tResource* resource)
/* We treat FTP control connections in a special way because we can throw
   almost every byte of input from them directly into the bit bucket. We need
   mostly two things: 1. we wanna know the server status code ("reply code" in
   FTP terminology); 2. we must recognize the end of a (single- or multi-line)
   control message; return value: 0=error, 1=proceeding, 2=reply complete. */
{ enum { FTP_BUFSIZE = (4096 + 1) };
  char buf[FTP_BUFSIZE], *ptr; /* the bit bucket */
  tTlHeaderState ths = resource->tlheaderstate;
  tResourceCommandHandshake handshake;
  tResourceError ftp_error = reServerClosed;
  tConnection* conn = resource->cconn;
  tServerStatusCode status;
  int err, count;

  conn_read(err, conn, buf, FTP_BUFSIZE - 1, return(1));
  if (err <= 0) /* error (EOF is also an error here because it's unexpected) */
  { failed: resource_set_error(resource, ftp_error);
    resource->tlheaderstate = ths; /* CHECKME: set "= 0" instead? */
    return(0);
  }

  buf[err] = '\0'; /* for simplicity (e.g. my_strchr()) */
#if CONFIG_DEBUG
  debugmsg("ftp_control_read(): "); debugmsg(buf);
  if (buf[err - 1] != '\n') debugmsg(strNewline);
#endif
#if CONFIG_CUSTOM_CONN
  if (doing_custom_conn(resource)) custom_conn_print(resource,buf,ccpkNetresp);
#endif
  ptr = buf; count = err; handshake = resource->handshake;

  if ( (handshake == rchConnected) || (handshake == rchFtpPasv) )
  { /* might need a part (first line, "snippet") of the reply text later... */
    tFtpRpsd* rpsd = (tFtpRpsd*) (resource->rpsd);
    const char *ptrr, *ptrn;
    size_t len;
    if (ths == 0) { dealloc(rpsd->reply_snippet); rpsd->flags &= ~frfSuffsnip;}
    else if (rpsd->flags & frfSuffsnip) goto skipsnip;
    ptrr = my_strchr(buf, '\r'); ptrn = my_strchr(buf, '\n');
    if ( (ptrr == NULL) || ( (ptrn != NULL) && (ptrr > ptrn) ) ) ptrr = ptrn;
    if (ptrr == NULL) len = strlen(buf); /* must store all */
    else { len = ptrr - buf; rpsd->flags |= frfSuffsnip; }
    if (len <= 0) goto skipsnip;
    if (rpsd->reply_snippet == NULL) rpsd->reply_snippet = my_strndup(buf,len);
    else
    { const size_t rrslen = strlen(rpsd->reply_snippet);
      char* s = __memory_allocate(rrslen + len + 1, mapString);
      my_memcpy(s, rpsd->reply_snippet, rrslen);
      my_memcpy(s + rrslen, buf, len); s[rrslen + len] = '\0';
      memory_deallocate(rpsd->reply_snippet); rpsd->reply_snippet = s;
    }
    skipsnip: {}
  }

  while (count-- > 0)
  { const char ch = *ptr++;
    switch (ths)
    { case 0: resource->server_status_code = 0; /*@fallthrough@*/
      case 1: case 2: /* handle the server status code at message start */
        if (!my_isdigit(ch)) { bad_ftp: ftp_error = reResponse; goto failed; }
        status = resource->server_status_code;
        status = 10 * status + ((tServerStatusCode) (ch - '0'));
        resource->server_status_code = status; ths++;
        if ( (ths == 3) && ( (status < 100) || (status > 699) ) )
          goto bad_ftp; /* (The 6yz reply codes were introduced in RFC2228.) */
        break;
      case 3: /* check whether single- or multi-line message */
        if (ch == ' ') ftp_frf(resource) |= frfLastLine;
        else if (ch == '-') ftp_frf(resource) &= ~frfLastLine;
        else goto bad_ftp;
        ths++; break;
      case 4: /* somewhere in a message line */
        if (ch == '\r') ths++;
        break;
      case 5:
        if (ch == '\n') /* reached the end of a message line */
        { ths++;
          if (ftp_frf(resource) & frfLastLine)
          { /* finished the last line; now look what to do: */
            resource->tlheaderstate = ths = 0;
            status = resource->server_status_code;
#if CONFIG_DEBUG
            sprint_safe(debugstrbuf,
              "found end of FTP control message (fd=%d, rch=%d, ssc=%d)\n",
              conn->fd, resource->handshake, status);
            debugmsg(debugstrbuf);
#endif
            if ( (status / 100) == 1 ) continue; /* intermediary reply */
#if 0
            else if (status == 421) { /* .... */ } /* RFC1123, 4.1.2.11 */
              /* forget it - some buggy servers send 421 when they mean 530 */
#endif
            return(2);
          }
        }
        else if (ch == '\r') { /* ths = 5; */ }
        else ths = 4;
        break;
      case 6: case 7: case 8: /* beginning of line within multi-line message */
        if (my_isdigit(ch))
        { /* still looks like the status code at the beginning of the last line
             of a multi-line message */
          ths++;
        }
        else ths = ( (ch == '\r') ? 5 : 4 );
        break;
      default: /* ths == 9 */
        if (ch == ' ') ftp_frf(resource) |= frfLastLine; /* _is_ last line */
        ths = ( (ch == '\r') ? 5 : 4 ); break;
    } /* switch */
  } /* while count */
  resource->tlheaderstate = ths; return(1);
}

#if CONFIG_USER_QUERY

static void ftp_user_callback(tUserQuery* query)
{ tMissingInformationFlags mif = query->mif;
  tResource* resource;
  tResourceError re;
  if (mif & mifObjectVanished) goto out; /* nothing further to do */
  resource = query->resource; resource_unsuspend(resource);
  if (mif & mifQueryFailed)
  { do_bug: resource_set_error(resource, reLogin);
    do_push: resource_stop(resource); push_to_main_res(resource, 0); goto out;
  }
  if (mif & mifUserCancelled) goto do_push;
  if (mif & mifUsername) resource->handshake = rchFtpUser;
  else if (mif & mifPassword) resource->handshake = rchFtpPassword;
  else goto do_bug;
  (void) ftp_control_start_command(resource, &re); /* CHECKME: handle error? */
  out:
  resource_ask_finish(query);
}

#if OPTION_TLS
static void ftps_dataclear_callback(tUserQuery* query)
{ tMissingInformationFlags mif = query->mif;
  tResource* resource;
  tResourceError re;
  if (mif & mifObjectVanished) goto out; /* nothing further to do */
  resource = query->resource; resource_unsuspend(resource);
  if (mif & (mifQueryFailed | mifUserCancelled))
  { resource_stop(resource); push_to_main_res(resource, 0); goto out; }
  ftps_ftm(resource) = ftmAuthTlsDataclear;
  (void) ftp_control_start_command(resource, &re); /* CHECKME: handle error? */
  out:
  resource_ask_finish(query);
}
#endif

#endif /* #if CONFIG_USER_QUERY */

static void ftp_control_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  tResourceError ftp_error;
  switch (ccek)
  { case ccekConnectSetup:
#if CONFIG_CUSTOM_CONN
      if (doing_custom_conn(resource)) custom_conn_tell_msg(resource);
#endif
      if (conn->flags & cnfConnected) goto do_setup;
      else { conn_set_write(conn); push_to_main_res(resource, 1); }
      break;
    case ccekConnectWorked:
      do_setup:
      resource->state = rsMsgExchange; resource->handshake = rchConnected;
      resource->rpsd = memory_allocate(sizeof(tFtpRpsd), mapRpsd);
      resource->rpsd->remover = ftp_rpsd_remove;
#if OPTION_TLS
      if (resource->protocol == rpFtps)
      { tFtpTlsMethod ftm = ftps_ftm(resource) = calc_ftp_tls_method(resource);
        if (ftm == ftmTls) /* immediate handshake */
        { start_session:
          if (!ftp_control_tls_session_init(conn, resource))
          { ftp_error = reTls; goto set_error; }
          else push_to_main_res(resource, 1);
          break; /* to skip the conn_set_read() call */
        }
      }
#endif
      conn_set_read(conn); /* to get the "server ready" message */
      push_to_main_res(resource, 1);
      break;
    case ccekConnectFailed:
      ftp_error = conn_get_failre(conn);
      set_error: resource_set_error(resource, ftp_error);
      stopnpush: conn_remove(&(resource->cconn));
      if (resource->dconn != NULL) conn_remove(&(resource->dconn));
      push_to_main_res(resource, 0);
      break;
    case ccekRead:
#if OPTION_TLS
      if (conn->flags & cnfTlsHandshaking)
      { unsigned char tdhres;
        tdh:
        tdhres = tls_do_handshaking(conn);
        if (tdhres == 0) goto stopnpush;
#if CONFIG_CUSTOM_CONN
        else if ( (tdhres == 2) && (doing_custom_conn(resource)) &&
                  (resource->handshake == rchFtpTlsAuthSsl) )
        { custom_conn_unbusify(resource); return; }
#endif
        break;
      }
#endif
      switch (ftp_control_read(resource))
      { case 0: goto stopnpush; /*@notreached@*/ break;
        case 1: break;
        case 2:
          switch (ftp_control_handshake_proceed(resource))
          { case 0: goto stopnpush; /*@notreached@*/ break;
            case 1: break; /* all fine */
#if OPTION_TLS
            case 2: goto start_session; /*@notreached@*/ break;
#endif
          }
          break;
      }
      break;
    case ccekWrite:
#if OPTION_TLS
      if (conn->flags & cnfTlsHandshaking) goto tdh;
#endif
      switch (conn_write_writedata(conn))
      { case 0:
          resource_set_error(resource, reServerClosed);
          goto stopnpush; /*@notreached@*/ break;
        case 1: break; /* keep writing */
        case 2: conn_set_read(conn); break; /* all written, start reading */
      }
      break;
    default: conn_bug(conn, ccek); break;
  }
}

#endif /* #if CONFIG_FTP */


/** Non-local resources: NNTP */

#if OPTION_NEWS

typedef signed int tNewsArticleNumber;
  /* ("signed" for simplicity only; e.g. if a <firstnum> is 0, a "num >=
      firstnum" test would always be true for an unsigned variable) */

my_enum1 enum
{ nrfNone = 0, nrfHeaderSeries = 0x01
} my_enum2(unsigned char) tNntpRpsdFlags;

typedef struct
{ tRpsdRemover remover;
  char* text;
  size_t usable, used;
  tNewsArticleNumber curr_artnum, end_artnum;
  tNntpRpsdFlags flags;
} tNntpRpsd;

static void news_rpsd_remove(tRpsdGeneric* _rpsd)
{ tNntpRpsd* rpsd = (tNntpRpsd*) _rpsd;
  __dealloc(rpsd->text);
}

static one_caller void news_rpsd_prepare(tResource* resource)
{ tNntpRpsd* rpsd = (tNntpRpsd*) resource->rpsd;
  if (rpsd == NULL)
  { void* x = memory_allocate(sizeof(tNntpRpsd), mapRpsd);
    rpsd = x; resource->rpsd = x;
    rpsd->remover = news_rpsd_remove;
  }
  else { rpsd->used = 0; /* rpsd->curr_artnum = rpsd->end_artnum = 0; */ }
}

typedef struct tNewsArticle
{ struct tNewsArticle* next;
  char *header, *lastheader, *body, *lastbody; /* content blocks */
  char* id;
  tNewsArticleNumber num;
} tNewsArticle;

enum { ngifNone = 0, ngifCannotPost = 0x01 };
typedef unsigned char tNewsGroupInformationFlags;

typedef struct tNewsGroupInformation
{ struct tNewsGroupInformation* next;
  char* name;
  tNewsArticle* articles;
  tNewsArticleNumber estnum, firstnum, lastnum;
    /* estimated number of articles; number of the first and last article */
  tNewsGroupInformationFlags flags;
} tNewsGroupInformation;

static /* __sallocator -- not an "only" reference */ tNewsGroupInformation*
  __callocator news_ngi_create(tCachedHostInformation* hostinfo,
  const char* name)
{ tNewsGroupInformation* retval = (tNewsGroupInformation*)
    memory_allocate(sizeof(tNewsGroupInformation), mapPermanent);
  retval->next = hostinfo->ngi;
  retval->name = my_strdup(name);
  hostinfo->ngi = retval;
  return(retval);
}

static tNewsGroupInformation* news_ngi_lookup(tCachedHostInformation*
  hostinfo, const char* desired_name, tBoolean do_create_if_null)
{ tNewsGroupInformation* retval = hostinfo->ngi;
  while (retval != NULL)
  { const char* name = retval->name;
    if ( (name != NULL) && (!strcmp(name, desired_name)) ) break; /* found */
    retval = retval->next;
  }
  if ( (retval == NULL) && (do_create_if_null) )
    retval = news_ngi_create(hostinfo, desired_name);
  return(retval);
}

static void news_resource2path(const tResource* resource,
  /*@out@*/ const char** _path, /*@out@*/ tBoolean* _has_at)
/* prepares the path value of the <resource> a little bit */
{ const char* path = resource->uri_data->path;
  *_has_at = falsE;
  if (path != NULL)
  { if (*path == '/') path++;
    if (*path == '\0') path = NULL;
    else if (my_strchr(path, '@')) *_has_at = truE;
  }
  *_path = path;
}

static void news_resource2group(const tResource* resource,
  /*@out@*/ const char** _group, /*@out@*/ tBoolean* _must_dealloc)
{ const char* path;
  tBoolean has_at;
  *_group = NULL; *_must_dealloc = falsE;
  news_resource2path(resource, &path, &has_at);
  if ( (path != NULL) && (!has_at) )
  { const char* temp = my_strchr(path, '/');
    if (temp == NULL) *_group = path; /* all group, no message number */
    else if (temp == path) { /* no group given, bad URI - CHECKME! */ }
    else { *_group = my_strndup(path, temp - path); *_must_dealloc = truE; }
  }
}

static void news_resource2article(const tResource* resource,
  /*@out@*/ const char** _article, /*@out@*/ tBoolean* _is_numerical)
{ const char* path;
  tBoolean has_at;
  *_article = NULL; *_is_numerical = falsE;
  news_resource2path(resource, &path, &has_at);
  if (path != NULL)
  { if (has_at) { *_article = path; /* *_is_numerical = falsE; */ }
    else
    { const char* temp = my_strchr(path, '/');
      if ( (temp != NULL) && (temp[1] == '\0') ) /* article string is empty */
        temp = NULL;
      if (temp != NULL) { *_article = temp + 1; *_is_numerical = truE; }
    }
  }
}

static tBoolean news_fetch_next_header(tResource* resource)
/* returns whether there actually is a "next" article header to be fetched */
{ tNntpRpsd* rpsd = (tNntpRpsd*) resource->rpsd;
  tBoolean retval;
  tNewsArticleNumber currnum = --(rpsd->curr_artnum);
  if (currnum >= rpsd->end_artnum)
  { char* spfbuf;
    my_spf(NULL, 0, &spfbuf, "HEAD %d\r\n", currnum);
    conn_set_writestr(resource->cconn, my_spf_use(spfbuf));
    retval = truE;
  }
  else retval = falsE;
  return(retval);
}

#define is_news_whitespace(ch) ( ((ch) == ' ') || ((ch) == '\t') )
  /* (RFC2822, 2.2.2) */

static one_caller unsigned char news_handshake(tResource* resource,
  const char** _valueptr)
/* return value: 0=error, 1=continue reading, 2=start writing, 3=done */
{ tResourceCommandHandshake handshake = resource->handshake;
  tServerStatusCode ssc = resource->server_status_code;
  const char *valueptr = *_valueptr, *group, *article;
  char* spfbuf;
  tConnection* conn = resource->cconn;
  tBoolean is_numerical, must_dealloc;
  switch (handshake)
  { case rchConnected:
      if (ssc == 200) /* "server ready, posting allowed" */
      { prepare_mode_reader:
        if (valueptr != NULL)
        { while (is_news_whitespace(*valueptr)) valueptr++;
          conn_set_software_id(conn, valueptr);
        }
        conn_set_dissolver(conn, quitcmd_dissolver);
        handshake = rchNntpModeReader;
        spfbuf = my_strdup("MODE READER\r\n"); /* be polite (RFC2980, 2.3) */
        switch_to_writing:
        conn_set_writestr(conn, my_spf_use(spfbuf));
        start_writing: resource->handshake = handshake;
        do_start_writing: return(2);
      }
      else if (ssc == 201) /* "server ready, no posting allowed" */
      { tSockaddrPortProtInfo* sppi = conn2sppi(conn, truE);
        if (sppi != NULL) sppi->sppif |= sppifNntpCannotPostArticles;
        goto prepare_mode_reader;
      }
      break;
    case rchNntpModeReader:
      /* Don't care about the status code here. If the server didn't
         understand "MODE READER", that's solely the server's problem. */
      news_resource2article(resource, &article, &is_numerical);
      if ( (article != NULL) && (!is_numerical) )
      { /* fetch the given article (header and body) */
        prepare_fetch_article:
        handshake = rchNntpFetchArticle;
        my_spf(NULL, 0, &spfbuf, "ARTICLE %s%s%s\r\n", (is_numerical ? strEmpty
          : strLt), article, (is_numerical ? strEmpty : strGt));
      }
      else
      { news_resource2group(resource, &group, &must_dealloc);
        if (group == NULL)
        { handshake = rchNntpGetGroups; spfbuf = my_strdup("LIST\r\n"); }
        else
        { /* must select a group first, in order to prepare further actions */
          handshake = rchNntpSelectGroup;
          my_spf(NULL, 0, &spfbuf, "GROUP %s\r\n", group);
        }
        if (must_dealloc) memory_deallocate(group);
      }
      goto switch_to_writing; /*@notreached@*/ break;
    case rchNntpGetGroups:
      if (ssc == 215) /* it worked, list of groups follows */
      { const char* hostname = resource2textual_host(resource)->hostname;
        char* spfbuf2;
        resource->cantent->kind = rckHtml;
        my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("News groups on %s"), hostname);
        my_spf(strbuf2, STRBUF_SIZE, &spfbuf2,
          _("List of available news groups on the server %s"), hostname);
        resource_collect_title2(resource, spfbuf, spfbuf2);
        my_spf_cleanup(strbuf, spfbuf);
        my_spf_cleanup(strbuf2, spfbuf2);
        continue_reading: ((tNntpRpsd*)(resource->rpsd))->used = 0; return(1);
      }
      break;
    case rchNntpFetchArticle:
      if (ssc == 220) goto continue_reading; /* it worked, article follows */
      break;
    case rchNntpFetchHeader:
      if (ssc == 221) goto continue_reading; /* it worked, header follows */
      else if ( (((tNntpRpsd*)(resource->rpsd))->flags & nrfHeaderSeries) &&
                ( (ssc == 423) || (ssc == 430) ) )
      { /* This specific article in a series doesn't exist, but further
           articles might exist. */
        if (news_fetch_next_header(resource)) goto do_start_writing;
        else goto completed;
      }
      break;
    case rchNntpSelectGroup:
      if (ssc == 211) /* selecting the group worked */
      { tNewsArticleNumber estnum, firstnum, lastnum;
        tBoolean need_valuedata;
        news_resource2group(resource, &group, &must_dealloc);
        news_resource2article(resource, &article, &is_numerical);
        need_valuedata = cond2boolean( (article == NULL) || (!is_numerical) );
          /* (The "!is_numerical" is a "should not happen" test here.) */
        if (need_valuedata)
        { /* interpret/store server data */
          if (valueptr == NULL)
          { bad_valueptr: if (must_dealloc) memory_deallocate(group);
            bad_news: resource_set_error(resource, reResponse);
            found_error: return(0);
          }
          while (*valueptr == ' ') { valueptr++; }
          { PREPARE_SSCANF_FORMAT(format, 20, "%d %d %d ")
            if (sscanf(valueptr, format, &estnum, &firstnum, &lastnum) != 3)
              goto bad_valueptr;
          }
          *_valueptr = NULL;
          if (group != NULL) /* "should" be true */
          { tNewsGroupInformation* ngi =
              news_ngi_lookup(resource2textual_host(resource), group, truE);
            ngi->estnum = estnum; ngi->firstnum = firstnum;
            ngi->lastnum = lastnum;
          }
        }
        /* prepare next command */
        if (need_valuedata)
        { const char *groupstr, *hostname;
          char* spfbuf2;
          tNntpRpsd* rpsd;
          prepare_article_index:
          groupstr = ((group != NULL) ? group : _(strUnknown));
          hostname = resource2textual_host(resource)->hostname;
          resource->cantent->kind = rckHtml;
          my_spf(strbuf, STRBUF_SIZE, &spfbuf,
            _("News group %s, articles %d-%d, server %s"), groupstr, firstnum,
            lastnum, hostname);
          my_spf(strbuf2, STRBUF_SIZE, &spfbuf2, _("Index of news articles %d-%d in the group %s on the server %s, latest first"),
            firstnum, lastnum, groupstr, hostname);
          resource_collect_title2(resource, spfbuf, spfbuf2);
          my_spf_cleanup(strbuf, spfbuf);
          my_spf_cleanup(strbuf2, spfbuf2);
          rpsd = (tNntpRpsd*) resource->rpsd; rpsd->curr_artnum = lastnum + 1;
          rpsd->end_artnum = firstnum; rpsd->flags |= nrfHeaderSeries;
          handshake = rchNntpFetchHeader;
          if (must_dealloc) memory_deallocate(group);
          if (news_fetch_next_header(resource)) goto start_writing;
          else { completed: return(3); }
        }
        else
        { const char* temp;
          if ( (article != NULL) && (is_numerical) && (my_isdigit(*article)) &&
               ( (temp = my_strchr(article, '-')) != NULL ) &&
               (my_isdigit(temp[1])) )
          { /* roughly looks like a numerical range of the form "37-42" */
            my_atoi(article, &firstnum, &temp, MY_ATOI_INT_MAX);
            if (*temp == '-')
            { my_atoi(temp + 1, &lastnum, &temp, MY_ATOI_INT_MAX);
              if (*temp == '\0') goto prepare_article_index; /* it is */
            }
          }
          if (must_dealloc) memory_deallocate(group);
          goto prepare_fetch_article;
        }
        /* currently: */ /*@notreached@*/
        goto switch_to_writing;
      }
      break;
    default:
      resource_set_error(resource, reHandshake); goto found_error;
      /*@notreached@*/ break;
  }
  goto bad_news;
}

static const char* __news_lookup_header_line(const char** lines,
  size_t linenum, const char* which)
{ const char* retval = NULL;
  size_t len = strlen(which), count;
  for (count = 0; count < linenum; count++)
  { const char* line = lines[count];
    if (strneqcase(line, which, len))
    { const char* temp = line + len;
      while (is_news_whitespace(*temp)) temp++;
      if (*temp == ':')
      { temp++;
        while (is_news_whitespace(*temp)) temp++;
        if (*temp != '\0') retval = temp;
        break;
      }
    }
  }
  return(retval);
}

#define news_lookup_header_line(which) \
  __news_lookup_header_line(lines, linenum, which)

#define store_header_line \
  if (linenum >= maxlinenum) \
  { maxlinenum += 20; \
    lines = memory_reallocate(lines, maxlinenum * sizeof(char*), mapOther); \
  } \
  lines[linenum++] = linestart;

static one_caller void news_handle_article_index(tResource* resource)
/* adds one line to an article index document */
{ tNntpRpsd* rpsd = (tNntpRpsd*) resource->rpsd;
  char ch, *text, *temp, *linestart; /* article header text */
  const char *subject, *htmlsubject, *sender, *htmlsender, *date, *htmldate,
    *bodylinenum, *htmlbodylinenum, *bodylinenote, *group;
  size_t used, linenum, maxlinenum;
  const char** lines;
  char* spfbuf;
  tBoolean near_r, must_dealloc_group;

  if (rpsd == NULL) return; /* "should not happen" */
  text = rpsd->text; used = rpsd->used;
  if ( (text == NULL) || (*text == '\0') || (used <= 0) ) /* no text */
    goto out;

  /* split the header into lines */

  linestart = temp = text; near_r = falsE;
  linenum = maxlinenum = 0; lines = NULL;
  while ( (ch = *temp++) != '\0' )
  { if (ch == '\r') near_r = truE;
    else
    { if ( (ch == '\n') && (near_r) )
      { /* IMPLEMENTME: strip comments (RFC2822, 3.2.3) */
        if (is_news_whitespace(*temp)) /* folded; unfold (RFC2822, 2.2.3) */
        { *(temp - 2) = *(temp - 1) = ' '; /* overwrites the CRLF */
        }
        else
        { *(temp - 2) = '\0'; /* overwrites the "\r" */
          store_header_line
          linestart = temp;
        }
      }
      near_r = falsE;
    }
  }
  store_header_line

  /* interpret the header lines */

  subject = news_lookup_header_line("subject");
  if (subject == NULL) subject = _("(no subject)");
  sender = news_lookup_header_line("from");
  if (sender == NULL)
    sender = news_lookup_header_line("sender"); /* better than nothing */
  date = news_lookup_header_line("date");
  bodylinenum = news_lookup_header_line("lines");
  if (bodylinenum == NULL) bodylinenote = strEmpty;
  else if (!strcmp(bodylinenum, "1")) bodylinenote = _(" line");
  else bodylinenote = _(" lines");

  /* build the HTML line */

  news_resource2group(resource, &group, &must_dealloc_group);
  htmlsubject = htmlify(subject);

#define hify(orig, ified) ified = ( (orig != NULL) ? htmlify(orig) : NULL )
  hify(sender, htmlsender); hify(date, htmldate);
  hify(bodylinenum, htmlbodylinenum);
#undef hify

#define appt(what) \
  ( (what != NULL) ? strSpacedDash : strEmpty ), \
  ( (what != NULL) ? what : strEmpty )
  my_spf(strbuf, STRBUF_SIZE, &spfbuf,
    "<br><a href=\"news://%s/%s/%d\">%s</a>%s%s%s%s%s%s%s\n",
    resource2textual_host(resource)->hostname,
    ( (group != NULL) ? group : "BUG" ),
    ((tNntpRpsd*) (resource->rpsd))->curr_artnum, htmlsubject,
    appt(htmlsender), appt(htmldate), appt(htmlbodylinenum), bodylinenote);
#undef appt

  resource_collect_str(resource, spfbuf);
  my_spf_cleanup(strbuf, spfbuf);

#define hc(orig, ified) if (orig != NULL) htmlify_cleanup(orig, ified)
  hc(subject, htmlsubject); hc(sender, htmlsender); hc(date, htmldate);
  hc(bodylinenum, htmlbodylinenum);
#undef hc

  if (must_dealloc_group) memory_deallocate(group);
  memory_deallocate(lines);
  out:
  rpsd->used = 0;
}

#undef store_header_line
#undef news_lookup_header_line

static one_caller void news_handle_groupline(tResource* resource, char* text)
/* adds one line to a groups index document; the <text> should be of the form
   "group last first p" (RFC977, 3.6.1) */
{ char *pos = my_strchr(text, ' '), *spfbuf;
  const char *info, *name, *htmlname;
  if (pos == NULL) info = NULL; /* server bug */
  else { *pos++ = '\0'; info = pos; }
  name = text; htmlname = htmlify(name);
  my_spf(strbuf, STRBUF_SIZE, &spfbuf,
    "<br><a href=\"news://%s/%s\">%s</a>%s%s\n",
    resource2textual_host(resource)->hostname, name, htmlname,
    ( (info != NULL) ? strSpace : strEmpty ), null2empty(info));
  resource_collect_str(resource, spfbuf);
  my_spf_cleanup(strbuf, spfbuf);
  htmlify_cleanup(name, htmlname);
}

static void news_collect(tResource* resource, const char* src, size_t size)
{ tResourceCommandHandshake handshake = resource->handshake;
  tNntpRpsd* rpsd = (tNntpRpsd*) resource->rpsd;
  if ( (handshake == rchNntpGetGroups) ||
       ((handshake == rchNntpFetchHeader) && (rpsd->flags & nrfHeaderSeries)) )
  {
    char* text;
    size_t used, usable, needed;
    if (rpsd == NULL) goto res_coll; /* "should not happen" */
    text = rpsd->text; used = rpsd->used; usable = rpsd->usable;
    needed = zero2one(used) + size;
    if (needed > usable)
    { text = rpsd->text = memory_reallocate(text, needed, mapOther);
      rpsd->usable = needed;
    }
    my_memcpy(text + used - cond2bool(used > 0), src, size);
    text[needed - 1] = '\0';
    rpsd->used = needed;
    if (handshake == rchNntpGetGroups)
    { /* look for "\r\n" */
      char *temp = text, *remainder = text;
      tBoolean near_r = falsE;
      char ch;
      while ( (ch = *temp++) != '\0' )
      { if (ch == '\r') near_r = truE;
        else
        { if ( (ch == '\n') && (near_r) )
          { size_t len = (temp - remainder) - 2; /* "-2" for the CRLF */
            if (len > 0) /* actually have useful text */
            { remainder[len] = '\0'; /* overwrites the "\r" */
              news_handle_groupline(resource, remainder);
            }
            remainder = temp;
          }
          near_r = falsE;
        }
      }
      if (remainder > text) /* handled some grouplines */
      { if (*remainder == '\0') rpsd->used = 0; /* handled whole text */
        else
        { char* dest = text;
          rpsd->used = rpsd->used - (remainder - text);
          do { ch = *remainder++; *dest++ = ch; } while (ch != '\0');
        }
        push_to_main_res(resource, 1);
      }
    }
  }
  else { res_coll: resource_collect(resource, src, size); }
}

#define news_read_flush \
  { if (read_buf_used > 0) \
    { news_collect(resource, read_buf, read_buf_used); read_buf_used = 0; } \
  }

#define news_read_append_ch(ch) \
  do \
  { if (read_buf_used >= NEWS_READ_BUFSIZE) news_read_flush \
    read_buf[read_buf_used++] = ch; \
  } while (0)

#define news_read_append(_addr, _count) \
  do \
  { const char* a = _addr; /* (evaluate it only once) */ \
    size_t c = _count; \
    while (c-- > 0) { char x = *a++; news_read_append_ch(x); } \
  } while (0)

static one_caller void news_read(tResource* resource)
/* We treat NNTP connections in a special way because we must filter out the
   control data and construct/store the actual content data. */
{ static const char strDotMarker[] = "\r\n.\r\n";
  enum { NEWS_BUFSIZE = (4096 + 1), NEWS_READ_BUFSIZE = (2048 - 5) };
  char buf[NEWS_BUFSIZE], *ptr; /* temporary raw data buffer */
  const char* valueptr; /* NULL or inside buf[] */
  char read_buf[NEWS_READ_BUFSIZE + 5]; /* temporary content buffer */
  tTlHeaderState ths = resource->tlheaderstate;
  tResourceError news_error = reServerClosed;
  int fd = resource->cconn->fd, err = my_read_sock(fd, buf, NEWS_BUFSIZE - 1);
  size_t count, read_buf_used = 0;
  tBoolean do_push = falsE;

  if (err <= 0) /* error (EOF is also an error here because it's unexpected) */
  { stop_reading:
    resource_set_error(resource, news_error);
    do_stop_reading:
    conn_remove(&(resource->cconn));
    resource->tlheaderstate = ths; /* CHECKME: set "= 0" instead? */
    news_read_flush
    rpsd_remove(resource);
    push_to_main_res(resource, 0);
    return;
  }

  buf[err] = '\0'; /* for simplicity (e.g. sscanf()) */
#if CONFIG_DEBUG
  debugmsg("news_read(): "); debugmsg(buf);
  if (buf[err - 1] != '\n') debugmsg(strNewline);
#endif
  ptr = buf; valueptr = NULL; count = err;
  while (count-- > 0)
  { const char ch = *ptr++;
    switch (ths)
    { case 0:
        resource->server_status_code = 0; /*@fallthrough@*/
      case 1: case 2: /* parsing server status code at message start */
        if (my_isdigit(ch))
        { tServerStatusCode status = resource->server_status_code;
          status = 10 * status + ((tServerStatusCode) (ch - '0'));
          resource->server_status_code = status;
          ths++;
          if (ths == 3)
          { if ( (status < 100) || (status > 599) )
            { bad_news: news_error = reResponse; goto stop_reading; }
            valueptr = ptr;
          }
        }
        else goto bad_news;
        break;
      case 3: /* somewhere in a status response line */
        if (ch == '\r') ths = 4; /* possibly at end of line */
        break;
      case 4:
        if (ch == '\r') { /* don't change ths */ }
        else if (ch != '\n') ths = 3; /* not actually at end of line */
        else /* at end of line; look what to do */
        { ths = 0;
          switch (news_handshake(resource, &valueptr))
          { case 0: goto bad_news; /*@notreached@*/ break;
            case 1: /* read some "content" */
              resource->state = rsReading; ths = 7; break;
            case 2:
              start_writing: conn_set_write(resource->cconn);
              goto out; /*@notreached@*/ break;
            case 3:
              resource_complete:
              resource->state = rsComplete; goto do_stop_reading;
              /*@notreached@*/ break;
          }
          valueptr = NULL;
        }
        break;
      case 5: case 6: case 7: case 8: case 9: /* "content" */
        if (ch == strDotMarker[ths - 5]) /* looks like the end marker */
        { ths++;
          if (ths == 10) /* _is_ a _complete_ end marker */
          { ths = 0; news_read_flush
            if ( (resource->handshake == rchNntpFetchHeader) &&
                 (((tNntpRpsd*)(resource->rpsd))->flags & nrfHeaderSeries) )
            { news_handle_article_index(resource);
              if (news_fetch_next_header(resource))
                goto start_writing; /* get the next article header */
            }
            goto resource_complete;
          }
        }
        else if (ths > 5)
        { /* It wasn't actually the end marker, it only looked like the
             beginning of the marker. */
          if ( (ths == 8) && (ch == '.') )
          { /* collapse double-dot to single-dot; how dotty... */
            ths = 7; /* FIXME: "=5" if there wasn't any actual text! */
          }
          news_read_append(strDotMarker, ths - 5);
          if (ch == '\r') ths = 6; /* possibly beginning of end marker */
          else { ths = 5; goto append_ch; } /* back to "normal content" mode */
        }
        else /* "normal content" */
        { append_ch:
          do_push = truE;
          news_read_append_ch(ch);
          if ( (ch == '\n') && (resource->cantent->kind == rckHtml) )
            news_read_append(strBr, 4); /* FIXME: do this in ...append_ch()! */
        }
        break;
    } /* switch (ths) */
  } /* while count */
  out:
  resource->tlheaderstate = ths;
  news_read_flush
  if (do_push) push_to_main_res(resource, 1);
}

static void news_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  switch (ccek)
  { case ccekConnectSetup:
      if (conn->flags & cnfConnected) goto do_setup;
      else { conn_set_write(conn); push_to_main_res(resource, 1); }
      break;
    case ccekConnectWorked:
      do_setup:
      resource->state = rsMsgExchange; resource->handshake = rchConnected;
      start_reading:
      conn_set_read(conn); /* to get the "server ready" message */
      news_rpsd_prepare(resource); push_to_main_res(resource, 1);
      break;
    case ccekConnectFailed:
      resource_set_error(resource, conn_get_failre(conn));
      stopnpush: conn_remove(&(resource->cconn)); push_to_main_res(resource,0);
      break;
    case ccekRead: news_read(resource); break;
    case ccekWrite:
      switch (conn_write_writedata(conn))
      { case 0:
          resource_set_error(resource, reServerClosed); goto stopnpush;
          /*@notreached@*/ break;
        case 1: break; /* keep writing */
        case 2: goto start_reading; /*@notreached@*/ break;
      }
      break;
    default: conn_bug(conn, ccek); break;
  }
}

#endif /* #if OPTION_NEWS */


/** Non-local resources: finger */

#if CONFIG_FINGER

static one_caller void finger_read(tResource* resource)
{ int fd = resource->cconn->fd;
  char* dest;
  size_t size;
  int err;
  resource_provide_room(resource, &dest, &size, resource->bytecount, 2);
  err = my_read_sock(fd, dest, size);
  if (err <= 0) /* error or EOF */
  { conn_remove(&(resource->cconn));
    if (err == 0) resource->state = rsComplete;
    else resource_set_error(resource, reServerClosed);
    push_to_main_res(resource, 0);
  }
  else
  { resource_record(resource, dest, err);
    push_to_main_res(resource, 1);
  }
}

static void finger_callback(tConnection* conn, tConnCbEventKind ccek)
{ tResource* resource = (tResource*) (conn->data);
  const char* username;
  switch (ccek)
  { case ccekConnectSetup:
      conn_set_write(conn);
      if (conn->flags & cnfConnected) goto set_req;
      else push_to_main_res(resource, 1); /* say "Connecting to..." */
      break;
    case ccekConnectWorked:
      set_req: resource->state = rsMsgExchange;
      username = resource->uri_data->username;
      if (username == NULL) conn_set_writestr(conn, my_strdup(strCrlf));
      else
      { char* spfbuf;
        my_spf(NULL, 0, &spfbuf, "%s\r\n", username);
        conn_set_writestr(conn, my_spf_use(spfbuf));
      }
      push_to_main_res(resource, 1);
      break;
    case ccekConnectFailed:
      resource_set_error(resource, conn_get_failre(conn));
      stopnpush: conn_remove(&(resource->cconn)); push_to_main_res(resource,0);
      break;
    case ccekRead: finger_read(resource); break;
    case ccekWrite:
      switch (conn_write_writedata(conn))
      { case 0:
          resource_set_error(resource, reServerClosed); goto stopnpush;
          /*@notreached@*/ break;
        case 1: break; /* keep writing */
        case 2: /* all written, start reading */
          conn_set_read(conn); resource->state = rsReading;
          push_to_main_res(resource, 1); break;
      }
      break;
    default: conn_bug(conn, ccek); break;
  }
}

#endif /* #if CONFIG_FINGER */


/** Non-local resources: General */

static tBoolean start_request(tResourceRequest* request,
  tCachedHostInformation* hostinfo)
/* starts (prepares) a network request; returns want-it-back info */
{ const tUriData* uri_data = request->uri_data;
  const tResourceProtocol protocol = uri_data->rp;
  const tConfigProxy* proxy = request->proxy;
  const tResourceRequestFlags rrf = request->flags;
  tResource* resource;
  tPortnumber connport, textual_portnumber;
  tConnection* conn;
  tConnCbFunc cb;

  switch (protocol)
  { case rpHttp:
#if OPTION_TLS
    case rpHttps:
#endif
      cb = http_callback; break;
#if CONFIG_FTP
    case rpFtp:
#if OPTION_TLS
    case rpFtps:
#endif
      cb = ftp_control_callback; break;
#endif
#if OPTION_NEWS
    case rpNntp: cb = news_callback; break;
#endif
#if CONFIG_FINGER
    case rpFinger: cb = finger_callback; break;
#endif
#if CONFIG_GOPHER
    case rpGopher: cb = gopher_callback; break;
#endif
    default: /* "should not happen" */
      resource_request_set_error(request, reProtocol);
      failed: return(falsE); /*@notreached@*/ break;
  }

  textual_portnumber = request->uri_data->portnumber;
  if ( (proxy != NULL) && (proxy->proxy_hostname != NULL) )
    connport = proxy->proxy_portnumber;
  else connport = textual_portnumber;

  resource = resource_create(request, NULL, UNKNOWN_CONTENTLENGTH, rckUnknown,
    rsConnecting);
  resource->proxy = proxy;
  resource->actual_hppi = hppi_lookup(hostinfo, connport, protocol, truE);

  { const char* hostname = request->uri_data->hostname;
    tCachedHostInformation* textual_host = hostinfo_lookup(hostname);
    if (textual_host == NULL) textual_host = hostinfo_create(hostname);
    resource->textual_hppi = hppi_lookup(textual_host, textual_portnumber,
      protocol, truE);
  }

  if (rrf & rrfPost) resource->flags |= rfPost;
  if (rrf & rrfIsRedirection) resource->flags |= rfIsRedirection;
  if (rrf & rrfIsEmbedded) resource->flags |= rfIsEmbedded;
  if (request->action == rraEnforcedReload) resource->flags |= rfIsEnforced;

  conn = resource->cconn = conn_create(-1, cb, resource, truE,
    resource->actual_hppi, protocol);
  if (conn_connect(conn)) conn_callback(conn, ccekConnectSetup);
  else
  { tResourceError re = conn->prelire;
    resource_request_set_error(request, re ? re : reConnectionFailureDefault);
    goto failed;
  }
  return(truE);
}


/** Resource handling basics/interface */

#if OPTION_THREADING

static void request_dns_callback(void*, tDhmNotificationFlags); /* prototype */

static void request_dns_vanisher(void* _request, tDhmNotificationFlags flags)
{ if (flags & dhmnfRemoval) /* "should" be true */
  { /* The request goes away, so it won't be interested in DNS results. */
    tResourceRequest* request = (tResourceRequest*) _request;
    tCachedHostInformation* hostinfo = request->lookup;
    if (hostinfo != NULL) /* "should" be true */
      dhm_notification_off(hostinfo, request_dns_callback, request);
  }
}

static void request_dns_callback(void* _request, tDhmNotificationFlags flags)
{ if (flags & dhmnfOnce) /* "should" be true */
  { tResourceRequest* request = (tResourceRequest*) _request;
    tCachedHostInformation* hostinfo = request->lookup;
    tBoolean want_it_back;
    dhm_notification_off(request, request_dns_vanisher, request);
    if (hostinfo == NULL) /* "should not happen" */
    { dns_failed: resource_request_set_error(request, reDns);
      want_it_back = falsE; goto out_once;
    }
    request->lookup = NULL;
    if (hostinfo->num_sockaddrs <= 0) goto dns_failed;
    want_it_back = start_request(request, hostinfo);
    out_once:
    push_to_main_req(request, boolean2bool(want_it_back));
  }
}

static void resource_dns_handler(__sunused void* data __cunused,
  __sunused tFdObservationFlags flags __cunused)
/* The DNS thread finished a lookup. */
{ static tDnsLookup* dns_lookup;
  static unsigned char count = 0;
  tCachedHostInformation* hostinfo;
  int err = my_read_pipe(fd_dns2resource_read, ((char*)(&dns_lookup)) + count,
    sizeof(dns_lookup) - count);
  if (err <= 0) return;
  count += err;
  if (count < sizeof(dns_lookup)) return; /* not yet a whole pointer */
  count = 0; /* for the next round */
  hostinfo = dns_lookup->hostinfo; hostinfo->flags &=~chifAddressLookupRunning;
  postprocess_dns_lookup(dns_lookup); dhm_notify(hostinfo, dhmnfOnce);
}

#endif /* #if OPTION_THREADING */

void resource_request_start(tResourceRequest* request)
{ tBoolean want_it_back = falsE;
  tResourceProtocol rp = request->uri_data->rp;
  switch (rp)
  { case rpLocal: fetch_local(request); break;
    case rpAbout: fetch_about(request); break;
#if OPTION_LOCAL_CGI
    case rpLocalCgi: want_it_back = start_local_cgi(request); break;
#endif
    default:
      if (resource_lookup(request) != NULL) { /* found in cache, done */ }
#if OPTION_POP
      else if (is_poplike(rp)) /* e-mail access is special */
        want_it_back = pop_fetch(request);
#endif
      else /* some network activity might be necessary :-) */
      { tCachedHostInformation* hostinfo;
        const tResourceRequestAction action = request->action;
        const tHostAddressLookupFlags half = ( (action == rraEnforcedReload)
#if CONFIG_CUSTOM_CONN
            || (action == rraCustomConn)
#endif
          ) ? halfEnforcedReload : halfNone;
        const tHostAddressLookupResult res = lookup_hostaddress(request,
          &hostinfo, half);
        if (res == halrFine) want_it_back = start_request(request, hostinfo);
#if CONFIG_ASYNC_DNS
        else if (res == halrLookupRunning)
        { request->state = rrsDnsLookup; request->lookup = hostinfo;
          dhm_notification_setup(hostinfo, request_dns_callback, request,
            dhmnfOnce, dhmnSet);
          dhm_notification_setup(request, request_dns_vanisher, request,
            dhmnfRemoval, dhmnSet);
          want_it_back = truE;
        }
#endif
        else resource_request_set_error(request, reDns);
      }
      break;
  }
  push_to_main_req(request, boolean2bool(want_it_back));
}

#if CONFIG_ASYNC_DNS
static void stop_request_dns_lookup(tResourceRequest* request)
{ if (request->state == rrsDnsLookup)
  { tCachedHostInformation* hostinfo = request->lookup;
    if (hostinfo != NULL) /* "should" be true */
    { dhm_notification_off(hostinfo, request_dns_callback, request);
      dhm_notification_off(request, request_dns_vanisher, request);
      request->lookup = NULL;
    }
    request->state = rrsStopped;
  }
}
#else
#define stop_request_dns_lookup(request) do { } while (0)
#endif

void resource_request_stop(tResourceRequest* request)
{ tResource* resource = request->resource;
  stop_request_dns_lookup(request);
  if (resource != NULL) resource_stop(resource);
  push_to_main_req(request, 0);
}

#if CONFIG_CUSTOM_CONN

void resource_custom_conn_start(tResource* resource, unsigned char what,
  const void* whatever)
{ const char* cmd;
  unsigned char x;
  tResourceError ftp_error;
  tResourceCommandHandshake handshake;
#if OPTION_TLS
  tFtpTlsMethod ftm;
#endif
  switch (what)
  { case 0:
      resource->handshake = rchFtpCustom; cmd = (const char*) whatever;
      ftp_control_do_start_command(resource, my_strdup(cmd)); break;
    case 1:
      resource->handshake = handshake = MY_POINTER_TO_INT(whatever);
      try_1:
      x = ftp_control_start_command(resource, &ftp_error);
      /* CHECKME: handle errors; at least unbusify! */
      if (0) { handle_ftp_error: {} }
      break;
#if OPTION_TLS
    case 2:
      ftm = MY_POINTER_TO_INT(whatever);
      if (ftm == ftmAutodetect) ftm = calc_ftp_tls_method(resource);
      switch (ftm)
      { case ftmAutodetect: case ftmAuthTls: case ftmAuthTlsDataclear:
          handshake = rchFtpTlsAuthTls; break;
        case ftmAuthSsl: handshake = rchFtpTlsAuthSsl; break;
        default: ftp_error = reHandshake; goto handle_ftp_error;
          /*@notreached@*/ break;
      }
      resource->handshake = handshake; ftps_ftm(resource) = ftm;
      goto try_1; /*@notreached@*/ break;
#endif
  }
}

#endif


/** Initialization */

#ifndef IPPORT_HTTP
#define IPPORT_HTTP 80
#endif
#ifndef IPPORT_FTP
#define IPPORT_FTP 21
#endif
#ifndef IPPORT_FINGER
#define IPPORT_FINGER 79
#endif
#ifndef IPPORT_GOPHER
#define IPPORT_GOPHER 70
#endif
#ifndef IPPORT_NNTP
#define IPPORT_NNTP 119
#endif
#ifndef IPPORT_POP3
#define IPPORT_POP3 110
#endif
#ifndef IPPORT_HTTPS
#define IPPORT_HTTPS 443
#endif

static tPortnumber __init __calculate_portnumber(const char* name,
  tPortnumber defaultport)
{
#if CONFIG_PLATFORM != 1
  const struct servent* ent = getservbyname(name, strTcp);
  if (ent != NULL) return((tPortnumber) ent->s_port);
  else
#endif
  { return((tPortnumber) htons(defaultport)); }
}

#define calculate_portnumber(rp, defaultport) \
  __calculate_portnumber(rp_data[rp].scheme, defaultport)

static void __init setup_pipe(/*@out@*/ int* reader, /*@out@*/ int* writer)
{ int fd_pair[2];
  unsigned char count;
  if (my_pipe(fd_pair) != 0) fatal_error(errno, _(strResourceError[rePipe]));
  for (count = 0; count <= 1; count++)
  { int fd = fd_pair[count];
    if (!fd_is_observable(fd)) fatal_tmofd(fd);
    make_fd_cloexec(fd);
  }
  *reader = fd_pair[0]; *writer = fd_pair[1];
}

#if OPTION_THREADING

static one_caller void does_not_return __init fatal_threading(const int err)
/* fun with annotations :-) */
{ fatal_error(err, _("can't create thread"));
}

#endif

#if OPTION_THREADING == 1

static void __init create_thread(void* (*fn)(void* dummy), void* data)
{ pthread_t dummy_id;
  pthread_attr_t thread_attr;
  int err;
  if ( ( (err = pthread_attr_init(&thread_attr)) != 0 ) ||
       ( (err = pthread_attr_setdetachstate(&thread_attr,
          PTHREAD_CREATE_DETACHED)) != 0 ) ||
       ( (err = pthread_create(&dummy_id, &thread_attr, fn, data)) != 0 ) )
  { fatal_threading(err);
      /* (SUSv3 says <err> is error number - <errno> not involved here) */
  }
}

#elif OPTION_THREADING == 2

static int __init create_thread(int (*fn)(void* dummy), void* data)
{ void* childstack = __memory_allocate(102400, mapPermanent);
  int err = clone(fn, childstack + 102400 - 128, CLONE_VM | CLONE_FS |
    CLONE_FILES | CLONE_SIGHAND, data);
  /* CHECKME: we don't want CLONE_SIGHAND!? */
  if (err == -1) fatal_threading(errno);
  return(err);
}

#endif /* #if OPTION_THREADING... */

#if USE_LWIP
static tBoolean lwip_timeout_handler(/*@out@*/ int* _msec)
{ *_msec = 100; return(truE); }
#endif

one_caller void __init resource_initialize(void)
/* initializes the resource handling; note that this is executed in the main
   thread - it _creates_ the other thread(s) at the end. */
{ const struct protoent* proto;
#if OPTION_THREADING
  static tDnsThreadData dns_thread_data;
#endif

  /* Get protocol and port numbers */

#if CONFIG_PLATFORM != 1
  proto = getprotobyname(strTcp);
  if (proto != NULL) ipprotocolnumber_tcp = proto->p_proto;
  /* "else": use the standard value IPPROTO_TCP */
#endif

#if CONFIG_PLATFORM != 1
  setservent(1);
#endif
  portnumber_http = calculate_portnumber(rpHttp, IPPORT_HTTP);
  portnumber_ftp = calculate_portnumber(__rpFtp, IPPORT_FTP);
  portnumber_finger = calculate_portnumber(__rpFinger, IPPORT_FINGER);
  portnumber_cvs = __calculate_portnumber("cvspserver", 2401);
  portnumber_gopher = calculate_portnumber(__rpGopher, IPPORT_GOPHER);
  portnumber_nntp = __calculate_portnumber(strNntp, IPPORT_NNTP);
  portnumber_pop3 = __calculate_portnumber(strPop3, IPPORT_POP3);
  portnumber_https = calculate_portnumber(__rpHttps, IPPORT_HTTPS);
#if CONFIG_PLATFORM != 1
  endservent();
#endif

  /* Initialize some data structures */

  my_memclr_arr(rc_head); my_memclr_arr(chi_head);
  my_memclr_arr(sockaddr_entry_head);
#if OPTION_BUILTIN_DNS
  my_memclr_var(dns_data);
#endif
#if USE_LWIP
  timeout_register(lwip_timeout_handler);
#endif

  /* Initialize cookies */

#if OPTION_COOKIES
  cookie_initialize();
#endif

  /* Setup some file descriptors */

#if OPTION_THREADING
  setup_pipe(&fd_dns2resource_read, &fd_dns2resource_write);
  setup_pipe(&fd_resource2dns_read, &fd_resource2dns_write);
  fd_observe(fd_dns2resource_read, resource_dns_handler, NULL, fdofRead);
  dns_thread_data.reader = fd_resource2dns_read;
  dns_thread_data.writer = fd_dns2resource_write;
#if NEED_FD_REGISTER
  /* the register isn't thread-safe, so we do the lookup here... */
  (void) fd_register_lookup(&(dns_thread_data.reader));
  (void) fd_register_lookup(&(dns_thread_data.writer));
#endif
#endif

#if CAN_HANDLE_SIGNALS
  setup_pipe(&fd_any2main_read, &fd_any2main_write);
#endif

#if CONFIG_TG == TG_XCURSES
  setup_pipe(&fd_xcurses2main_read, &fd_xcurses2main_write);
  if (is_environed) fd_keyboard_input = fd_xcurses2main_read;
#endif

  /* Setup the DNS thread */

#if OPTION_THREADING == 1
  create_thread(dns_handler_thread, &dns_thread_data);
#elif OPTION_THREADING == 2
  { int err;
    pid_main = getpid();
    err = create_thread(dns_handler_thread, &dns_thread_data);
    if (err > 0) pid_dns = err;
  }
#endif
}

void resource_quit(void)
{ /* IMPLEMENTME: remove all connections, so that the dissolvers can run? Only
     if no fatal error happened? Only if this doesn't need "much time", so
     users don't have to wait when quitting? Only if the dissolver is
     "important", e.g. for POP3 or TLS? ... */
#if OPTION_TLS
  tls_deinitialize();
#endif
#if OPTION_THREADING == 2
  if (pid_dns > 0) (void) kill(pid_dns, SIGKILL); /* let's be rude :-) */
#endif
}
