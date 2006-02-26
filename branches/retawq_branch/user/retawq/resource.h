/* retawq/resource.h - resource handling (network, cache, DNS, ...)
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

/* In the comments for data structure declarations in this file, "EXAC(thread)"
   meant that <thread> is the only thread which may access the marked member
   in any way ("exclusive access"). For a pointer, EXACD(thread) meant that
   only that thread may dereference the pointer. */

#ifndef __retawq_resource_h__
#define __retawq_resource_h__

#include "stuff.h"

extern /*@observer@*/ const char strRedirection[];

#if CAN_HANDLE_SIGNALS
extern const_after_init int fd_any2main_read, fd_any2main_write;
#endif
#if CONFIG_TG == TG_XCURSES
extern const_after_init int fd_xcurses2main_read, fd_xcurses2main_write;
#endif

extern const char strCrlf[];
#if CONFIG_FTP
extern const char strList[], strRetr[];
#endif


/* Resource protocols */

my_enum1 enum
{ rpUnknown = 0, rpDisabled = 1, rpHttp = 2, __rpFtp = 3, rpLocal = 4,
  rpAbout = 5, __rpFinger = 6, __rpLocalCgi = 7, __rpNntp = 8, __rpPop = 9,
  __rpPops = 10, rpCvs = 11, __rpGopher = 12, rpInfo = 13, __rpMailto = 14,
  __rpJavascript = 15, __rpHttps = 16, __rpFtps = 17, __rpExecextShell = 18
} my_enum2(unsigned char) tResourceProtocol;
#define rpMax (18)
#define is_rp_nice(rp) ((rp) > rpDisabled)
#define MAXSCHEMESTRSIZE ((100) + 1)
  /* (just a value which is "big enough", even for _(strUnknown)) */

#if CONFIG_FINGER
#define rpFinger (__rpFinger)
#endif
#if CONFIG_GOPHER
#define rpGopher (__rpGopher)
#endif
#if CONFIG_FTP
#define rpFtp (__rpFtp)
#endif
#if OPTION_LOCAL_CGI
#define rpLocalCgi (__rpLocalCgi)
#endif
#if OPTION_NEWS
#define rpNntp (__rpNntp)
#endif
#if OPTION_POP
#define rpPop (__rpPop)
#if OPTION_TLS
#define rpPops (__rpPops)
#endif
#endif
#if CONFIG_MAILTO
#define rpMailto (__rpMailto)
#endif
#if CONFIG_JAVASCRIPT
#define rpJavascript (__rpJavascript)
#endif

#if OPTION_TLS
#define rpHttps (__rpHttps)
#if CONFIG_FTP
#define rpFtps  (__rpFtps)
#endif
#endif

#if OPTION_EXECEXT & EXECEXT_SHELL
#define rpExecextShell (__rpExecextShell)
#endif

my_enum1 enum
{ rpfNone = 0, rpfIsLocallike = 0x01, rpfIsHttplike = 0x02,
  rpfIsFtplike = 0x04, rpfIsPoplike = 0x08, rpfIsTlslike = 0x10,
  rpfUsesAuthority = 0x20, rpfIsPathHierarchical = 0x40, rpfUiToupper = 0x80,
  rpfUiAlternate = 0x100
} my_enum2(unsigned short) tRpFlags;

typedef struct
{ const char* scheme;
  tResourceProtocol final_rp;
  tRpFlags flags;
} tRpData;

extern const tRpData rp_data[rpMax + 1];

#define __has_rpflag(rp, rpf) (rp_data[(rp)].flags & (rpf))
#define is_locallike(rp) __has_rpflag((rp), rpfIsLocallike)
#define is_httplike(rp) __has_rpflag((rp), rpfIsHttplike)
#if CONFIG_FTP
#define is_ftplike(rp) __has_rpflag((rp), rpfIsFtplike)
#endif
#if OPTION_POP
#define is_poplike(rp) __has_rpflag((rp), rpfIsPoplike)
#endif
#if OPTION_TLS
#define is_tlslike(rp) __has_rpflag((rp), rpfIsTlslike)
#endif


/* Resource data */

my_enum1 enum
{ rfNone = 0, rfFinal = 0x01, rfPost = 0x02, rfIsRedirection = 0x04,
  rfIsEmbedded = 0x08, rfIsEnforced = 0x10, rfActivityWatched = 0x20,
  rfActivityUnwatchable = 0x40
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  , rfDownload = 0x80
#endif
#if CONFIG_ANY_MAIL
  , rfMailbox = 0x100
#endif
#if OPTION_COOKIES
  , rfCookieSender = 0x200, rfCookieStorer = 0x400 /* RFC2965, 6.1, *2 */
#define rfCookieAnything (rfCookieSender | rfCookieStorer)
#endif
#if CONFIG_CUSTOM_CONN
  , rfCustomConn = 0x800, rfCustomConnStopSequencer = 0x1000,
  rfCustomConnBd1 = 0x2000, rfCustomConnBd2 = 0x4000, rfCustomConnBd3 = 0x8000,
  rfCustomConnBd4 = 0x10000
#define rfCustomConnBdAny (rfCustomConnBd1 | rfCustomConnBd2 | rfCustomConnBd3 | rfCustomConnBd4)
  /* "Bd" as in "Brain-damaged"; the FTP protocol seriously deserves it... */
#endif
} my_enum2(unsigned int) tResourceFlags;

my_enum1 enum
{ rsUnknown = 0, rsError = 1, rsConnecting = 2, rsMsgExchange = 3,
  rsReading = 4, rsComplete = 5, rsStopped = 6
} my_enum2(unsigned char) tResourceState;
#define MAX_RESOURCE_STATE (6)
extern /*@observer@*/ const char* const
  strResourceState[MAX_RESOURCE_STATE + 1];

my_enum1 enum
{ rchUnknown = 0, rchConnected = 1
#if CONFIG_FTP
  , rchFtpUser = 2, rchFtpPassword = 3, rchFtpAccount = 4, rchFtpPasv = 5,
  rchFtpRequest = 6
#if OPTION_TLS
  , rchFtpTlsAuthTls = 7, rchFtpTlsAuthSsl = 8, rchFtpTlsPbsz = 9,
  rchFtpTlsProt = 10
#endif
#if CONFIG_CUSTOM_CONN
  , rchFtpCustom = 11
#endif
#endif
#if OPTION_NEWS
  , rchNntpModeReader = 12, rchNntpGetGroups = 13, rchNntpFetchArticle = 14,
  rchNntpFetchHeader = 15, rchNntpSelectGroup = 16
#endif
#if OPTION_POP
  , rchPopUser = 17, rchPopPass = 18, rchPopApop = 19, rchPopStat = 20,
  rchPopTop = 21, rchPopUidl = 22, rchPopRetr = 23
#if OPTION_TLS
  , rchPopStls = 24
#endif
#endif
} my_enum2(unsigned char) tResourceCommandHandshake;

my_enum1 enum
{ reFine = 0, reKind = 1, reSocket = 2, reConnect = 3, reRefused = 4,
  reNetwork = 5, reDns = 6, reFile = 7, reUri = 8, reServerClosed = 9,
  reHandshake = 10, rePortnumber = 11, reProtocol = 12, reTimeout = 13,
  reResponse = 14, reLogin = 15, reRedirection = 16, reRandom = 17, reTls = 18,
  reProxyAuth = 19, rePipe = 20, reFork = 21, reConfigForbids = 22,
  reExec = 23, reProtDisabled = 24, reHostname = 25, reTmofd = 26,
  reConnReset = 27
} my_enum2(unsigned char) tResourceError;
#define MAX_RESOURCE_ERROR (27)
extern /*@observer@*/ const char* const
  strResourceError[MAX_RESOURCE_ERROR + 1];

#if OPTION_TLS
my_enum1 enum
{ teFine = 0, teUnknown = 1, teSessionExpired = 2, teFatalAlert = 3,
  teFile = 4, teData = 5, teOom = 6, teCipher = 7, teCompression = 8,
  teCrypt = 9, teSrp = 10, tePk = 11, tePacket = 12, teUnimplemented = 13,
  teHandshake = 14, teInit = 15
} my_enum2(unsigned char) tTlsError;
#define MAX_TLS_ERROR (15)
extern /*@observer@*/ const char* const strTlsError[MAX_TLS_ERROR + 1];
#define is_tls_error_expressive(te) ((te) > teUnknown) /* for UI */
#endif

my_enum1 enum
{ udfNone = 0, udfTryIpv6 = 0x01, udfGotExplicitPortnumber = 0x02
#if CONFIG_FTP
  , udfFtpTypeA = 0x04, udfFtpTypeD = 0x08, udfFtpTypeI = 0x10
#endif
} my_enum2(unsigned char) tUriDataFlags;

typedef struct
{ char *uri, *hostname, *path, *query, *post, *username, *password;
  tPortnumber portnumber; /* (in network byte order) */
  unsigned char refcount; /* 0..2 */
  tResourceProtocol rp;
  tResourceError re;
  tUriDataFlags udf;
} tUriData;

typedef struct tSaveAs
{ struct tSaveAs* next;
  int fd;
} tSaveAs;

typedef unsigned short tServerStatusCode; /* 0 or roughly 100..699 */
typedef signed char tTlHeaderState; /*transport-level header state (misnomer)*/

#define UNKNOWN_CONTENTLENGTH ((size_t) (~((size_t) 0)))

#if 1 || CONFIG_FTP || OPTION_NEWS || OPTION_POP || OPTION_TRAP
/* (the "1 ||" is for HTTP nowadays) */
#define USE_RPSD 1
struct tRpsdGeneric; /* resource-protocol-specific data */
#else
#define USE_RPSD 0
#endif

typedef struct tTransferData
{ struct tTransferData* next;
  const char* name;
  char* data;
  size_t size;
  tBoolean need_unmap;
} tTransferData;

my_enum1 enum
{ sdfNone = 0, sdfIsDownloadFdValid = 0x01, sdfIsRestartMarkerValid = 0x02
} my_enum2(unsigned char) tSinkingDataFlags;

typedef struct
{ tTransferData* transfer_data;
  size_t restart_marker;
  int download_fd;
  tSinkingDataFlags flags;
} tSinkingData;
/* data which "sinks" from the high-level user interface over tResourceRequest
   to the low-level networking code */

struct tConnection;

typedef struct tResource
{ struct tResource *next, *prev; /* for the RAM cache; EXAC(resource) */
  struct tConnection *cconn, *dconn; /* control/data; EXAC(resource) */
  tCantent* cantent;
  tUriData* uri_data;
  tDhmGenericData* dhm_data;
  const tConfigProxy* proxy;
  struct tHostPortProtInfo *actual_hppi, *textual_hppi; /* EXAC(resource) */
    /* (might differ, e.g. when proxies are used; textual_hppi always holds
        stuff which originates from the URI) */
  tSinkingData* sinking_data;
  tSaveAs* save_as; /* for saving to disk; EXAC(resource) */
#if USE_RPSD
  struct tRpsdGeneric* rpsd; /* EXAC(resource) */
#endif
#if CONFIG_CUSTOM_CONN
  void* custconn_handle; /* tConsoleTaskNum or tWindow* */
#endif
  size_t nominal_contentlength;
  size_t bytecount; /* roughly "number of bytes in document" */
  tResourceFlags flags;
  tServerStatusCode server_status_code; /* (FTP terminology: reply code) */
  tTlHeaderState tlheaderstate; /* EXAC(resource) */
  tResourceProtocol protocol;
  tResourceState state;
  tResourceCommandHandshake handshake;
  tResourceError error;
#if OPTION_TLS
  tTlsError tls_error; /* used if ->error == reTls */
#endif
} tResource;


/* Resource requests */

my_enum1 enum
{ rraLoad = 0, rraReload = 1, rraEnforcedReload = 2
#if CONFIG_CUSTOM_CONN
  , rraCustomConn = 3
#endif
} my_enum2(unsigned char) tResourceRequestAction;

#define is_loadrra(rra) \
  ( ((rra) == rraLoad) || ((rra) == rraReload) || ((rra) == rraEnforcedReload))

my_enum1 enum
{ rrsUnknown = 0, rrsError = 1, rrsPreparedByMain = 2, rrsAttachedResource = 3,
  rrsStopped = 4
#if CONFIG_ASYNC_DNS
  , rrsDnsLookup = 5
#endif
} my_enum2(unsigned char) tResourceRequestState;
#define MAX_RESOURCE_REQUEST_STATE (5)
extern /*@observer@*/ const char* const
  strResourceRequestState[MAX_RESOURCE_REQUEST_STATE + 1];

my_enum1 enum
{ rrfNone = 0, rrfResourceChanged = 0x01, rrfPost = 0x02,
  rrfIsRedirection = 0x04, rrfIsEmbedded = 0x08
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  , rrfDownload = 0x10
#endif
} my_enum2(unsigned char) tResourceRequestFlags;

struct tCachedHostInformation;

typedef struct
{ tUriData* uri_data;
  tDhmGenericData* dhm_data; /* (non-refcounted) */
  const tConfigProxy* proxy; /* EXAC(resource) */
  tResource* resource; /* the result of the request */
  struct tCachedHostInformation* lookup; /* EXAC(resource) */
  tSinkingData* sinking_data;
  tResourceRequestFlags flags;
  tResourceRequestAction action;
  tResourceRequestState state;
  tResourceError error;
} tResourceRequest;


/* User queries */

#if CONFIG_USER_QUERY

my_enum1 enum
{ mifNone = 0, mifQueryFailed = 0x01, mifUserCancelled = 0x02,
  mifObjectVanished = 0x04, mifProxyrelated = 0x08,
  mifPriorLoginAttemptFailed = 0x10, mifUsername = 0x20, mifPassword = 0x40,
  mifAccount = 0x80, mifSharedSecret = 0x100
#if CONFIG_FTP && OPTION_TLS
  , mifFtpsDataclear = 0x200
#endif
} my_enum2(unsigned short) tMissingInformationFlags;
#define mifSet (mifUsername | mifPassword | mifAccount | mifSharedSecret)

struct tUserQuery;
typedef void (*tUserQueryCallback)(struct tUserQuery*);
typedef struct tUserQuery
{ tUserQueryCallback callback; /* a function in resource.c */
  tResource* resource;
  struct tHostPortProtInfo* hppi; /* EXAC(resource) */
  const char* hostname; /* (just a pointer copy, no strdup()ing necessary) */
  tPortnumber portnumber; /* (in network byte order) */
  tMissingInformationFlags mif;
} tUserQuery;

extern void user_query_queue(tUserQuery*); /* see main.c */

#else

typedef struct { unsigned char dummy; } tUserQuery;

#endif


/* Execution of shell commands */

#if OPTION_EXECEXT & EXECEXT_SHELL

my_enum1 enum
{ esfNone = 0, esfReadStdout = 0x01, esfReadStderr = 0x02,
  esfEnforceHtml = 0x04
} my_enum2(unsigned char) tExecextShellFlags;

typedef struct
{ tResourceRequest* request;
  const char *command, *writedata;
  size_t writedata_size;
  tExecextShellFlags esf;
} tExecextShellData;

extern void resource_start_execext_shell(tExecextShellData*);

#endif


/* Custom connections */

#if CONFIG_CUSTOM_CONN

my_enum1 enum
{ ccpkNormal = 0, ccpkError = 1, ccpkNetcmd = 2, ccpkNetresp = 3
} my_enum2(unsigned char) tCustomConnPrintingKind;

typedef struct
{ const char* text;
  size_t len; /* (only needed for ccpkNetresp) */
  tCustomConnPrintingKind ccpk;
} tCustomConnPrintingData;

extern void resource_custom_conn_start(tResource*, unsigned char, const void*);

extern void main_handle_custom_conn(tResource*, unsigned char, void*);
  /* see main.c */

#endif


/* Functions */

extern tPortnumber rp2portnumber(tResourceProtocol);
extern void rp2scheme(tResourceProtocol, /*@out@*/ char*);
extern void resource_preplex(void);
extern void resource_postplex(void);
extern void resource_request_start(tResourceRequest*);
extern void resource_request_stop(tResourceRequest*);
extern void resource_start_saving(tResource*, const tCantent*, int);
extern void uri_put(tUriData*);
#define uri_attach(dest, u) do { (u)->refcount++; (dest) = (u); } while (0)
#define uri_detach(u) do { uri_put(u); (u) = NULL; } while (0)
extern tCantent* cantent_create(void);
extern void cantent_put(tCantent*);
#define cantent_attach(dest, c) do { (c)->refcount++; (dest) = (c); } while (0)
#define cantent_detach(c) do { cantent_put(c); (c) = NULL; } while (0)
extern void cantent_collect_str(tCantent*, const char*);
extern void cantent_collect_title(tCantent*, const char*);
extern void sinking_data_cleanup(tSinkingData*);
extern void sinking_data_deallocate(tSinkingData**);

extern const char* htmlify(const char*);
#define htmlify_cleanup(orig, htmlified) \
  do { if (htmlified != orig) memory_deallocate(htmlified); } while (0)

#if CONFIG_BLOAT & BLOAT_SSC
extern /*@observer@*/ const char* ssc2info(tResourceProtocol,
  tServerStatusCode);
#else
#define ssc2info(rp, ssc) (strEmpty)
#endif

#define USE_S2U ((CONFIG_BLOAT & BLOAT_UIIP) || (CONFIG_DEBUG))
#if USE_S2U
extern tBoolean resource_ui_conn_ip(const tResource*, /*@out@*/ char*, size_t);
#else
#define resource_ui_conn_ip(res, buf, size) (falsE)
#endif

#if OPTION_TLS
extern tBoolean resource_in_tls(const tResource*, tBoolean);
#endif

extern void resource_initialize(void);
extern void resource_quit(void);

#endif /* #ifndef __retawq_resource_h__ */
